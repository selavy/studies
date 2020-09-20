#include <iostream>
#include <cstdint>
#include <cstddef>
#include <immintrin.h>
#include <fmt/ostream.h>

constexpr size_t ALIGN = 32;
constexpr size_t BITS_PER_OP  = 128;
constexpr size_t BITS_PER_KEY = 32;
constexpr size_t STRIDE = BITS_PER_OP / BITS_PER_KEY;
constexpr size_t M = 4;
constexpr uint32_t vals[M] = {
    0xFFFFFFFFu,
    0x00000000u,
    0xDEADBEEFu,
    0x0F0F0F0Fu,
};

void dump_keys(const uint32_t* keys)
{
    for (size_t i = 0; i != STRIDE; ++i) {
        fmt::print("keys[{:d}] = 0x{:08x};\n", i, keys[i]);
    }
}

int main(int argc, const char** argv)
{
    size_t i;
    alignas(ALIGN) uint32_t keys[STRIDE];
    alignas(ALIGN) uint32_t outs[STRIDE];
    __m128i  needle;
    __m128i  haystk;
    __m128i  result;
    uint32_t mvmask;

    fmt::print("align        = {}\n", ALIGN);
    fmt::print("bits_per_op  = {}\n", BITS_PER_OP);
    fmt::print("bits_per_key = {}\n", BITS_PER_KEY);
    fmt::print("stride       = {}\n", STRIDE);

    for (i = 0; i != STRIDE; ++i) {
        keys[i] = vals[i % M];
    }
    dump_keys(keys);

    needle = _mm_set1_epi32(vals[3]);
    haystk = _mm_load_si128((__m128i const*)&keys[0]);
    result = _mm_cmpeq_epi32(needle, haystk);
    // TODO: try _mm_cmpneq_ps(__m128, __m128)
    _mm_store_si128((__m128i*)&outs[0], result);
    dump_keys(outs);
    mvmask = _mm_movemask_ps((__m128)result);
    for (size_t i = 0; i != STRIDE; ++i) {
        fmt::print("mvmask[{:d}] = {:d}\n", i, (mvmask >> i) & 0x1u);
    }

    // __m128i result;
    // int     mask;

    // result = _mm_add_epi32(a, b);
    // mask   = _mm_movemask_ps((__m128)result);


    return 0;
}
