#include "fp.h"
#include <cstring>
#include <cassert>

// TEMP
#include <cstdio>
#include <cstdlib>

constexpr int Binary16_ExpBias = 15;
constexpr int Binary32_ExpBias = 127;
constexpr uint16_t Binary16_SignBit = 0b1000'0000'0000'0000u;
constexpr uint16_t Binary16_NAN   = 0b0111111111111111u;
constexpr uint16_t Binary16_INF   = 0b0111110000000000u;
constexpr uint16_t Binary16_NINF  = 0b1111110000000000u;
constexpr uint16_t Binary16_ZERO  = 0b0000000000000000u;
constexpr uint16_t Binary16_NZERO = 0b1000000000000000u;

constexpr uint16_t Binary16_SignMask     = 0b1000'0000'0000'0000u;
constexpr uint16_t Binary16_ExponentMask = 0b0111'1100'0000'0000u;
constexpr uint16_t Binary16_MantissaMask = 0b0000'0011'1111'1111u;

#define NYI() do {                                                            \
    assert(0 && "not yet implemented");                                       \
    puts("This feature has not been implemented");                            \
    exit(0);                                                                  \
} while (0)


static bool all(uint16_t a, uint16_t mask)
{
    return (a & mask) == mask;
}

static bool any(uint16_t a, uint16_t mask)
{
    return (a & mask) != 0;
}

static bool none(uint16_t a, uint16_t mask)
{
    return !any(a, mask);
}

binary16 _make_biased(uint16_t sign, uint16_t exponent, uint16_t mantissa)
{
    binary16 rv;
    rv.rep = ((sign     & 0x1u  ) << 15)
           | ((exponent & 0x1Fu ) << 10)
           | ((mantissa & 0x3FFu) <<  0);
    return rv;
}

binary16 binary16_make(const uint16_t sign, const int16_t exponent, const uint16_t mantissa)
{
    constexpr uint16_t expbits = 0b1'1111u;
    constexpr uint16_t sigbits = 0b11'1111'1111u;
    const uint16_t sgn = sign ? 0x1 : 0x0;
    const uint16_t exp = (static_cast<uint16_t>(exponent) & expbits) + Binary16_ExpBias;
    const uint16_t sig = mantissa & sigbits;
    return _make_biased(sgn, exp, sig);
}

uint16_t binary16_torep(binary16 val)
{
    return val.rep;
}

binary16 binary16_fromrep(uint16_t val)
{
    binary16 rv;
    rv.rep = val;
    return rv;
}

int32_t clampi32(int32_t val, int32_t lo, int32_t hi)
{
    if (val < lo) {
        return lo;
    } else if (val > hi) {
        return hi;
    } else {
        return val;
    }
}

binary16 binary16_fromfloat(float f)
{
    // 5-bits => [1, 30] - 15 => [-14, 15]
    // N.B. 0 and 31 are reserved for 0/subnormal and inf/nan
    constexpr uint32_t MaxExponent = Binary32_ExpBias + 15;
    constexpr uint32_t MinExponent = Binary32_ExpBias - 14;

    uint32_t rep;
    memcpy(&rep, &f, sizeof(rep));
    const uint32_t sign     = (rep >> 31) & 0x01u;
    const uint32_t exponent = (rep >> 23) & 0xFFu;
    const uint32_t mantissa = rep & 0x7FFFFFu;

    uint16_t exp;
    uint16_t sig;
    if (exponent == 0x00u) {
        exp = 0b00000u;
        sig = mantissa >> (23 - 10);
    } else if (exponent == 0xFFu) {
        exp = 0b11111u;
        sig = mantissa >> (23 - 10);
    } else if (exponent < MinExponent) {
        // round to zero if exponent too negative
        exp = 0b00000u;
        sig = 0b00000u;
    } else if (exponent > MaxExponent) {
        // round to +/- inf if exponent too positive
        exp = 0b11111u;
        sig = mantissa == 0 ? 0x0u : 0x1u;
    } else {
        exp = exponent - Binary32_ExpBias + Binary16_ExpBias;
        sig = mantissa >> (23 - 10);
    }

    const uint16_t sgn = static_cast<uint16_t>(sign);

    binary16 rv;
    rv.rep = ((sgn & 0x1u  ) << 15)
           | ((exp & 0x1Fu ) << 10)
           | ((sig & 0x3FFu) <<  0);
    return rv;
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
    uint16_t bexp = (x >> 10) & expmask;
    return bexp == 0x0 ? 0x0 : bexp == expmask ? expmask : bexp - Binary16_ExpBias;
}

uint16_t binary16_mantissa(binary16 x_)
{
    constexpr uint16_t sigmask = 0b11'1111'1111u;
    uint16_t x = x_.rep;
    return x & sigmask;
}

bool binary16_isinf(const binary16 x_)
{
    //| Sign | Exponent  | Mantissa                |
    //+---------------------------------------------+
    //|   X  | 1 1 1 1 1 | 0 0 , 0 0 0 0 , 0 0 0 0 |

    uint16_t x = x_.rep;
    return (x & ~Binary16_SignMask) == Binary16_ExponentMask;
}

