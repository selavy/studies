#include "fp.h"
#include <cstring>
#include <cassert>

// TEMP TEMP TEMP
#include <cstdio>
#include <cinttypes>

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

constexpr int Binary32_SignBits     = 1;
constexpr int Binary32_ExponentBits = 8;
constexpr int Binary32_MantissaBits = 23;
constexpr int Binary32_ExponentBias = 127;

constexpr int Binary16_SignBits     = 1;
constexpr int Binary16_ExponentBits = 5;
constexpr int Binary16_MantissaBits = 10;
constexpr int Binary16_ExponentBias = 15;

constexpr uint16_t Binary16_NAN   = 0b0111'1111'1111'1111u;
constexpr uint16_t Binary16_INF   = 0b0111'1100'0000'0000u;
constexpr uint16_t Binary16_NINF  = 0b1111'1100'0000'0000u;
constexpr uint16_t Binary16_ZERO  = 0b0000'0000'0000'0000u;
constexpr uint16_t Binary16_NZERO = 0b1000'0000'0000'0000u;

constexpr int Binary16_MantissaShift = 0;
constexpr int Binary16_ExponentShift = Binary16_MantissaShift + Binary16_MantissaBits;
constexpr int Binary16_SignShift     = Binary16_ExponentShift + Binary16_ExponentBits;

constexpr uint16_t Binary16_SignMask     = 0b1000'0000'0000'0000u;
constexpr uint16_t Binary16_ExponentMask = 0b0111'1100'0000'0000u;
constexpr uint16_t Binary16_MantissaMask = 0b0000'0011'1111'1111u;
constexpr int      Binary16_MinBiasNormalExponent =  1;
constexpr int      Binary16_MaxBiasNormalExponent = 30;
constexpr int      Binary16_MinBiasExponent =  0;
constexpr int      Binary16_MaxBiasExponent = 31;
constexpr int      Binary16_MinExponent  = Binary16_MinBiasExponent - Binary16_ExponentBias;
constexpr int      Binary16_MaxExponent  = Binary16_MaxBiasExponent - Binary16_ExponentBias;
constexpr int      Binary16_MinNormalExponent = Binary16_MinBiasNormalExponent - Binary16_ExponentBias;
constexpr int      Binary16_MaxNormalExponent = Binary16_MaxBiasNormalExponent - Binary16_ExponentBias;
constexpr uint16_t Binary16_MaxNormalMantissa = Binary16_MantissaMask - 1;
constexpr uint16_t Binary16_MaxMantissa       = Binary16_MantissaMask;

// TODO: remove these:
constexpr int Binary16_ExpBias = Binary16_ExponentBias;  // TODO: remove
constexpr int Binary32_ExpBias = Binary32_ExponentBias;  // TODO: remove

static_assert(Binary16_MaxBiasExponent == ((1 << Binary16_ExponentBits) - 1));
static_assert(__builtin_popcount(Binary16_SignMask)     == Binary16_SignBits);
static_assert(__builtin_popcount(Binary16_ExponentMask) == Binary16_ExponentBits);
static_assert(__builtin_popcount(Binary16_MantissaMask) == Binary16_MantissaBits);
static_assert(Binary16_SignShift + Binary16_SignBits == 16);
static_assert(Binary16_MantissaMask == (((uint16_t(1) << Binary16_ExponentShift) - 1) & ~0));
static_assert(Binary16_ExponentMask == (((uint16_t(1) << Binary16_SignShift    ) - 1) & ~Binary16_MantissaMask));
static_assert(Binary16_SignMask     == (((uint16_t(1) << 16                    ) - 1) & ~(Binary16_MantissaMask | Binary16_ExponentMask)));

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

static uint64_t _min_u64(uint64_t x, uint64_t y)
{
    return x < y ? x : y;
}

static uint64_t _max_u64(uint64_t x, uint64_t y)
{
    return x > y ? x : y;
}

static uint16_t _min_u16(uint16_t x, uint16_t y)
{
    return x < y ? x : y;
}

static uint16_t _max_u16(uint16_t x, uint16_t y)
{
    return x > y ? x : y;
}

