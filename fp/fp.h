#pragma once

#include <cstdint>
#include <cstddef>

// http://half.sourceforge.net/index.html#usage

struct binary16
{
    uint16_t rep;
};
static_assert(sizeof(binary16) == sizeof(uint16_t));

// DEBUG
uint16_t binary16_sign(binary16 x);
uint16_t binary16_exponent(binary16 x);
uint16_t binary16_mantissa(binary16 x);

binary16 binary16_make(uint16_t sgn, int16_t exp, uint16_t sig);
binary16 binary16_fromrep(uint16_t val);
uint16_t binary16_torep(binary16 val);

bool     binary16_isinf(binary16 x);
bool     binary16_isnan(binary16 x);
bool     binary16_iszero(binary16 x);
bool     binary16_issubnormal(binary16 x);
bool     binary16_isnormal(binary16 x);
bool     binary16_signbit(binary16 x);
float    binary16_tofloat(binary16 x);
binary16 binary16_fromfloat(float f);

binary16 binary16_add(binary16 a, binary16 b);

