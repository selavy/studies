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
#include <stdint.h>

// #define USE_FD

char buf[4096];

#define popcnt __builtin_popcount

#if 1

// using pshufb
static uint32_t simd_isspace(__m256i d)
{
    const __m256i shuffle = _mm256_set_epi64x(
            0x0d0c0b0a0900, // = '\r' '\f' '\v' '\n' '\t' '\0'
            0x20,           // = ' '
            0x0d0c0b0a0900, // = '\r' '\f' '\v' '\n' '\t' '\0'
            0x20            // = ' '
            );
    const __m256i isnewline  = _mm256_cmpeq_epi8(d, _mm256_set1_epi8('\n'));
    const __m256i a = _mm256_shuffle_epi8(shuffle, d);
    const __m256i b = _mm256_cmpeq_epi8(a, d);
    return _mm256_movemask_epi8(b);
}

#else

static uint32_t simd_isspace(__m256i d)
{
	const __m256i isnewline  = _mm256_cmpeq_epi8(d, _mm256_set1_epi8('\n'));
	const __m256i ishspace   = _mm256_cmpeq_epi8(d, _mm256_set1_epi8( ' '));
	const __m256i isformfeed = _mm256_cmpeq_epi8(d, _mm256_set1_epi8('\f'));
	const __m256i iscarriage = _mm256_cmpeq_epi8(d, _mm256_set1_epi8('\r'));
	const __m256i ishorztab  = _mm256_cmpeq_epi8(d, _mm256_set1_epi8('\t'));
	const __m256i isverttab  = _mm256_cmpeq_epi8(d, _mm256_set1_epi8('\v'));
	const __m256i mask =
		_mm256_or_si256(isnewline,
				_mm256_or_si256(ishspace,
					_mm256_or_si256(isformfeed,
						_mm256_or_si256(iscarriage,
							_mm256_or_si256(ishorztab, isverttab)
							)
						)
					)
				);

	return _mm256_movemask_epi8(mask);
}

#endif

int process(int fd, const char* filename)
{
    const size_t stride = 32;
    size_t len, extra;
    int lines = 0, words = 0, bytes = 0;
    uint32_t prevchk = 0xFFFFFFFFu;

    while ((len = read(fd, &buf[0], sizeof(buf))) > 0) {
        bytes += len;

        extra = stride - (bytes % stride);
        memset(&buf[len], 0, extra);
        assert((bytes + extra) % stride == 0);

        for (int i = 0; i < len; i += stride) {
            // isspace()
            //     checks for white-space characters.  In the "C" and "POSIX" locales,
            //     these are: space, form-feed ('\f'), newline ('\n'), carriage return ('\r'),
            //     horizontal tab ('\t'), and vertical tab ('\v').
            const __m256i data = _mm256_loadu_si256((const __m256i*)&buf[i]);
            const __m256i isnewline  = _mm256_cmpeq_epi8(data, _mm256_set1_epi8('\n'));
            const uint32_t isspace = simd_isspace(data);
            const uint32_t previsspace = (isspace << 1) | ((prevchk & (1u << 31)) >> 31);
            const uint32_t isword  = ~isspace & previsspace;
            prevchk = isspace;
            words += popcnt(isword);
            lines += popcnt(_mm256_movemask_epi8(isnewline));
        }
    }

    if (len < 0) {
        fprintf(stderr, "error: read error while reading input: %s\n", strerror(errno));
        return 1;
    }

	printf(" %7d %7d %7d %s\n", lines, words, bytes, filename);

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
