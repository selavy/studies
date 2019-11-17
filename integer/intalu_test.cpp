// #include <cgreen/cgreen.h>
#include <catch2/catch.hpp>
#include "intalu.h"
#include <vector>
#include <limits>
#include <random>


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
