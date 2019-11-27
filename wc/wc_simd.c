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

#ifdef USE_FD
int process(int fd, const char* filename)
#else
int process(FILE* stream, const char* filename)
#endif
{
    const size_t stride = 32;
    size_t len, extra;
    int lines = 0, words = 0, bytes = 0;
    uint32_t prevchkmask = 0xFFFFFFFFu;

    const __m256i newline  = _mm256_set1_epi8('\n');
    const __m256i hspace   = _mm256_set1_epi8( ' ');
    const __m256i formfeed = _mm256_set1_epi8('\f');
    const __m256i carriage = _mm256_set1_epi8('\r');
    const __m256i horztab  = _mm256_set1_epi8('\t');
    const __m256i verttab  = _mm256_set1_epi8('\v');

#ifdef USE_FD
    while ((len = read(fd, &buf[0], sizeof(buf))) > 0) {
#else
    while ((len = fread(&buf[0], 1, sizeof(buf), stream)) > 0) {
#endif
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
            const __m256i isnewline  = _mm256_cmpeq_epi8(data, newline);
            const __m256i ishspace   = _mm256_cmpeq_epi8(data, hspace);
            const __m256i isformfeed = _mm256_cmpeq_epi8(data, formfeed);
            const __m256i iscarriage = _mm256_cmpeq_epi8(data, carriage);
            const __m256i ishorztab  = _mm256_cmpeq_epi8(data, horztab);
            const __m256i isverttab  = _mm256_cmpeq_epi8(data, verttab);
            const __m256i isspace =
                _mm256_or_si256(isnewline,
                        _mm256_or_si256(ishspace,
                            _mm256_or_si256(isformfeed,
                                _mm256_or_si256(iscarriage,
                                    _mm256_or_si256(ishorztab, isverttab)
                                    )
                                )
                            )
                        );

            const uint32_t isspacemask     = _mm256_movemask_epi8(isspace);
            const uint32_t isprevspacemask = (isspacemask << 1) | ((prevchkmask & (1u << 31)) >> 31);
            const uint32_t iswordmask      = ~isspacemask & isprevspacemask;
            prevchkmask = isspacemask;

            words += popcnt(iswordmask);
            lines += popcnt(_mm256_movemask_epi8(isnewline));
        }
    }

#ifndef USE_FD
    if (ferror(stream)) {
        fprintf(stderr, "error: read error while reading input: %s\n", strerror(errno));
        return 1;
    }
#endif

	printf(" %7d %7d %7d %s\n", lines, words, bytes, filename);

    return 0;
}

int run_file(const char* filename)
{
    int rc;
#ifdef USE_FD
    int stream = open(filename, O_RDONLY);
    if (stream < 0) {
        fprintf(stderr, "error: unable to open input: %s\n", filename);
        return 1;
    }
	posix_fadvise(stream, 0, 0, POSIX_FADV_SEQUENTIAL);
#else
    FILE* stream = fopen(filename, "rb");
    if (!stream) {
        fprintf(stderr, "error: unable to open input: %s\n", filename);
        return 1;
    }
#endif
    rc = process(stream, filename);
#ifndef USE_FD
    fclose(stream);
#endif
    return rc;
}

int main(int argc, char** argv)
{

    if (argc == 1) {
#ifdef USE_FD
        process(STDIN_FILENO, "");
#else
        process(stdin, "");
#endif
    } else {
        for (int i = 1; i < argc; ++i) {
            run_file(argv[i]);
        }
    }
    return 0;
}
