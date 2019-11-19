#include "intalu.h"

uint8_t intalu_neg_u8(uint8_t x)
{
    // return ~x + 1;
    return intalu_add_u8(~x, 1);
}

int8_t intalu_neg_i8(int8_t x)
{
    return intalu_neg_u8(x);
}

uint8_t intalu_add_u8(uint8_t x, uint8_t y)
{
    uint8_t result, a, b, r, c, i;
    result = 0;
    c = 0;
    for (i = 0; i < 8; ++i) {
        a = x & (1u << i) ? 1 : 0;
        b = y & (1u << i) ? 1 : 0;
        r = a ^ b ^ c;
        c = (a & b) + ((a ^ b)*c);
        result = result | (r << i);
    }
    return result;
}

int8_t intalu_add_i8(int8_t x, int8_t y)
{
    return intalu_add_u8(x, y);
}

uint8_t intalu_sub_u8(uint8_t x, uint8_t y)
{
    return intalu_add_i8(x, intalu_neg_i8(y));
}

int8_t intalu_sub_i8(int8_t x, int8_t y)
{
    return intalu_sub_u8(x, y);
}

static uint16_t _mulu8_ext_u16(uint16_t x, uint16_t y)
{
    uint16_t result = 0;

    // TODO: could do ctz to fastforward to 1s

    // while (y > 0) {
    //     result += x * !!(y & 1);
    //     y     >>= 1;
    //     x     <<= 1;
    // }

    // More hardware-ish:
    result += (x <<  0) * ((y >>  0) & 1);
    result += (x <<  1) * ((y >>  1) & 1);
    result += (x <<  2) * ((y >>  2) & 1);
    result += (x <<  3) * ((y >>  3) & 1);
    result += (x <<  4) * ((y >>  4) & 1);
    result += (x <<  5) * ((y >>  5) & 1);
    result += (x <<  6) * ((y >>  6) & 1);
    result += (x <<  7) * ((y >>  7) & 1);
    result += (x <<  8) * ((y >>  8) & 1);
    result += (x <<  9) * ((y >>  9) & 1);
    result += (x << 10) * ((y >> 10) & 1);
    result += (x << 11) * ((y >> 11) & 1);
    result += (x << 12) * ((y >> 12) & 1);
    result += (x << 13) * ((y >> 13) & 1);
    result += (x << 14) * ((y >> 14) & 1);
    result += (x << 15) * ((y >> 15) & 1);

    return result;
}

uint8_t intalu_mul_u8(uint8_t x, uint8_t y)
{
    return _mulu8_ext_u16(x, y);
}

int8_t intalu_mul_i8( int8_t x,  int8_t y)
{
    uint16_t x_neg  = x < 0;
    uint16_t y_neg  = y < 0;
    uint16_t is_neg = !!(x_neg) ^ !!(y_neg);
    uint16_t a      = x_neg ? -((int16_t)x) : x;
    uint16_t b      = y_neg ? -((int16_t)y) : y;
    uint16_t result = _mulu8_ext_u16(a, b);
    return is_neg ? -result : result;
}

uint8_t intalu_div_u8(uint8_t n, uint8_t d)
{
    uint8_t quot = 0;
    uint8_t rem  = 0;
    for (int i = 7; i >= 0; --i) {
        rem = (rem << 1) | ((n >> i) & 1);
        if (rem >= d) {
            rem  -= d;
            quot |= 1u << i;
        }
    }
    return quot;
}

uint8_t _abs_u8(int8_t x)
{
    if (x >= 0)
        return x;
    else if (x == -128)
        return 128;
    else
        return intalu_neg_i8(x);
}

int8_t intalu_div_i8(int8_t n, int8_t d)
{
    uint8_t quot = intalu_div_u8(_abs_u8(n), _abs_u8(d));
    return ((n < 0) ^ (d < 0)) ? -quot : quot;
}

uint8_t intalu_fma_u8(uint8_t a, uint8_t b, uint8_t c)
{
    return intalu_add_u8(intalu_mul_u8(a, b), c);
}

int8_t intalu_fma_i8(int8_t a, int8_t b, int8_t c)
{
    return intalu_add_i8(intalu_mul_i8(a, b), c);
}