static uint64_t _max_int(uint64_t x, uint64_t y)
{
    return x > y ? x : y;
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
    constexpr uint32_t MaxExponent = Binary32_ExpBias + Binary16_ExponentBias;
    constexpr uint32_t MinExponent = Binary32_ExpBias - Binary16_ExponentBias;

    uint32_t rep;
    memcpy(&rep, &f, sizeof(rep));
    uint32_t sign     = (rep >> 31) & 0x01u;
    uint32_t exponent = (rep >> 23) & 0xFFu;
    uint32_t mantissa = rep & 0x7FFFFFu;

    uint16_t exp;
    uint16_t sig;
    if (exponent == 0x00u) {
        exp = 0b00000u;
        sig = mantissa >> (23 - 10);
    } else if (exponent == 0xFFu) {
        exp = 0b11111u;
        sig = mantissa >> (23 - 10);
    } else if (exponent < MinExponent) {
        // TODO: is it true that all subnormal floats are all closer to +/-0
        // than they are to +/- small 16-bit subnormal value?

        // round to zero if exponent too negative
        exp = 0b00000u;
        sig = 0b00000u;
    } else if (exponent > MaxExponent) {
        // round to +/- inf if exponent too positive
        exp = 0b11111u;
        sig = mantissa == 0 ? 0x0u : 0x1u;
    } else if (exponent == MinExponent) {
        // this is _not_ a subnormal float so it still has an implied 1 at the
        // front of the mantissa, but the binary16 will be subnormal

        // put the implied 1 back on
        mantissa = mantissa | 1u << Binary32_MantissaBits;

        // round the 10th decimal place
        mantissa += 0b1'1111'1111'1111;
        // TODO: handle overflowing

        // shift right to remove the extraneous bits
        constexpr int shift = ((Binary32_MantissaBits + 1) - Binary16_MantissaBits);
        mantissa = mantissa >> shift;

        exp = 0b00000u;
        sig = mantissa;
    } else {
        // round 10th decimal place:
        // binary32 mantissa = ____ ____ ____ ____ ____ ___
        // binary16 mantissa = ____ ____ __xx xxxx xxxx xxx
        mantissa += 0b1111'1111'1111;
        if (mantissa > 0b111'1111'1111'1111'1111'1111) {
            exponent++;
            // exponent--;
            mantissa = 0;
            // // TODO: undo instead of rounding to infinity?
            // // rounding overflowed to infinity
            // if (exponent >= Binary16_MaxBiasExponent) {
            //     exponent = Binary16_MaxBiasExponent;
            //     mantissa = 0;
            // }
        }
        exp = exponent - Binary32_ExpBias + Binary16_ExpBias;
        sig = mantissa >> (Binary32_MantissaBits - Binary16_MantissaBits);
    }

    const uint16_t sgn = static_cast<uint16_t>(sign);

    binary16 rv;
    rv.rep = ((sgn & 0x1u  ) << 15)
           | ((exp & 0x1Fu ) << 10)
           | ((sig & 0x3FFu) <<  0);
    return rv;
}

binary16 binary16_next(const binary16 x_)
{
    if (binary16_isnan(x_) || binary16_isinf(x_)) {
        return x_;
    }
    uint16_t x        = x_.rep;
    uint16_t sign     = _binary16_signbits(x);
    uint16_t mbits    = _binary16_mantissabits(x);
    uint16_t ebits    = _binary16_exponentbits(x);
    uint16_t mantissa = mbits < Binary16_MaxMantissa ? mbits + 1 : 0;
    uint16_t exponent = mbits < Binary16_MaxMantissa ? ebits : _min_u16(ebits + 1, Binary16_MaxBiasExponent);
    assert(sign == 0x0u || sign == 0x1u);
    assert(0 <= mantissa && mantissa <= Binary16_MaxMantissa);
    assert(0 <= exponent && exponent <= Binary16_MaxBiasExponent);
    return _make_biased(sign, exponent, mantissa);
}

binary16 binary16_prev(const binary16 x_)
{
    if (binary16_isnan(x_) || binary16_isinf(x_)) {
        return x_;
    }
    uint16_t x        = x_.rep;
    uint16_t sign     = _binary16_signbits(x);
    uint16_t mbits    = _binary16_mantissabits(x);
    uint16_t ebits    = _binary16_exponentbits(x);
    uint16_t mantissa = mbits > 0 ? mbits - 1 : Binary16_MaxMantissa;
    uint16_t exponent = mbits > 0 ? ebits : (ebits > 0 ? ebits - 1 : 0);
    assert(sign == 0x0u || sign == 0x1u);
    assert(0 <= mantissa && mantissa <= Binary16_MaxMantissa);
    assert(0 <= exponent && exponent <= Binary16_MaxBiasExponent);
    return _make_biased(sign, exponent, mantissa);
}

uint16_t binary16_sign(const binary16 x_)
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

