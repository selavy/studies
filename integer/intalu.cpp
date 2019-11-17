#include "intalu.h"

int8_t intalu_adds8(int8_t x, int8_t y)
{
    uint8_t result, a, b, r, c, i;

    result = 0;
    c = 0;
    for (i = 0; i < 8; ++i) {
        a = x & (1u << i) ? 1 : 0;
        b = y & (1u << i) ? 1 : 0;
        c = ((a & b) & !c) + ((a | b) & c);
        r = (!(a ^ b) & c) + ((a ^ b) & !c);
        result = result | (r << i);
    }

    return result;
}
