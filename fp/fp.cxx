#include "fp.h"
#include <cstring>
#include <cassert>

//-----------------------------------------------------------------------------
// TODOs:
// + make branchless
// + implement addition with subnormals
// + check for exponent overflow
// + check for exponent underflow
// + implement multiplication
//-----------------------------------------------------------------------------

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

int binary16_exponent(binary16 x_)
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

int binary16_desc(const binary16 x, char* buf, const int len)
{
    if (binary16_isnan(x)) {
        const char* const desc = "nan";
        const int N = strlen(desc) + 1;
        if (!(len >= N)) {
            return N;
        }
        memcpy(buf, desc, N*sizeof(char));
        return N;
    }
    if (binary16_isinf(x)) {
        const char* const desc = "+inf";
        const int N = strlen(desc) + 1;
        if (!(len >= N)) {
            return N;
        }
        memcpy(buf, desc, N*sizeof(char));
        if (binary16_signbit(x)) {
            buf[0] = '-';
        }
        return N;
    }
    if (binary16_issubnormal(x)) {
        int sign        = binary16_signbit(x);
        double mantissa = binary16_mantissa(x) / 1024.0;
        return snprintf(buf, len, "-1**%d x %0.10f x 2**-14", sign, mantissa);
    }
    if (binary16_isnormal(x)) {
        int    sign     = binary16_signbit(x);
        double mantissa = 1.0f + binary16_mantissa(x) / 1024.0f;
        int    exponent = binary16_exponent(x);
        return snprintf(buf, len, "-1**%d x %0.10f x 2**%d", sign, mantissa, exponent);
    }
    assert(0 && "unknown float pointing class");
    return  -1;
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
    uint16_t x         = x_.rep;
    uint32_t sign      = x >> 15;
    uint32_t bexponent = (x & Binary16_ExponentMask) >> 10;
    uint32_t mantissa  = (x & Binary16_MantissaMask);

    uint32_t exponent;
    if (bexponent == 0b00000u) {
        exponent = 0x00u;
    } else if (bexponent == 0b11111u) {
        exponent = 0xFF;
    } else {
        exponent = bexponent - Binary16_ExpBias + Binary32_ExpBias;
    }

    // re-normalize subnormals
    if (exponent == 0x00u && mantissa != 0) {
        // subnormals have implied exponent = -14
        exponent = -14 + 127;
        constexpr uint32_t ImpliedOne = 1u << 10;
        while ((mantissa & ImpliedOne) == 0) {
            mantissa <<= 1;
            exponent -=  1;
        }
        mantissa &= ~ImpliedOne;
    }

    // TODO: subnormals need to be re-normalized; they won't be subnormals in binary32
    // TODO: handle inf and nan
    uint32_t rep = ((sign     & 0x001u) << 31)
                 | ((exponent & 0x0FFu) << 23)
                 | ((mantissa & 0x3FFu) << (23 - 10));
    float res;
    static_assert(sizeof(rep) == sizeof(res));
    memcpy(&res, &rep, sizeof(res));
    return res;
}

static uint64_t _make_2scomp(uint16_t sign, uint64_t val)
{
    // return sign ? (val ^ 0xFFFF'FFFF'FFFF'FFFFull) + 1ull : val;
    return sign ? (val ^ -1ull) + 1ull : val;
}

static uint64_t _abs_2scomp(uint16_t sign, uint64_t val)
{
    return _make_2scomp(sign, val);
    // if ((val >> 15) & 0x1) {
    //     return (val ^ 0xFFFFu) + 1;
    // } else {
    //     return val;
    // }

    // TODO: branchless?
    // uint16_t mask = val >> 15;
    // return (val + mask) ^ mask;
}

binary16 binary16_neg(const binary16 a_)
{
    uint16_t a = a_.rep;
    return binary16_fromrep(a ^ Binary16_SignMask);
}

uint64_t _min(uint64_t x, uint64_t y)
{
    return x < y ? x : y;
}

uint64_t _max(uint64_t x, uint64_t y)
{
    return x > y ? x : y;
}

