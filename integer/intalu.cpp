#include "intalu.h"

uint8_t  intalu_negu8(uint8_t x)
{
    // return ~x + 1;
    return intalu_addu8(~x, 1);
}

int8_t  intalu_negs8(int8_t x)
{
    return intalu_negu8(x);
}

uint8_t intalu_addu8(uint8_t x, uint8_t y)
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

int8_t intalu_adds8(int8_t x, int8_t y)
{
    return intalu_addu8(x, y);
}

uint8_t intalu_subu8(uint8_t x, uint8_t y)
{
    return intalu_adds8(x, intalu_negs8(y));
}

int8_t intalu_subs8(int8_t x, int8_t y)
{
    return intalu_subu8(x, y);
}

uint8_t intalu_mulu8(uint8_t x, uint8_t y)
{
    uint16_t a = x;
    uint16_t b = y;
    uint16_t result = 0;
    while (b > 0) {
        result += a * !!(b & 1);
        b     >>= 1;
        a     <<= 1;
    }
    // TODO: handle overflow
    return result;
}

uint16_t intalu_mulu16(uint16_t x, uint16_t y)
{
    return x * y;
}

int8_t  intalu_muls8( int8_t x,  int8_t y)
{
    int16_t x_neg = x < 0;
    int16_t y_neg = y < 0;
    int16_t x_pos = x_neg ? -((int16_t)x) : x;
    int16_t y_pos = y_neg ? -((int16_t)y) : y;
    int16_t multu = intalu_mulu16(x_pos, y_pos);
    int16_t result = x_neg ^ y_neg ? -multu : multu;
    if (result > 127)
        return 127;
    else if (result < -128)
        return x == -128 || y == -128 ? -128 : 0;
    else
        return result;

    // uint16_t mask = x_neg ^ y_neg ? 0xFFFFu : 0x0000u;
    // return (mask & -multu) | (~mask & multu);
}
