#pragma once

#include <cstdint>
#include <cstddef>

constexpr size_t Binary16_ExpBias = 15;

struct binary16
{
    union
    {
        // TODO: remove
        struct {
            uint16_t sign : 1;
            uint16_t exp  : 5;
            uint16_t frac : 10;
        } b;

        uint16_t rep;
    };
};
static_assert(sizeof(binary16) == sizeof(uint16_t));

bool     binary16_isinf(binary16 x);
bool     binary16_isnan(binary16 x);
bool     binary16_iszero(binary16 x);
bool     binary16_issubnormal(binary16 x);
bool     binary16_isnormal(binary16 x);
bool     binary16_signbit(binary16 x);
binary16 binary16_add(binary16 a, binary16 b);
