#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

#define IN_WORD  0
#define OUT_WORD 1

char buf[4096];
int run(const char* filename)
{
    size_t read, i;
    FILE* stream;
    int lines = 0, words = 0, bytes = 0;
    int state = OUT_WORD;

    stream = fopen(filename, "rb");
    if (!stream) {
        fprintf(stderr, "error: unable to open input: %s\n", filename);
        return 1;
    }

    do {
        read = fread(&buf[0], 1, sizeof(buf), stream);
        bytes += read;
        for (i = 0; i < read; ++i) {
            lines += buf[i] == '\n';
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
        fclose(stream);
        return 1;
    }

    printf(" %d\t%d\t%d\t%s\n", lines, words, bytes, filename);

    fclose(stream);
    return 0;
}

int main(int argc, char** argv)
{
    int i;
    for (i = 1; i < argc; ++i) {
        run(argv[i]);
    }
    return 0;
}
