#pragma once

#include <cstdint>
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
    static constexpr std::size_t N       = 32;
    constexpr static std::size_t NO_SLOT = -1;
    constexpr static uint64_t         INVALID = 0;

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

    uint32_t xs[N] = { 0 };
    uint32_t ys[N] = { 0 };
    uint64_t zs[N] = { 0 };
};

} // namespace v1

} // namespace gentbl
