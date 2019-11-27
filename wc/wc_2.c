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
#define simd_cmpws_i8_mask(a)     simd_cmpws_i8(a)
#define simd_store(a, b)          _mm256_store_si256((a), (b))
typedef __m256i simd_vector;
typedef uint32_t simd_imask;
typedef simd_vector simd_mask;
static inline simd_vector simd_cmpws_i8(simd_vector a)
{
	simd_vector shuffle_src = _mm256_set_epi64x(0x0d0c0b0a0900, 0x20,
	                                            0x0d0c0b0a0900, 0x20);
	return simd_cmpeq_i8(_mm256_shuffle_epi8(shuffle_src, a), a);
}
// shift a left by 1 byte, shifting in byte from the end of b
static inline simd_vector simd_shl1_from_i8(simd_vector a, simd_vector b)
{
	return _mm256_alignr_epi8(a, _mm256_permute2x128_si256(b, a, 0x21), 15);
}

typedef struct {
	simd_vector vcount, count;
	uint8_t iterations;
} lcount_state;
typedef struct {
	simd_vector vcount, count, prev_eqws;
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
		simd_vector vec;
		uint64_t u64[sizeof(simd_vector)/sizeof(uint64_t)];
	} unpack;
	simd_store(&unpack.vec, state->count);
	for (int i = 0; i != sizeof(simd_vector)/sizeof(uint64_t); ++i)
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
		simd_vector vec;
		uint64_t u64[sizeof(simd_vector)/sizeof(uint64_t)];
	} unpack;
	simd_store(&unpack.vec, state->count);
	for (int i = 0; i != sizeof(simd_vector)/sizeof(uint64_t); ++i)
		sum += unpack.u64[i];
	return sum;
}

static inline int count_lines(simd_vector vec, lcount_state *state)
{
	simd_vector is_line_feed = simd_cmpeq_i8(vec, simd_set_i8('\n'));
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

    simd_vector *buf = aligned_alloc(sizeof(simd_vector), BUFSIZE);
	ssize_t len;
	size_t lcount = 0,
	       wcount = 0,
	       ccount = 0,
	       rem = 0;
	lcount_state lstate = LCOUNT_INITIAL;
	wcount_state wstate = WCOUNT_INITIAL;

    while ((len = read(fd, (char*)buf + rem, BUFSIZE - rem)) > 0) {
		rem += len;
		ccount += len;
		simd_vector *vp = buf;
		while (rem >= sizeof(simd_vector)) {
			lcount += count_lines(*vp, &lstate);
			wcount += count_words(*vp, &wstate);
			rem -= sizeof(simd_vector);
			vp++;
		}
        memmove(buf, vp, rem);
    }

    if (len < 0) {
        perror("fastlwc: read");
        exit(EXIT_FAILURE);
    }

	if (rem) {
		memset((char*)buf + rem, ' ', sizeof(simd_vector) - rem);
		simd_vector *vp = buf;
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
    if (argc == 1) {
        process(STDIN_FILENO, "");
    } else {
        for (int i = 1; i < argc; ++i) {
            run_file(argv[i]);
        }
    }
    return 0;
}
