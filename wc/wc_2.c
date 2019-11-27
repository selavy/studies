#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <immintrin.h>
#include <emmintrin.h>
#include <nmmintrin.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#define popcnt __builtin_popcount
#define BUFSIZE 4096
#define simd_vector_from_mask(a)  (a)
#define simd_imask_from_mask(a)   _mm256_movemask_epi8(a)
#define simd_set_i8(a)            _mm256_set1_epi8(a)
#define simd_setzero()            _mm256_setzero_si256()
#define simd_cmpeq_i8(a, b)       _mm256_cmpeq_epi8((a), (b))
#define simd_andnot_i8(a, b)      _mm256_andnot_si256((a), (b))
#define simd_add_i64(a, b)        _mm256_add_epi64((a), (b))
#define simd_sub_i8(a, b)         _mm256_sub_epi8((a), (b))
#define simd_sad_u8(a, b)         _mm256_sad_epu8((a), (b))
#define simd_imask_popcnt(a)      _mm_popcnt_u32(a)
#define simd_cmpeq_i8_mask(a, b)  simd_cmpeq_i8((a), (b))
// #define simd_cmpws_i8_mask(a)     simd_cmpws_i8(a)
#define simd_store(a, b)          _mm256_store_si256((a), (b))
typedef __m256i simd_vector;
typedef uint32_t simd_imask;
typedef simd_vector simd_mask;
static inline __m256i simd_cmpws_i8_mask(__m256i a)
{
    __m256i shuffle_src = _mm256_set_epi64x(
                0x0d0c0b0a0900,
                0x20,
                0x0d0c0b0a0900,
                0x20
            );
	return simd_cmpeq_i8(_mm256_shuffle_epi8(shuffle_src, a), a);
}
// shift a left by 1 byte, shifting in byte from the end of b
static inline __m256i simd_shl1_from_i8(__m256i a, __m256i b)
{
	return _mm256_alignr_epi8(a, _mm256_permute2x128_si256(b, a, 0x21), 15);
}

typedef struct {
	__m256i vcount, count;
	uint8_t iterations;
} lcount_state;
typedef struct {
	__m256i vcount, count, prev_eqws;
	uint8_t iterations;
} wcount_state;
#define LCOUNT_INITIAL \
	(lcount_state){ simd_setzero(), simd_setzero(), 0 }
#define WCOUNT_INITIAL \
	(wcount_state){ simd_setzero(), simd_setzero(), simd_set_i8(-1), 0 }
#define WCOUNT_CONTINUE \
	(wcount_state){ simd_setzero(), simd_setzero(), simd_setzero(), 0 }

static inline void wcount_state_set(wcount_state *state, bool wcontinue)
{
	state->prev_eqws = wcontinue ? simd_setzero() : simd_set_i8(-1);
}

static inline uint64_t count_lines_final(lcount_state *state)
{
	state->count = simd_add_i64(state->count,
	                            simd_sad_u8(state->vcount, simd_setzero()));
	state->vcount = simd_setzero();
	uint64_t sum = 0;
	union {
		__m256i vec;
		uint64_t u64[sizeof(__m256i)/sizeof(uint64_t)];
	} unpack;
	simd_store(&unpack.vec, state->count);
	for (int i = 0; i != sizeof(__m256i)/sizeof(uint64_t); ++i)
		sum += unpack.u64[i];
	return sum;
}

static inline uint64_t count_words_final(wcount_state *state)
{
	state->count = simd_add_i64(state->count,
	                            simd_sad_u8(state->vcount, simd_setzero()));
	state->vcount = simd_setzero();
	uint64_t sum = 0;
	union {
		__m256i vec;
		uint64_t u64[4];
	} unpack;
	simd_store(&unpack.vec, state->count);
	for (int i = 0; i != 4; ++i)
		sum += unpack.u64[i];
	return sum;
}

static inline int count_lines(__m256i vec, lcount_state *state)
{
	__m256i is_line_feed = simd_cmpeq_i8(vec, simd_set_i8('\n'));
	// is_line_feed has a value of -1 for line feeds, 0 otherwise
	state->vcount = simd_sub_i8(state->vcount, is_line_feed);
	state->iterations++;
	if (state->iterations == 255) {
		// sum line feed position counts before they can overflow
		state->count = simd_add_i64(state->count,
		                            simd_sad_u8(state->vcount, simd_setzero()));
		state->vcount = simd_setzero();
		state->iterations = 0;
	}
	return 0;
}

