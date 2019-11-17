// #include <cgreen/cgreen.h>
#include <catch2/catch.hpp>
#include "intalu.h"
#include <vector>


TEST_CASE("Add 8-bit signed integer")
{
    std::vector<int8_t> vs = { 1, 2, 3, 4, 5, 6, 100, 125 };
    for (int8_t a : vs) {
        SECTION("8-bit add") {
            int8_t b = 1;
            int8_t expect = a + b;
            int8_t result = intalu_adds8(a, b);
            INFO("a = " << ((int)a) << ", b = " << ((int)b) << ", result = " << ((int)result));
            REQUIRE(result == expect);
        }
    }
}
