#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <immintrin.h>
#include <emmintrin.h>


#if defined(__INTEL_COMPILER) && (__INTEL_COMPILER >= 1300)

#include <immintrin.h>

int has_intel_knl_features()
{
    const unsigned long knl_features =
        (_FEATURE_AVX512F | _FEATURE_AVX512ER | 
         _FEATURE_AVX512PF | _FEATURE_AVX512CD );
    return _may_i_use_cpu_feature( knl_features );
}

#else /* non-Intel compiler */

#include <stdint.h>
#if defined(_MSC_VER)
#include <intrin.h>
#endif

void run_cpuid(uint32_t eax, uint32_t ecx, uint32_t* abcd)
{
#if defined(_MSC_VER)
    __cpuidex(abcd, eax, ecx);
#else
    uint32_t ebx, edx;
#if defined( __i386__ ) && defined ( __PIC__ )
    /* in case of PIC under 32-bit EBX cannot be clobbered */
    __asm__ ( "movl %%ebx, %%edi \n\t cpuid \n\t xchgl %%ebx, %%edi" : "=D" (ebx),
# else
            __asm__ ( "cpuid" : "+b" (ebx),
# endif
                "+a" (eax), "+c" (ecx), "=d" (edx) );
            abcd[0] = eax; abcd[1] = ebx; abcd[2] = ecx; abcd[3] = edx;
#endif
            }

            int check_xcr0_zmm() {
            uint32_t xcr0;
            uint32_t zmm_ymm_xmm = (7 << 5) | (1 << 2) | (1 << 1);
#if defined(_MSC_VER)
            xcr0 = (uint32_t)_xgetbv(0);  /* min VS2010 SP1 compiler is required */
#else
            __asm__ ("xgetbv" : "=a" (xcr0) : "c" (0) : "%edx" );
#endif
            return ((xcr0 & zmm_ymm_xmm) == zmm_ymm_xmm); /* check if xmm, zmm and zmm state are enabled in XCR0 */
            }

            int has_intel_knl_features() {
                uint32_t abcd[4];
                uint32_t osxsave_mask = (1 << 27); // OSX.
                uint32_t avx2_bmi12_mask = (1 << 16) | // AVX-512F
                    (1 << 26) | // AVX-512PF
                    (1 << 27) | // AVX-512ER
                    (1 << 28);  // AVX-512CD
                run_cpuid( 1, 0, abcd );
                // step 1 - must ensure OS supports extended processor state management
                if ( (abcd[2] & osxsave_mask) != osxsave_mask ) 
                    return 0;
                // step 2 - must ensure OS supports ZMM registers (and YMM, and XMM)
                if ( ! check_xcr0_zmm() )
                    return 0;

                return 1;
            }
#endif /* non-Intel compiler */

static int can_use_intel_knl_features() {
    static int knl_features_available = -1;
    /* test is performed once */
    if (knl_features_available < 0 )
        knl_features_available = has_intel_knl_features();
    return knl_features_available;
}

#define IN_WORD  0
#define OUT_WORD 1

char buf[4096];

#define popcount __builtin_popcount

int process(FILE* stream, const char* filename)
{
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

        extra = 32 - (bytes % 32);
        memset(&buf[read], 0, extra);
        bytes += extra;

        for (i = 0; i < read; i += (256 / 8)) {
            m = _mm256_loadu_si256((const __m256i*)&buf[i]);
            r = _mm256_cmpeq_epi8(m, newline);
            lines += popcount(_mm256_movemask_epi8(r));
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
    int i;

    if ( can_use_intel_knl_features() )
        printf("This CPU supports AVX-512F+CD+ER+PF as introduced in Knights Landing\n");
    else
        printf("This CPU does not support all Knights Landing AVX-512 features\n");

    if (argc == 1) {
        process(stdin, "");
    } else {
        for (i = 1; i < argc; ++i) {
            run_file(argv[i]);
        }
    }
    return 0;
}
