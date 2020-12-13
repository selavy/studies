#include "fp.h"
#include <cstring>

binary16 binary16_make(uint16_t sign, int16_t exponent, uint16_t mantissa)
{
    constexpr uint16_t expbits = 0b1'1111u;
    constexpr uint16_t sigbits = 0b11'1111'1111u;

    binary16 res;
    const uint16_t sgn = sign ? 0x1 : 0x0;
    const uint16_t exp = (static_cast<uint16_t>(exponent) & expbits) + Binary16_ExpBias;
    const uint16_t sig = mantissa & sigbits;
    res.rep = ((sgn & 0x1u  ) << 15)
            | ((exp & 0x1Fu ) << 10)
            | ((sig & 0x3FFu) <<  0);
    return res;
}

uint16_t binary16_sign(binary16 x_)
{
    uint16_t x = x_.rep;
    return (x >> 15) & 0x01;
}

uint16_t binary16_exponent(binary16 x_)
{
    constexpr uint16_t expmask = 0b11111u;
    uint16_t x = x_.rep;
    return ((x >> 10) & expmask) - Binary16_ExpBias;
}

uint16_t binary16_mantissa(binary16 x_)
{
    constexpr uint16_t sigmask = 0b11'1111'1111u;
    uint16_t x = x_.rep;
    return x & sigmask;
}

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

float binary16_tofloat(const binary16 x_)
{
    constexpr size_t Binary32_ExpBias = 127;
    constexpr uint16_t expmask = 0b0000'0000'0001'1111u;
    constexpr uint16_t sigmask = 0b0000'0011'1111'1111u;
    const uint16_t x = x_.rep;
    const uint32_t sgn = x >> 15;
    const uint32_t exp = ((x >> 10) & expmask) + (Binary32_ExpBias - Binary16_ExpBias);
    const uint32_t sig = (x & sigmask);
    // TODO: do subnormals need to be handled differently
    // TODO: handle inf and nan
    uint32_t rep = ((sgn & 0x01) << 31) | ((exp & 0xFFu) << 23) | ((sig & 0x3FFu) << (23 - 10));
    float res;
    static_assert(sizeof(rep) == sizeof(res));
    memcpy(&res, &rep, sizeof(res));
    return res;
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

