#pragma once

#include <cstdint>
#include <immintrin.h>
#include <fmt/ostream.h> // TEMP TEMP


// TODO: clean this up
#define LIKELY(x)   __builtin_expect(!!(x), true)
#define UNLIKELY(x) __builtin_expect(!!(x), false)
#define restrict __restrict

namespace gentbl {

namespace v1 {

struct Table
{
public:
    constexpr static std::size_t N = 32;
    constexpr static std::size_t NO_SLOT = -1;
    constexpr static uint64_t    INVALID = 0;
    constexpr static std::size_t BITS_PER_OP  = 256;
    constexpr static std::size_t BITS_PER_KEY = 32;
    constexpr static std::size_t STRIDE = BITS_PER_OP / BITS_PER_KEY;
    constexpr static std::size_t ALIGN = 32;
    static_assert((N % STRIDE) == 0);

    bool insert(uint32_t x, uint32_t y, uint64_t z) noexcept
    {
        auto slot = find_open_slot(x, xs);
        if (LIKELY(slot != NO_SLOT)) {
            xs[slot] = x;
            ys[slot] = y;
            zs[slot] = z;
            return true;
        }
        return false;
    }

    uint64_t find(uint32_t x, uint32_t y) const noexcept
    {
        auto slot = find_slot(x, xs, y, ys);
        if (LIKELY(slot == NO_SLOT)) {
            return INVALID;
        }
        return zs[slot];
    }

    int size(uint32_t x) const noexcept
    {
        return count_slots(x, xs);
    }

    // TEMP TEMP
    void dump(std::ostream& os)
    {
        fmt::print(os, "Table = {{\n");
        for (std::size_t i = 0; i != N; ++i) {
            fmt::print(os, "    {:08x} {:08x} {:016x}\n", xs[i], ys[i], zs[i]);
        }
        fmt::print(os, "}}\n");
    }

private:
    static std::size_t find_open_slot(
            uint32_t x, const uint32_t* restrict xs) noexcept
    {
        std::size_t i;
        __m256i needle;
        __m256i haystk;
        __m256i result;
        needle = _mm256_set1_epi32(x);
        for (i = 0; i != N; i += STRIDE) {
            haystk = _mm256_load_si256(&xs[i]);
            result = _mm256_cmpeq_epi32(needle, haystk);
        }

        for (std::size_t i = 0; i != N; ++i) {
            if (xs[i] != x) {
                return i;
            }
        }
        return NO_SLOT;
    }

    static std::size_t find_slot(
            uint32_t x, const uint32_t* restrict xs,
            uint32_t y, const uint32_t* restrict ys) noexcept
    {
        for (std::size_t i = 0; i != N; ++i) {
            if (UNLIKELY(x == xs[i] && y == ys[i])) {
                return i;
            }
        }
        return NO_SLOT;
    }

    static int count_slots(uint32_t x, const uint32_t* restrict xs) noexcept
    {
        int result = 0;
        for (std::size_t i = 0; i != N; ++i) {
            if (x == xs[i]) {
                ++result;
            }
        }
        return result;
    }

    alignas(ALIGN) uint32_t xs[N] = { 0 };
    alignas(ALIGN) uint32_t ys[N] = { 0 };
    uint64_t zs[N] = { 0 };
};

} // namespace v1

} // namespace gentbl
