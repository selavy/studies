#include <iostream>
#include <cstdint>
#include <cstddef>
#include <immintrin.h>
#include <fmt/ostream.h>

// #define BITS_PER_OP 128
#define BITS_PER_OP 256

constexpr size_t BITS_PER_OP_  = BITS_PER_OP;
constexpr size_t BITS_PER_KEY = 32;
constexpr size_t STRIDE = BITS_PER_OP / BITS_PER_KEY;
constexpr size_t M = 4;
constexpr uint32_t vals[M] = {
    0xFFFFFFFFu,
    0x00000000u,
    0xDEADBEEFu,
    0x0F0F0F0Fu,
};

#if   BITS_PER_OP == 128
constexpr size_t ALIGN = 16;
#elif BITS_PER_OP == 256
constexpr size_t ALIGN = 32;
#else
#error "Invalid BITS_PER_OP"
#endif

void dump(const char* name, const uint32_t* vals)
{
    for (size_t i = 0; i != STRIDE; ++i) {
        fmt::print("{:s}[{:d}] = 0x{:08x};\n", name, i, vals[i]);
    }
    fmt::print("\n");
}

int main(int argc, const char** argv)
{
    size_t i, j;
    alignas(ALIGN) uint32_t keys[STRIDE];
    alignas(ALIGN) uint32_t outs[STRIDE];

    fmt::print("align        = {}\n", ALIGN);
    fmt::print("bits_per_op  = {}\n", BITS_PER_OP);
    fmt::print("bits_per_key = {}\n", BITS_PER_KEY);
    fmt::print("stride       = {}\n", STRIDE);

    for (i = 0; i != STRIDE; ++i) {
        keys[i] = vals[i % M];
    }
    dump("keys", keys);

    for (j = 0; j != M; ++j) {
#if   BITS_PER_OP == 128
        __m128i  needle;
        __m128i  haystk;
        __m128i  result;
        __m128i  ones;
        uint32_t mvmask;

        ones   = _mm_set1_epi32(0xFFFFFFFFu);
        needle = _mm_set1_epi32(vals[j]);
        haystk = _mm_load_si128((__m128i const*)&keys[0]);
        result = _mm_cmpeq_epi32(needle, haystk);
        result = _mm_xor_si128(result, ones);
        _mm_store_si128((__m128i*)&outs[0], result);
        mvmask = _mm_movemask_ps((__m128)result);
#elif BITS_PER_OP == 256
        __m256i needle;
        __m256i haystk;
        __m256i result;
        __m256i ones;
        uint32_t mvmask;

        ones   = _mm256_set1_epi32(0xFFFFFFFFu);
        needle = _mm256_set1_epi32(vals[j]);
        haystk = _mm256_load_si256((__m256i const*)&keys[0]);
        result = _mm256_cmpeq_epi32(needle, haystk);
        result = _mm256_xor_si256(result, ones);
        _mm256_store_si256((__m256i*)&outs[0], result);
        mvmask = _mm256_movemask_ps((__m256)result);
#else
#error "Invalid BITS_PER_OP"
#endif

        fmt::print("j = {}\n", j);
        dump("outs", outs);
        for (size_t i = 0; i != STRIDE; ++i) {
            fmt::print("mvmask[{:d}] = {:d}\n", i, (mvmask >> i) & 0x1u);
        }
    }

    return 0;
}
