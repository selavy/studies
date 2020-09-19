#pragma once

#include <cstdint>

using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using s16 = int16_t;
using s32 = int32_t;
using s64 = int64_t;

#define LIKELY(x)   __builtin_expect(!!(x), true)
#define UNLIKELY(x) __builtin_expect(!!(x), false)
#define restrict __restrict

namespace gentbl {

namespace v1 {

struct Table
{
public:
    constexpr static u64         INVALID = 0;
    constexpr static std::size_t NO_SLOT = -1;

    bool insert(u32 x, u32 y, u64 z) noexcept
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

    u64 find(u32 x, u32 y) const noexcept
    {
        auto slot = find_slot(x, xs, y, ys);
        if (LIKELY(slot == NO_SLOT)) {
            return INVALID;
        }
        return zs[slot];
    }

    int size(u32 x) const noexcept
    {
        return count_slots(x, xs);
    }

private:
    static constexpr std::size_t N = 32;

    static std::size_t find_open_slot(
            u32 x, const u32* restrict xs) noexcept
    {
        for (std::size_t i = 0; i != N; ++i) {
            if (xs[i] != x) {
                return i;
            }
        }
        return NO_SLOT;
    }

    static std::size_t find_slot(
            u32 x, const u32* restrict xs,
            u32 y, const u32* restrict ys) noexcept
    {
        for (std::size_t i = 0; i != N; ++i) {
            if (UNLIKELY(x == xs[i] && y == ys[i])) {
                return i;
            }
        }
        return NO_SLOT;
    }

    static int count_slots(u32 x, const u32* restrict xs) noexcept
    {
        int result = 0;
        for (std::size_t i = 0; i != N; ++i) {
            if (x == xs[i]) {
                ++result;
            }
        }
        return result;
    }

    u32 xs[N] = { 0 };
    u32 ys[N] = { 0 };
    u64 zs[N] = { 0 };
};

} // namespace v1

} // namespace gentbl
