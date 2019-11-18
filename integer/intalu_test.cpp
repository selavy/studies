// #include <cgreen/cgreen.h>
#include <catch2/catch.hpp>
#include "intalu.h"
#include <vector>
#include <limits>
#include <random>

TEST_CASE("Negative 8-bit signed integer -- exhaustive")
{
    for (int i = -128; i < 128; ++i) {
        int8_t a = i;
        int8_t expect = -a;
        int8_t result = intalu_negs8(a);
        REQUIRE(result == expect);
    }
}

TEST_CASE("Add positive 8-bit signed integer with 1")
{
    std::vector<int8_t> vs = { 0, 1, 2, 3, 4, 5, 6, 100, 125 };
    for (int8_t a : vs) {
        int8_t b = 1;
        int8_t expect = a + b;
        int8_t result = intalu_adds8(a, b);
        REQUIRE(result == expect);
    }
}

TEST_CASE("Positive 8-bit signed integer add -- exhaustive")
{
    for (int i = -128; i < 128; ++i) {
        for (int j = -128; j < 128; ++j) {
            int8_t a = i;
            int8_t b = j;

            SECTION("a + b")
            {
                int8_t expect = a + b;
                int8_t result = intalu_adds8(a, b);
                REQUIRE(result == expect);
            }

            SECTION("b + a")
            {
                int8_t expect = b + a;
                int8_t result = intalu_adds8(b, a);
                REQUIRE(result == expect);
            }
        }
    }
}

TEST_CASE("Positive 8-bit signed integer overflow -- 2s compliment overflow behavior")
{

    SECTION("Add 1 to CHAR_MAX")
    {
        int8_t a = 127;
        int8_t b = 1;
        int8_t expect = -128;
        int8_t result = intalu_adds8(a, b);
        REQUIRE(result == expect);
    }

    SECTION("Add 5 to CHAR_MAX")
    {
        int8_t a = 127;
        int8_t b = 5;
        int8_t expect = -124;
        int8_t result = intalu_adds8(a, b);
        REQUIRE(result == expect);
    }

    SECTION("Add CHAR_MAX + CHAR_MAX")
    {
        int8_t a = 127;
        int8_t b = 127;
        int8_t expect = -128 + 126; // == -2
        int8_t result = intalu_adds8(a, b);
        REQUIRE(result == expect);
    }
}

TEST_CASE("Add positive 8-bit signed integers with -1")
{
    std::vector<int8_t> vs = { 0, 1, 2, 3, 4, 5, 6, 100, 126 };
    for (int8_t a : vs) {
        int8_t b = -1;
        int8_t expect = a + b;
        int8_t result = intalu_adds8(a, b);
        REQUIRE(result == expect);
    }
}

TEST_CASE("Add positive and negative 8-bit signed integers")
{
    std::vector<std::pair<int8_t, int8_t>> vs = {
        { 0, -1 },
        { 125, -125 },
        { 100, -127 },
        { 127, -128 },
        { 5, -128 },
        { 10, -15 },
        { 10, -5 },
    };

    for (auto [a, b] : vs) {
        SECTION("a + b") {
            int8_t expect = a + b;
            int8_t result = intalu_adds8(a, b);
            REQUIRE(result == expect);
        }

        SECTION("b + a") {
            int8_t expect = b + a;
            int8_t result = intalu_adds8(b, a);
            REQUIRE(result == expect);
        }
    }
}

TEST_CASE("8-bit signed integer underflow -- 2s compliment underflow behavior")
{
    SECTION("Add -1 to CHAR_MIN")
    {
        int8_t a = -128;
        int8_t b = -1;
        int8_t expect = 127;
        int8_t result = intalu_adds8(a, b);
        REQUIRE(result == expect);
    }

    SECTION("Add -5 to CHAR_MIN")
    {
        int8_t a = -128;
        int8_t b = -5;
        int8_t expect = 123;
        int8_t result = intalu_adds8(a, b);
        REQUIRE(result == expect);
    }

    SECTION("Add CHAR_MIN + CHAR_MIN")
    {
        int8_t a = -128;
        int8_t b = -128;
        int8_t expect = 0;
        int8_t result = intalu_adds8(a, b);
        REQUIRE(result == expect);
    }
}

TEST_CASE("Subtract 8-bit integers")
{
    std::vector<std::pair<int8_t, int8_t>> vs = {
        { 0, -1 },
        { 125, -125 },
        { 100, -127 },
        { 127, -128 },
        { 5, -128 },
        { 10, -15 },
        { 10, -5 },
        { 10, 10 },
        { 10, 5 },
        { 10, 15 },
        { 15, 25 },
    };

    for (auto [a, b] : vs) {
        SECTION("a - b")
        {
            int8_t expect = a - b;
            int8_t result = intalu_subs8(a, b);
            REQUIRE(result == expect);
        }

        SECTION("b - a")
        {
            int8_t expect = b - a;
            int8_t result = intalu_subs8(b, a);
            REQUIRE(result == expect);
        }
    }
}