static uint16_t _signbits(const uint16_t x)
{
    return x >> (Binary16_ExponentBits + Binary16_MantissaBits);
}

static uint16_t _exponentbits(const uint16_t x)
{
    return (x & Binary16_ExponentMask) >> Binary16_MantissaBits;
}

static uint16_t _mantissabits(const uint16_t x)
{
    return x & Binary16_MantissaMask;
}

// TODO: remove
uint16_t _binary16_signbits(uint16_t x)     { return _signbits(x); }
uint16_t _binary16_exponentbits(uint16_t x) { return _exponentbits(x); }
uint16_t _binary16_mantissabits(uint16_t x) { return _mantissabits(x); }

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

static uint16_t _make_2scomp_u16(uint16_t sign, uint16_t val)
{
    return sign ? (val ^ ~uint16_t(0)) + uint16_t(1) : val;
}

static uint16_t _abs_2scomp_u16(uint16_t sign, uint16_t val)
{
    return _make_2scomp_u16(sign, val);
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

binary16 binary16_abs(const binary16 a_)
{
    uint16_t a = a_.rep;
    return binary16_fromrep(a & ~Binary16_SignMask);
}

uint64_t _saturate_add(uint64_t a, uint64_t b, uint64_t max_value)
{
    uint64_t min_elem   = _min_u64(a, b);
    uint64_t max_elem   = _max_u64(a, b);
    uint64_t max_amount = max_value - max_elem;
    uint64_t add_value  = _min_u64(max_amount, min_elem);
    uint64_t result = max_elem + add_value;
    assert(0 <= result && result <= max_value);
    return result;
}

uint64_t slr(uint64_t x, int shift)
{
    return shift >= 0 ? x >> shift : x << -shift;
}

binary16 _normalize(const uint16_t sign, const uint64_t exponent, const uint64_t mantissa)
{
    // preconditions:
    //     + input is not NaN

    constexpr uint16_t MaxExponent = 0b11111u;
    constexpr uint16_t MinExponent = 0b00000u;

    // 1) Shift mantissa to 12 places with exponent adjusted accordingly
    // 2) Add 1 to round based on the final place
    // 3) Shift mantissa to 11 places with exponent adjusted accordingly
    // // TODO: handle subnormals
    // 4) If not subnormal, mask off top bit of mantissa

    // 64-16         = 48
    // mantissa      = 48 x 0's + 0010 0100 1111 0010
    // clz(mantissa) = 48 + 2 = 50
    // want          = 48 x 0's + 0000 1001 0011 1100
    // desired clz   = 48 + 4 = 52
    // need shift    = 2 => 52 - clz(mantissa)
    // (positive shift = shift right = increase exponent)

    constexpr int Digits0 = 64;
    // constexpr int Digits1 = 12;
    constexpr int Digits1 = 13;
    constexpr int Digits2 = 11; // N.B. 1 implied digit + 10 digits precision

    // TODO: handle underflow for negative shift -> round to subnormal
    // TODO: handle overflow -> round to infinity
    const int      clz       = __builtin_clzll(mantissa);
    const int      shift     = (Digits0 - Digits1) - clz;
    const uint64_t mantissa1 = slr(mantissa, shift) + 1;
    const uint64_t exponent1 = exponent + shift;
    const uint64_t mantissa2 = mantissa1 >> (Digits1 - Digits2);
    const uint64_t exponent2 = exponent1 +  (Digits1 - Digits2);
    const uint16_t mantissa3 = (uint16_t)mantissa2;
    const uint16_t exponent3 = (uint16_t)exponent2;

    // assert((mantissa2 & ~0x3FFu) == 0);
    // assert((0b00000u      <= exponent2) && (exponent2 <= 0b11111u     ));
    // assert((0b0000000000u <= mantissa2) && (mantissa2 <= 0b1111111111u));
    // assert((sign == 0) || (sign == 1));

#if 0
    printf("_normalize:");
    printf("\n\tDigits0   = %d", Digits0);
    printf("\n\tDigits1   = %d", Digits1);
    printf("\n\tDigits2   = %d", Digits2);
    printf("\n\tsign      = %u", sign);
    printf("\n\texponent  = 0x%016" PRIx64, exponent);
    printf("\n\tmantissa  = 0x%016" PRIx64, mantissa);
    printf("\n\tclz       = %d", clz);
    printf("\n\tshift     = %d", shift);
    printf("\n\texponent1 = 0x%016" PRIx64, exponent1);
    printf("\n\tmantissa1 = 0x%016" PRIx64, mantissa1);
    printf("\n\texponent2 = 0x%016" PRIx64, exponent2);
    printf("\n\tmantissa2 = 0x%016" PRIx64, mantissa2);
    printf("\n\texponent3 = 0x%02"  PRIx16, exponent3);
    printf("\n\tmantissa3 = 0x%02"  PRIx16, mantissa3);
    printf("\n");
#endif

    return _make_biased(sign, exponent3, mantissa3);

    // constexpr uint16_t MaxExponent = 0b11111;
    // constexpr uint16_t MinExponent = 0b00001;

    // const auto mantissa_orig = mantissa;
    // const auto exponent_orig = exponent;

    // assert((mantissa >> 63) == 0u && "top-bit shouldn't be set on mantissa!");

    // printf("Incoming mantissa: 0x%08" PRIx64 "\n", mantissa);
    // while ((mantissa & ~0xFFFFu) != 0) {
    //     mantissa >>= 1;
    //     exponent++;
    // }
    // int clz = __builtin_clzll(mantissa) - (64 - 16);
    // assert(clz >= 0);
    // uint16_t mantissa_C;
    // uint16_t exponent_C;

    // // TEMP TEMP TEMP
    // printf("_normalize:");
    // printf("\n\tsign      = %u", sign);
    // printf("\n\texponent0 = 0x%016" PRIx64, exponent_orig);
    // printf("\n\tmantissa0 = 0x%016" PRIx64, mantissa_orig);
    // printf("\n\texponent1 = 0x%016" PRIx64, exponent);
    // printf("\n\tmantissa1 = 0x%016" PRIx64, mantissa);
    // printf("\n\tclz       = %d"      , clz);
    // printf("\n");

    // if (clz > 5) {
    //     uint16_t shift = clz - 5;
    //     if (exponent > shift) {
    //         mantissa_C = mantissa << shift;
    //         exponent_C = exponent -  shift;
    //     } else {
    //         // subnormal
    //         exponent_C = 0;
    //         mantissa_C = mantissa;
    //     }
    // } else {
    //     mantissa_C = mantissa >> (5 - clz);
    //     exponent_C = _saturate_add(exponent, 5 - clz, MaxExponent);
    // }

    // if (exponent_C == MaxExponent) {
    //     // TODO: probably should generate a standard form of INF
    //     mantissa_C = 0;
    // }

    // return _make_biased(sign, (uint16_t)exponent_C, (uint16_t)mantissa_C);
}

binary16 binary16_add(const binary16 a_, const binary16 b_)
{
    constexpr uint64_t ImpliedOne = 0b0000010000000000u;

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

    const binary16 aa = binary16_gt(binary16_abs(a_), binary16_abs(b_)) ? a_ : b_;
    const binary16 bb = binary16_gt(binary16_abs(a_), binary16_abs(b_)) ? b_ : a_;
    const uint16_t a = aa.rep;
    const uint16_t b = bb.rep;

    const uint16_t sign_A     = _signbits(a);
    const uint16_t sign_B     = _signbits(b);
    const uint16_t exponent_A = _exponentbits(a);
    const uint16_t exponent_B = _exponentbits(b);
    const uint16_t exponent   = _max_u16(exponent_A, exponent_B);
    const int      shift_A    = exponent - exponent_A;
    const int      shift_B    = exponent - exponent_B;
    const uint16_t mantissa_A1 = _mantissabits(a);
    const uint16_t mantissa_B1 = _mantissabits(b);
    const uint16_t implied_A   = binary16_issubnormal(aa) ? 0 : ImpliedOne;
    const uint16_t implied_B   = binary16_issubnormal(bb) ? 0 : ImpliedOne;
    const uint16_t mantissa_A2 = (mantissa_A1 | implied_A) << 2;
    const uint16_t mantissa_B2 = (mantissa_B1 | implied_B) << 2;
    const uint16_t mantissa_A3 = _make_2scomp_u16(sign_A, mantissa_A2) >> shift_A;
    const uint16_t mantissa_B3 = _make_2scomp_u16(sign_B, mantissa_B2) >> shift_B;
    const uint16_t result      = mantissa_A3 + mantissa_B3;
    const uint16_t sign_C      = (result >> 15) & 0x1u;
    const uint16_t mantissa_C1 = _abs_2scomp_u16(sign_C, result);
    const uint16_t mantissa_C2 = mantissa_C1 + 1; // round
    const uint16_t mantissa_C  = mantissa_C2 >> 2;
    const uint16_t exponent_C  = exponent;

    // TODO: normalize mantissa/exponent C
    const int desired_digits = 10;
    const int current_digits = 31 - __builtin_clz(mantissa_C);
    const int shift          = current_digits - desired_digits;
    // TODO: handle subnormals (exponent == -14 => exponent_biased = 0) and exponent overflowing
    const uint16_t mantissa_D  = shift >= 0 ? mantissa_C >> shift : mantissa_C << shift;
    const uint16_t exponent_D  = exponent_C + shift;
    const uint16_t sign_D      = sign_C;

#if 0
    printf("binary16_add()");
    printf("\n\ta_ = %0.8f", binary16_tofloat(a_));
    printf("\n\tb_ = %0.8f", binary16_tofloat(b_));
    printf("\n\ta  = %0.8f", binary16_tofloat(a));
    printf("\n\tb  = %0.8f", binary16_tofloat(b));
    printf("\n\tsign_A      = %u = 0x%04X", sign_A, sign_A);
    printf("\n\tsign_B      = %u = 0x%04X", sign_B, sign_B);
    printf("\n\texponent_A  = %u = 0x%04X", exponent_A, exponent_A);
    printf("\n\texponent_B  = %u = 0x%04X", exponent_B, exponent_B);
    printf("\n\texponent    = %u = 0x%04X", exponent, exponent);
    printf("\n\tshift_A     = %u = 0x%04X", shift_A, shift_A);
    printf("\n\tshift_B     = %u = 0x%04X", shift_B, shift_B);
    printf("\n\tmantissa_A1 = %u = 0x%04X", mantissa_A1, mantissa_A1);
    printf("\n\tmantissa_B1 = %u = 0x%04X", mantissa_B1, mantissa_B1);
    printf("\n\timplied_A   = %u = 0x%04X", implied_A, implied_A);
    printf("\n\timplied_B   = %u = 0x%04X", implied_B, implied_B);
    printf("\n\tmantissa_A2 = %u = 0x%04X", mantissa_A2, mantissa_A2);
    printf("\n\tmantissa_B2 = %u = 0x%04X", mantissa_B2, mantissa_B2);
    printf("\n\tmantissa_A3 = %u = 0x%04X", mantissa_A3, mantissa_A3);
    printf("\n\tmantissa_B3 = %u = 0x%04X", mantissa_B3, mantissa_B3);
    printf("\n\tresult      = %u = 0x%04X", result, result);
    printf("\n\tsign_C      = %u = 0x%04X", sign_C, sign_C);
    printf("\n\tmantissa_C1 = %u = 0x%04X", mantissa_C1, mantissa_C1);
    printf("\n\tmantissa_C2 = %u = 0x%04X", mantissa_C2, mantissa_C2);
    printf("\n\tmantissa_C  = %u = 0x%04X", mantissa_C , mantissa_C );
    printf("\n\texponent_C  = %u = 0x%04X", exponent_C, exponent_C);
    printf("\n\tdesired_digits = %d", desired_digits);
    printf("\n\tcurrent_digits = %d", current_digits);
    printf("\n\tshift          = %d", shift);
    printf("\n\tmantissa       = %u = 0x%04X", mantissa_D, mantissa_D);
    printf("\n\texponent       = %u = 0x%04X", exponent_D, exponent_D);
    printf("\n\tsign           = %u", sign_D);
    printf("\n");
#endif

    assert(!binary16_gt(binary16_abs(bb), binary16_abs(aa)));
    assert(exponent_A >= exponent_B);
    assert(exponent_A == exponent);
    assert(shift_A == 0);
    assert(shift_B >= 0);

    // return binary16_fromrep(0);
    return _make_biased(sign_D, exponent_D, mantissa_D);

#if 0
    uint16_t sign_A     = binary16_sign(a_);
    uint16_t sign_B     = binary16_sign(b_);
    // NOTE: subnormal exponent = -14 => +1 (biased)
    uint64_t exponent_A = _max((a & Binary16_ExponentMask) >> 10, 1);
    uint64_t exponent_B = _max((b & Binary16_ExponentMask) >> 10, 1);
    // TODO: should use the larger of the 2 exponents because it has more
    //       significant figures
    uint64_t exponent   = _min_u64(exponent_A, exponent_B);
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
#endif
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

bool binary16_gt (const binary16 a, binary16 b)
{
    if (binary16_isnan(a) || binary16_isnan(b)) {
        return false;
    }
    // TODO: finish and handle negative numbers
    assert(!binary16_sign(a));
    assert(!binary16_sign(b));
    return a.rep > b.rep;
}