bool binary16_isnan(const binary16 x_)
{
    // Sign      = X
    // Exponent  = 0b11111
    // Mantissa != 0
    uint16_t x = x_.rep;
    return all(x, Binary16_ExponentMask) && any(x, Binary16_MantissaMask);
}

bool binary16_iszero(const binary16 x_)
{
    // Sign     = X
    // Exponent = 0b00000
    // Mantissa = 0
    uint16_t x = x_.rep;
    return (x & ~Binary16_SignMask) == 0;
}

bool binary16_issubnormal(binary16 x_)
{
    // Sign      = X
    // Exponent  = 0b00000
    // Mantissa != 0
    uint16_t x = x_.rep;
    return none(x, Binary16_ExponentMask) && any(x, Binary16_MantissaMask);
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
    uint16_t x = x_.rep;
    return any(x, Binary16_SignMask);
}

float binary16_tofloat(const binary16 x_)
{
    constexpr uint16_t expmask = 0b0000'0000'0001'1111u;
    constexpr uint16_t sigmask = 0b0000'0011'1111'1111u;
    const uint16_t x = x_.rep;
    const uint32_t sgn = x >> 15;
    const uint32_t bexp = (x >> 10) & expmask;
    const uint32_t sig = (x & sigmask);

    uint32_t exp;
    if (bexp == 0b00000u) {
        exp = 0x00u;
    } else if (bexp == 0b11111u) {
        exp = 0xFF;
    } else {
        exp = bexp - Binary16_ExpBias + Binary32_ExpBias;
    }

    // TODO: do subnormals need to be handled differently
    // TODO: handle inf and nan
    uint32_t rep = ((sgn & 0x01) << 31) | ((exp & 0xFFu) << 23) | ((sig & 0x3FFu) << (23 - 10));
    float res;
    static_assert(sizeof(rep) == sizeof(res));
    memcpy(&res, &rep, sizeof(res));
    return res;
}

uint16_t _sign_2scomp(uint16_t sign, uint16_t val)
{
    return sign ? (val ^ 0xFFFFu) + 1 : val;
}

uint16_t _abs_2scomp(uint16_t val)
{
    if ((val >> 15) & 0x1) {
        return (val ^ 0xFFFFu) + 1;
    } else {
        return val;
    }

    // TODO: branchless?
    // uint16_t mask = val >> 15;
    // return (val + mask) ^ mask;
}

binary16 binary16_neg(const binary16 a_)
{
    constexpr uint16_t SignMask = 0b1000'0000'0000'0000u;
    uint16_t a = a_.rep;
    return binary16_fromrep(a ^ SignMask);
}

// TODO: implement
binary16 binary16_add(const binary16 a_, const binary16 b_)
{
    constexpr uint16_t ExponentMask = 0b0111110000000000u;
    constexpr uint16_t FractionMask = 0b0000001111111111u;
    constexpr uint16_t ImpliedOne   = 0b0000010000000000u;

    uint16_t a = a_.rep;
    uint16_t b = b_.rep;

    if (binary16_isnan(a_) || binary16_isnan(b_)) {
        return binary16_fromrep(Binary16_NAN);
    }
    if (binary16_issubnormal(a_) || binary16_issubnormal(b_)) {
        NYI();
    }

    // case #1: exponent(A) == exponent(B)
    uint16_t exponent_A = (a & ExponentMask) >> 10;
    uint16_t exponent_B = (b & ExponentMask) >> 10;
    uint16_t exptcomm   = exponent_A; // TODO: only works for case #1
    uint16_t sign_A     = binary16_sign(a_);
    uint16_t sign_B     = binary16_sign(b_);
    uint16_t mantissa_A = _sign_2scomp(sign_A, (a & FractionMask) | ImpliedOne);
    uint16_t mantissa_B = _sign_2scomp(sign_B, (b & FractionMask) | ImpliedOne);
    uint16_t result     = mantissa_A + mantissa_B;
    uint16_t sign_C     = (result >> 15) & 0x1u;
    uint16_t mantissa   = _abs_2scomp(result);
    // re-normalize mantissa
    assert(((mantissa >> 15) & 0x1u) == 0u && "top-bit shouldn't be set on mantissa!");
    // NOTE: __bulitin_clz promotes mantissa to 32-bits, for now converting back to 16-bits
    uint16_t clz        = __builtin_clz(mantissa) - (32 - 16);
    uint16_t mantissa_C;
    uint16_t exponent_C;
    if (clz > 5) {
        mantissa_C = mantissa << (clz - 5);
        exponent_C = exptcomm - (clz - 5);
    } else {
        mantissa_C = mantissa >> (5 - clz);
        exponent_C = exptcomm + (5 - clz);
    }

    if (exponent_A != exponent_B) {
        NYI();
    }
    return _make_biased(sign_C, exponent_C, mantissa_C);
}

binary16 binary16_sub(const binary16 a_, const binary16 b_)
{
    return binary16_add(a_, binary16_neg(b_));
}