TEST_CASE("Randomized 8-bit add and sub")
{
    for (int i = 0; i < 100; ++i) {
        int8_t a = rand();
        int8_t b = rand();

        INFO("a = " << static_cast<int>(a) << ", b = " << static_cast<int>(b));

        SECTION("a + b")
        {
            int8_t expect = a + b;
            int8_t result = intalu_adds8(a, b);
            REQUIRE(result == expect);
        }

        SECTION("b + a")
        {
            int8_t expect = b + a;
            int8_t result = intalu_adds8(b, a);
            REQUIRE(result == expect);
        }

        SECTION("a - b")
        {
            int8_t expect = a - b;
            int8_t result = intalu_subs8(a, b);
            REQUIRE(result == expect);
        }

        SECTION("b - a")
        {
            int8_t expect = b - a;
            int8_t result = intalu_subs8(b, a);
            REQUIRE(result == expect);
        }
    }
}

TEST_CASE("Positive 8-bit unsigned multiply")
{
    for (int i = 0; i < 256; ++i) {
        for (int j = 0; j < 256; ++j) {
            uint8_t a = i;
            uint8_t b = j;

            SECTION("a * b")
            {
                uint8_t expect = a*b;
                uint8_t result = intalu_mulu8(a, b);
                REQUIRE((int)expect == (int)result);
            }

            SECTION("b * a")
            {
                uint8_t expect = b*a;
                uint8_t result = intalu_mulu8(b, a);
                REQUIRE((int)expect == (int)result);
            }
        }
    }
}

TEST_CASE("Positive 8-bit signed multiply")
{
    std::vector<std::pair<int8_t, int8_t>> vs = {
        { 1, 1 },
        { 2, 3 },
        { 10, 4 },
        { 127, 1 },
        { 127, 2 },
        { 127, 3 },
        { 127, 4 },
        { 127, 127 },
        { -128, 1 },
        { -128, 0 },
        { -128, -1 },
        { -128, -128 },
    };

    for (auto [a, b] : vs) {
        SECTION("a * b")
        {
            int8_t expect = a * b;
            int8_t result = intalu_muls8(a, b);
            INFO("a = " << ((int)a) << ", b = " << ((int) b));
            REQUIRE((int)expect == (int)result);
        }

        SECTION("b * a")
        {
            int8_t expect = b * a;
            int8_t result = intalu_muls8(b, a);
            INFO("a = " << ((int)a) << ", b = " << ((int) b));
            REQUIRE((int)expect == (int)result);
        }
    }

    for (int i = -128; i < 128; ++i) {
        for (int j = -128; j < 128; ++j) {
            int8_t a = i;
            int8_t b = j;

            SECTION("a * b")
            {
                int8_t expect = a*b;
                int8_t result = intalu_muls8(a, b);
                REQUIRE((int)expect == (int)result);
            }

            SECTION("b * a")
            {
                int8_t expect = b*a;
                int8_t result = intalu_muls8(b, a);
                REQUIRE((int)expect == (int)result);
            }
        }
    }
}

TEST_CASE("Positive 8-bit unsigned divide")
{
    for (int i = 0; i < 256; ++i) {
        for (int j = 0; j < 256; ++j) {
            if (i == 0 || j == 0)
                continue;

            uint8_t a = i;
            uint8_t b = j;
            uint8_t expect = a / b;
            uint8_t result = intalu_divu8(a, b);
            REQUIRE((int)expect == (int)result);
        }
    }
}

TEST_CASE("Positive 8-bit signed divide")
{
    for (int i = 0; i < 128; ++i) {
        for (int j = 0; j < 128; ++j) {
            if (i == 0 || j == 0)
                continue;
            int8_t a = i;
            int8_t b = j;
            int8_t expect = a / b;
            int8_t result = intalu_divs8(a, b);
            INFO("a = " << ((int)a) << ", b = " << ((int)b));
            REQUIRE((int)expect == (int)result);
        }
    }
}

TEST_CASE("Negative 8-bit signed divide")
{
    for (int i = -128; i < 128; ++i) {
        for (int j = -128; j < 128; ++j) {
            if (i == 0 || j == 0)
                continue;
            int8_t a = i;
            int8_t b = j;
            int8_t expect = a / b;
            int8_t result = intalu_divs8(a, b);
            INFO("a = " << ((int)a) << ", b = " << ((int)b));
            REQUIRE((int)expect == (int)result);
        }
    }
}
