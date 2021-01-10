#pragma once

#include <cstdint>
#include <cstddef>

// http://half.sourceforge.net/index.html#usage

struct binary16
{
    uint16_t rep;
};
static_assert(sizeof(binary16) == sizeof(uint16_t));

// struct binary32
// {
//     uint32_t rep;
// };
// static_assert(sizeof(binary32) == sizeof(uint32_t));

// DEBUG
uint16_t binary16_sign(binary16 x);
int      binary16_exponent(binary16 x);
uint16_t binary16_mantissa(binary16 x);
int      binary16_desc(binary16 x, char* buf, int len);

// TODO: make these private
uint16_t _binary16_signbits(uint16_t x);
uint16_t _binary16_exponentbits(uint16_t x);
uint16_t _binary16_mantissabits(uint16_t x);

binary16 binary16_make(uint16_t sign, int16_t exponent, uint16_t mantissa);
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
binary16 binary16_next(binary16 x);
binary16 binary16_prev(binary16 x);

binary16 binary16_neg(binary16 a);
binary16 binary16_abs(binary16 a);
binary16 binary16_add(binary16 a, binary16 b);
binary16 binary16_sub(binary16 a, binary16 b);
bool     binary16_eq (binary16 a, binary16 b);
bool     binary16_neq(binary16 a, binary16 b);
bool     binary16_gt (binary16 a, binary16 b);
