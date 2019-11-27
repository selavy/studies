#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <immintrin.h>
#include <emmintrin.h>

#define IN_WORD  0
#define OUT_WORD 1

char buf[4096];

#define popcnt __builtin_popcount

int process(FILE* stream, const char* filename)
{
    const size_t stride = 32;
    size_t read, i, j, extra;
    int lines = 0, words = 0, bytes = 0;
    int state = OUT_WORD;
    __m256i m, r, newline;

    newline = _mm256_set_epi8(
            '\n', '\n', '\n', '\n', '\n', '\n', '\n', '\n',
            '\n', '\n', '\n', '\n', '\n', '\n', '\n', '\n',
            '\n', '\n', '\n', '\n', '\n', '\n', '\n', '\n',
            '\n', '\n', '\n', '\n', '\n', '\n', '\n', '\n'
            );

    do {
        read = fread(&buf[0], 1, sizeof(buf), stream);
        bytes += read;

        extra = stride - (bytes % stride);
        memset(&buf[read], 0, extra);
        assert((bytes + extra) % stride == 0);

        for (i = 0; i < read; i += stride) {
            m = _mm256_loadu_si256((const __m256i*)&buf[i]);
            r = _mm256_cmpeq_epi8(m, newline);
            lines += popcnt(_mm256_movemask_epi8(r));
        }

        for (i = 0; i < read; ++i) {
            if (!isspace(buf[i])) {
                words += state == OUT_WORD;
                state = IN_WORD;
            } else {
                state = OUT_WORD;
            }
        }
    } while (read > 0);

    if (ferror(stream)) {
        fprintf(stderr, "error: read error while reading input: %s\n", strerror(errno));
        return 1;
    }

    printf(" %d\t%d\t%d\t%s\n", lines, words, bytes, filename);

    return 0;
}

int run_file(const char* filename)
{
    int rc;
    FILE* stream;
    stream = fopen(filename, "rb");
    if (!stream) {
        fprintf(stderr, "error: unable to open input: %s\n", filename);
        return 1;
    }
    rc = process(stream, filename);
    fclose(stream);
    return rc;
}

int main(int argc, char** argv)
{
    if (argc == 1) {
        process(stdin, "");
    } else {
        for (int i = 1; i < argc; ++i) {
            run_file(argv[i]);
        }
    }
    return 0;
}