static inline int count_words_wsmask(simd_mask eqmask, wcount_state *state)
{
	simd_vector eqws = simd_vector_from_mask(eqmask),
	            andmsk = simd_shl1_from_i8(eqws, state->prev_eqws),
	            is_first_char = simd_andnot_i8(eqws, andmsk);
	state->prev_eqws = eqws;
	// is_first_char has a value of -1 for word starting characters, 0 otherwise
	state->vcount = simd_sub_i8(state->vcount, is_first_char);
	state->iterations++;
	if (state->iterations == 255) {
		// sum first character position counts before they can overflow
		state->count = simd_add_i64(state->count,
		                            simd_sad_u8(state->vcount, simd_setzero()));
		state->vcount = simd_setzero();
		state->iterations = 0;
	}
	return 0;
}

static inline int count_words(simd_vector vec, wcount_state *state)
{
	return count_words_wsmask(simd_cmpws_i8_mask(vec), state);
}

int process(int fd, const char* filename)
{

    __m256i *buf = aligned_alloc(sizeof(__m256i), BUFSIZE);
	ssize_t len;
	size_t lcount = 0, wcount = 0, ccount = 0, rem = 0;
	lcount_state lstate = {
        .vcount = simd_setzero(),
        .count = simd_setzero(),
        .iterations = 0,
    };
	wcount_state wstate = {
        .vcount = simd_setzero(),
        .count = simd_setzero(),
        .prev_eqws = simd_set_i8(-1),
        .iterations = 0
    };

    while ((len = read(fd, (char*)buf + rem, BUFSIZE - rem)) > 0) {
		rem += len;
		ccount += len;
		__m256i *vp = buf;
		while (rem >= sizeof(__m256i)) {
			lcount += count_lines(*vp, &lstate);
			wcount += count_words(*vp, &wstate);
			rem -= sizeof(__m256i);
			vp++;
		}
        memmove(buf, vp, rem);
    }

    if (len < 0) {
        perror("fastlwc: read");
        exit(EXIT_FAILURE);
    }

	if (rem) {
		memset((char*)buf + rem, ' ', sizeof(__m256i) - rem);
		__m256i *vp = buf;
		lcount += count_lines(*vp, &lstate);
		wcount += count_words(*vp, &wstate);
	}

	lcount += count_lines_final(&lstate);
	wcount += count_words_final(&wstate);

	printf(" %7zu %7zu %7zu %s\n", lcount, wcount, ccount, filename);
    free(buf);

    return 0;
}

int run_file(const char* filename)
{
    int rc;
    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        fprintf(stderr, "error: unable to open input: %s\n", filename);
        return 1;
    }
	posix_fadvise(fd, 0, 0, POSIX_FADV_SEQUENTIAL);
    rc = process(fd, filename);
    close(fd);
    return rc;
}

