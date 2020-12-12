#include "fp.h"

bool binary16_isinf(const binary16 x_)
{
    //| Sign | Exp         | Frac                    |
    //+---------------------------------------------+
    //|   X  | 1 1 1 , 1 1 | 0 0 , 0 0 0 0 , 0 0 0 0 |

    // constexpr uint16_t mask = 0x7C00u;
    constexpr uint16_t sgnmask = 0b1000'0000'0000'0000u;
    constexpr uint16_t expmask = 0b0111'1100'0000'0000u;
    uint16_t x = x_.rep;
    return (x & ~sgnmask) == expmask;
}

bool binary16_isnan(const binary16 x_)
{
    // Sign == X
    // Exp  == 0b11111
    // Frac != 0
    constexpr uint16_t expmask = 0b0111'1100'0000'0000u;
    constexpr uint16_t frcmask = 0b0000'0011'1111'1111u;
    uint16_t x = x_.rep;
    return ((x & expmask) == expmask) && ((x & frcmask) != 0);
}

bool binary16_iszero(const binary16 x_)
{
    // Sign == X
    // Exp  == 0b00000
    // Frac == 0
    constexpr uint16_t sgnmask = 0b1000'0000'0000'0000u;
    uint16_t x = x_.rep;
    return (x & ~sgnmask) == 0;
}

bool binary16_issubnormal(binary16 x_)
{
    // Sign == X
    // Exp  == 0b00000
    // Frac != 0
    constexpr uint16_t expmask = 0b0111'1100'0000'0000u;
    constexpr uint16_t frcmask = 0b0000'0011'1111'1111u;
    uint16_t x = x_.rep;
    return ((x & expmask) == 0) && ((x & frcmask) != 0);
}

bool binary16_isnormal(binary16 x_)
{
    // Determines if the given floating point number arg is normal, i.e. is
    // neither zero, subnormal, infinite, nor NaN. The macro returns an
    // integral value.
    if (binary16_isinf(x_)) {
        return false;
    }
    if (binary16_isnan(x_)) {
        return false;
    }
    if (binary16_iszero(x_)) {
        return false;
    }
    if (binary16_issubnormal(x_)) {
        return false;
    }
    return true;
}

bool binary16_signbit(const binary16 x_)
{
    constexpr uint16_t mask = 0b1000'0000'0000'0000u;
    uint16_t x = x_.rep;
    return (x & mask) != 0;
}

// TODO: implement
binary16 binary16_add(const binary16 a_, const binary16 b_)
{
    binary16 r;
    uint16_t a = a_.rep;
    uint16_t b = b_.rep;
    uint16_t c = a + b;
    r.rep = c;
    return r;
}

