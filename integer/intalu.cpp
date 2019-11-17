#include "intalu.h"

uint8_t  intalu_negu8(uint8_t x)
{
    // uint8_t result, i;

    // result = 0;


    // uint8_t result, a, b, r, c, i;
    // result = 0;
    // c = 0;

    return -x;
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
        // r = (!(a ^ b) & c) + ((a ^ b) & !c);
        // c = ( (a | b) & c) + ((a & b) & !c);
        result = result | (r << i);
    }

    return result;
}

int8_t intalu_adds8(int8_t x, int8_t y)
{
    return intalu_addu8(x, y);
}

int8_t intalu_subs8(int8_t x, int8_t y)
{
    return intalu_adds8(x, intalu_negs8(y));
}