int main(int argc, char** argv)
{
    const char *ss = "This is\nan example\tsentence.\rabd";
    __m256i *buf = aligned_alloc(sizeof(__m256i), BUFSIZE);
    memcpy(buf, ss, strlen(ss));

    __m256i shuffle_src = _mm256_set_epi64x(
                0x0d0c0b0a0900,
                0x20,
                0x0d0c0b0a0900,
                0x20
            );
    __m256i v = *buf;
    __m256i a = _mm256_shuffle_epi8(shuffle_src, v);
    __m256i b = _mm256_cmpeq_epi8(a, v);

    printf("[%2d] %c %3d %3d\n",  0, (char)_mm256_extract_epi8(v,  0), _mm256_extract_epi8(a,  0), _mm256_extract_epi8(b,  0));
    printf("[%2d] %c %3d %3d\n",  1, (char)_mm256_extract_epi8(v,  1), _mm256_extract_epi8(a,  1), _mm256_extract_epi8(b,  1));
    printf("[%2d] %c %3d %3d\n",  2, (char)_mm256_extract_epi8(v,  2), _mm256_extract_epi8(a,  2), _mm256_extract_epi8(b,  2));
    printf("[%2d] %c %3d %3d\n",  3, (char)_mm256_extract_epi8(v,  3), _mm256_extract_epi8(a,  3), _mm256_extract_epi8(b,  3));
    printf("[%2d] %c %3d %3d\n",  4, (char)_mm256_extract_epi8(v,  4), _mm256_extract_epi8(a,  4), _mm256_extract_epi8(b,  4));
    printf("[%2d] %c %3d %3d\n",  5, (char)_mm256_extract_epi8(v,  5), _mm256_extract_epi8(a,  5), _mm256_extract_epi8(b,  5));
    printf("[%2d] %c %3d %3d\n",  6, (char)_mm256_extract_epi8(v,  6), _mm256_extract_epi8(a,  6), _mm256_extract_epi8(b,  6));
    printf("[%2d] %c %3d %3d\n",  7, (char)_mm256_extract_epi8(v,  7), _mm256_extract_epi8(a,  7), _mm256_extract_epi8(b,  7));
    printf("[%2d] %c %3d %3d\n",  8, (char)_mm256_extract_epi8(v,  8), _mm256_extract_epi8(a,  8), _mm256_extract_epi8(b,  8));
    printf("[%2d] %c %3d %3d\n",  9, (char)_mm256_extract_epi8(v,  9), _mm256_extract_epi8(a,  9), _mm256_extract_epi8(b,  9));
    printf("[%2d] %c %3d %3d\n", 10, (char)_mm256_extract_epi8(v, 10), _mm256_extract_epi8(a, 10), _mm256_extract_epi8(b, 10));
    printf("[%2d] %c %3d %3d\n", 11, (char)_mm256_extract_epi8(v, 11), _mm256_extract_epi8(a, 11), _mm256_extract_epi8(b, 11));
    printf("[%2d] %c %3d %3d\n", 12, (char)_mm256_extract_epi8(v, 12), _mm256_extract_epi8(a, 12), _mm256_extract_epi8(b, 12));
    printf("[%2d] %c %3d %3d\n", 13, (char)_mm256_extract_epi8(v, 13), _mm256_extract_epi8(a, 13), _mm256_extract_epi8(b, 13));
    printf("[%2d] %c %3d %3d\n", 14, (char)_mm256_extract_epi8(v, 14), _mm256_extract_epi8(a, 14), _mm256_extract_epi8(b, 14));
    printf("[%2d] %c %3d %3d\n", 15, (char)_mm256_extract_epi8(v, 15), _mm256_extract_epi8(a, 15), _mm256_extract_epi8(b, 15));
    printf("[%2d] %c %3d %3d\n", 16, (char)_mm256_extract_epi8(v, 16), _mm256_extract_epi8(a, 16), _mm256_extract_epi8(b, 16));
    printf("[%2d] %c %3d %3d\n", 17, (char)_mm256_extract_epi8(v, 17), _mm256_extract_epi8(a, 17), _mm256_extract_epi8(b, 17));
    printf("[%2d] %c %3d %3d\n", 18, (char)_mm256_extract_epi8(v, 18), _mm256_extract_epi8(a, 18), _mm256_extract_epi8(b, 18));
    printf("[%2d] %c %3d %3d\n", 19, (char)_mm256_extract_epi8(v, 19), _mm256_extract_epi8(a, 19), _mm256_extract_epi8(b, 19));
    printf("[%2d] %c %3d %3d\n", 20, (char)_mm256_extract_epi8(v, 20), _mm256_extract_epi8(a, 20), _mm256_extract_epi8(b, 20));
    printf("[%2d] %c %3d %3d\n", 21, (char)_mm256_extract_epi8(v, 21), _mm256_extract_epi8(a, 21), _mm256_extract_epi8(b, 21));
    printf("[%2d] %c %3d %3d\n", 22, (char)_mm256_extract_epi8(v, 22), _mm256_extract_epi8(a, 22), _mm256_extract_epi8(b, 22));
    printf("[%2d] %c %3d %3d\n", 23, (char)_mm256_extract_epi8(v, 23), _mm256_extract_epi8(a, 23), _mm256_extract_epi8(b, 23));
    printf("[%2d] %c %3d %3d\n", 24, (char)_mm256_extract_epi8(v, 24), _mm256_extract_epi8(a, 24), _mm256_extract_epi8(b, 24));
    printf("[%2d] %c %3d %3d\n", 25, (char)_mm256_extract_epi8(v, 25), _mm256_extract_epi8(a, 25), _mm256_extract_epi8(b, 25));
    printf("[%2d] %c %3d %3d\n", 26, (char)_mm256_extract_epi8(v, 26), _mm256_extract_epi8(a, 26), _mm256_extract_epi8(b, 26));
    printf("[%2d] %c %3d %3d\n", 27, (char)_mm256_extract_epi8(v, 27), _mm256_extract_epi8(a, 27), _mm256_extract_epi8(b, 27));
    printf("[%2d] %c %3d %3d\n", 28, (char)_mm256_extract_epi8(v, 28), _mm256_extract_epi8(a, 28), _mm256_extract_epi8(b, 28));
    printf("[%2d] %c %3d %3d\n", 29, (char)_mm256_extract_epi8(v, 29), _mm256_extract_epi8(a, 29), _mm256_extract_epi8(b, 29));
    printf("[%2d] %c %3d %3d\n", 30, (char)_mm256_extract_epi8(v, 30), _mm256_extract_epi8(a, 30), _mm256_extract_epi8(b, 30));
    printf("[%2d] %c %3d %3d\n", 31, (char)_mm256_extract_epi8(v, 31), _mm256_extract_epi8(a, 31), _mm256_extract_epi8(b, 31));

    exit(0);

    if (argc == 1) {
        process(STDIN_FILENO, "");
    } else {
        for (int i = 1; i < argc; ++i) {
            run_file(argv[i]);
        }
    }

    return 0;
}