uint64_t _saturate_add(uint64_t a, uint64_t b, uint64_t max_value)
{
    uint64_t min_elem   = _min(a, b);
    uint16_t max_elem   = _max(a, b);
    uint64_t max_amount = max_value - max_elem;
    uint64_t add_value  = _min(max_amount, min_elem);
    uint64_t result = max_elem + add_value;
    assert(0 <= result && result <= max_value);
    return result;
}

binary16 _normalize(uint16_t sign, uint64_t exponent, uint64_t mantissa)
{
    // preconditions:
    //     + input is not NaN

    constexpr uint16_t MaxExponent = 0b11111;
    constexpr uint16_t MinExponent = 0b00001;

    assert((mantissa >> 63) == 0u && "top-bit shouldn't be set on mantissa!");

    while ((mantissa & ~0xFFFFu) != 0) {
        mantissa >>= 1;
        exponent++;
    }

    int clz = __builtin_clzll(mantissa) - (64 - 16);
    assert(clz >= 0);
    uint16_t mantissa_C;
    uint16_t exponent_C;
    if (clz > 5) {
        mantissa_C = mantissa << (clz - 5);
        exponent_C = exponent - (clz - 5);
    } else {
        mantissa_C = mantissa >> (5 - clz);
        exponent_C = _saturate_add(exponent, 5 - clz, MaxExponent);
    }

    if (exponent_C == MaxExponent) {
        mantissa_C = 0;
    }

    return _make_biased(sign, (uint16_t)exponent_C, (uint16_t)mantissa_C);
}

binary16 binary16_add(const binary16 a_, const binary16 b_)
{
    constexpr uint64_t ImpliedOne = 0b0000010000000000u;

    uint16_t a = a_.rep;
    uint16_t b = b_.rep;

    // TODO: I think I shouldn't need these explicit checks
    if (binary16_isnan(a_) || binary16_isnan(b_)) {
        return binary16_fromrep(Binary16_NAN);
    }
    if (binary16_iszero(a_)) {
        return b_;
    }
    if (binary16_iszero(b_)) {
        return a_;
    }

    uint16_t sign_A     = binary16_sign(a_);
    uint16_t sign_B     = binary16_sign(b_);
    // NOTE: subnormal exponent = -14 => +1 (biased)
    uint64_t exponent_A = _max((a & Binary16_ExponentMask) >> 10, 1);
    uint64_t exponent_B = _max((b & Binary16_ExponentMask) >> 10, 1);
    uint64_t exponent   = _min(exponent_A, exponent_B);
    uint64_t shift_A    = exponent_A - exponent;
    uint64_t shift_B    = exponent_B - exponent;
    uint64_t mantbits_A = a & Binary16_MantissaMask;
    uint64_t mantbits_B = b & Binary16_MantissaMask;
    uint64_t implied_A  = binary16_issubnormal(a_) ? 0 : ImpliedOne;
    uint64_t implied_B  = binary16_issubnormal(b_) ? 0 : ImpliedOne;
    uint64_t mantissa_A = _make_2scomp(sign_A, mantbits_A | implied_A) << shift_A;
    uint64_t mantissa_B = _make_2scomp(sign_B, mantbits_B | implied_B) << shift_B;
    uint64_t result     = mantissa_A + mantissa_B;
    uint16_t sign       = (result >> 63) & 0x1u;
    uint64_t mantissa   = _abs_2scomp(sign, result);
    assert(exponent_A >= exponent);
    assert(exponent_B >= exponent);
    assert(1 <= exponent_A && exponent_A < 32);
    assert(1 <= exponent_B && exponent_B < 32);
    assert(0 <= shift_A && shift_A < 32); // NOTE: exponent := [1, 32) so max shift = 31
    assert(0 <= shift_B && shift_B < 32);
    assert((mantissa >> 63) == 0 && "sign bit should not be set post abs()");
    return _normalize(sign, exponent, mantissa);
}

binary16 binary16_sub(const binary16 a_, const binary16 b_)
{
    return binary16_add(a_, binary16_neg(b_));
}

bool binary16_eq(const binary16 a, const binary16 b)
{
    if (binary16_isnan(a) || binary16_isnan(b)) {
        return false;
    }
    return a.rep == b.rep;
}

bool binary16_neq(const binary16 a, const binary16 b)
{
    return !binary16_eq(a, b);
}
