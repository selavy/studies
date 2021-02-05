#include <catch2/catch.hpp>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <iomanip>

#include "fp.h"

#include "half.hpp"

using namespace half_float::literal;

#define U16(x) uint16_t(x)

std::string dump_u16(uint16_t x)
{
    std::string result(21, '0');
    size_t i = 0;
    size_t index = 15;
    result[i++] = '0';
    for (size_t z = 0; z < 4; ++z) {
        result[i++] = ' ';
        for(size_t j = 0; j < 4; ++j) {
            result[i++] = (x & (1u << index--)) != 0 ? '1' : '0';
        }
    }
    result[1] = 'b';
    return result;
}

std::string dump_u32(uint32_t x)
{
    constexpr size_t M = 32;
    constexpr size_t N = M / 4;
    std::string result(2 + 5*(N-1) + 4, '0');
    size_t i = 0;
    size_t index = M-1;
    result[i++] = '0';
    for (size_t z = 0; z < N; ++z) {
        result[i++] = ' ';
        for(size_t j = 0; j < 4; ++j) {
            result[i++] = (x & (1u << index--)) != 0 ? '1' : '0';
        }
    }
    result[1] = 'b';
    return result;
}

std::string dump_f32(float x)
{
    uint32_t v;
    memcpy(&v, &x, sizeof(v));
    return dump_u32(v);
}

struct FP
{
    struct {
        uint32_t sig : 23;
        uint32_t exp : 8;
        uint32_t sgn : 1;
    } __attribute__((packed)) r;
};


TEST_CASE("binary16_tofloat")
{
    struct Test
    {
        float    expect;
        uint16_t sign;
        int16_t  exponent;
        uint16_t mantissa;
    };
    std::vector<Test> tests = {
        { .expect=5.5              , .sign=0, .exponent=2  , .mantissa=0b01'1000'0000 },
        { .expect=-5.5             , .sign=1, .exponent=2  , .mantissa=0b01'1000'0000 },
        { .expect=1.0              , .sign=0, .exponent=0  , .mantissa=0b00'0000'0000 },
        { .expect=-1.0             , .sign=1, .exponent=0  , .mantissa=0b00'0000'0000 },
        { .expect=0.99951172       , .sign=0, .exponent=-1 , .mantissa=0b11'1111'1111 }, // largest number less than one
        { .expect=0.00006103515625 , .sign=0, .exponent=-14, .mantissa=0b00'0000'0000 }, // smallest positive normal number
        { .expect=-0.00006103515625, .sign=1, .exponent=-14, .mantissa=0b00'0000'0000 },
        { .expect=65504            , .sign=0, .exponent=15 , .mantissa=0b11'1111'1111 }, // large normal number
        { .expect=-65504           , .sign=1, .exponent=15 , .mantissa=0b11'1111'1111 },
        { .expect=1.00097656       , .sign=0, .exponent=0  , .mantissa=0b00'0000'0001 }, // smallest number > 1
        { .expect=-1.00097656      , .sign=1, .exponent=0  , .mantissa=0b00'0000'0001 },
        { .expect=0.33325195       , .sign=0, .exponent=-2 , .mantissa=0b01'0101'0101 },
    };

    for (auto&& t : tests) {
        auto val    = binary16_make(t.sign, t.exponent, t.mantissa);
        auto expect = t.expect;
        auto result = binary16_tofloat(val);
        CHECK(result == expect);
    }
}

TEST_CASE("binary16 rountrip #2")
{
    std::vector<uint16_t> cases = {
        0b0100011010100000, // 6.625
        0b1100011010100000, // 6.625
        0b0101011001001010, // 100.625
        0b1101011001001010, // -100.625
        0b0100010111100110, // 5.8984375
        0b1100010111100110, // -5.8984375
        0b0100010111010110, // 5.8359375
        0b1100010111010110, // -5.8359375
    };

    for (auto d : cases) {
        auto b = binary16_fromrep(d);
        auto f = binary16_tofloat(b);
        auto o = binary16_fromfloat(f);
        auto e = binary16_torep(o);
        CHECK(d == e);
    }
}

TEST_CASE("binary16 float roundtrip")
{
    std::vector<float> cases = {
        5.5f,
        -5.5f,
        1.0f,
        -1.0f,
        2.0f,
        -2.0f,
        0.00006103515625f,
        -0.00006103515625f,
        65504.0f,
        -65504.0f,
        1.00097656f,
        -1.00097656f,
        0.33325195f,
        0.0f,
        -0.0f,
        INFINITY,
        -INFINITY,
    };

    for (auto expect : cases) {
        binary16 b = binary16_fromfloat(expect);
        float result = binary16_tofloat(b);
        CHECK(result == expect);
    }

    SECTION("NaN")
    {
        // Since NaNs don't compare equal, have to do it separately
        float expect = NAN;
        binary16 b = binary16_fromfloat(expect);
        float result = binary16_tofloat(b);
        CHECK(std::isnan(expect) == std::isnan(result));
    }
}

TEST_CASE("binary16_isinf")
{
    std::vector<std::pair<uint16_t, bool>> cases = {
        // value                | isinf
        { 0b0111'1100'0000'0000u, true   }, // +inf
        { 0b1111'1100'0000'0000u, true   }, // -inf
        { 0b0000'0000'0000'0000u, false  },
        { 0b1011'1100'0000'0000u, false  },
        { 0b1101'1100'0000'0000u, false  },
        { 0b1110'1100'0000'0000u, false  },
        { 0b1111'0100'0000'0000u, false  },
        { 0b1111'1000'0000'0000u, false  },
        { 0b0111'1110'0000'0000u, false  },
        { 0b0111'1101'0000'0000u, false  },
        { 0b0111'1100'1000'0000u, false  },
        { 0b0111'1100'0100'0000u, false  },
        { 0b0111'1100'0010'0000u, false  },
        { 0b0111'1100'0001'0000u, false  },
        { 0b0111'1100'0000'1000u, false  },
        { 0b0111'1100'0000'0100u, false  },
        { 0b0111'1100'0000'0010u, false  },
        { 0b0111'1100'0000'0001u, false  },
    };

    for (auto&& [rep, expect] : cases) {
        binary16 x;
        x.rep = rep;
        CHECK(binary16_isinf(x) == expect);
    }
}

TEST_CASE("dump")
{
    uint16_t x =   0b0001'0101'1111'1000u;
    auto expect = "0b0001 0101 1111 1000";
    auto result = dump_u16(x);
    CHECK(result == expect);
}

TEST_CASE("binary16_isnan")
{
    std::vector<std::pair<uint16_t, bool>> cases = {
        // value                | isnan
        { 0b0111'1110'0000'0000u, true   },
        { 0b0111'1101'0000'0000u, true   },
        { 0b0111'1100'1000'0000u, true   },
        { 0b0111'1100'0100'0000u, true   },
        { 0b0111'1100'0010'0000u, true   },
        { 0b0111'1100'0001'0000u, true   },
        { 0b0111'1100'0000'1000u, true   },
        { 0b0111'1100'0000'0100u, true   },
        { 0b0111'1100'0000'0010u, true   },
        { 0b0111'1100'0000'0001u, true   },
        { 0b0111'1100'0000'0001u, true   },
        { 0b1111'1110'0000'0000u, true   },
        { 0b1111'1101'0000'0000u, true   },
        { 0b1111'1100'1000'0000u, true   },
        { 0b1111'1100'0100'0000u, true   },
        { 0b1111'1100'0010'0000u, true   },
        { 0b1111'1100'0001'0000u, true   },
        { 0b1111'1100'0000'1000u, true   },
        { 0b1111'1100'0000'0100u, true   },
        { 0b1111'1100'0000'0010u, true   },
        { 0b1111'1100'0000'0001u, true   },
        { 0b1111'1100'0000'0001u, true   },

        { 0b0111'1100'0000'0000u, false  }, // +inf
        { 0b1111'1100'0000'0000u, false  }, // -inf
        { 0b0011'1100'0000'0001u, false  },
        { 0b0101'1100'0000'0001u, false  },
        { 0b0110'1100'0000'0001u, false  },
        { 0b0111'0100'0000'0001u, false  },
        { 0b0111'1000'0000'0001u, false  },
        { 0b1011'1100'0000'0001u, false  },
        { 0b1101'1100'0000'0001u, false  },
        { 0b1110'1100'0000'0001u, false  },
        { 0b1111'0100'0000'0001u, false  },
        { 0b1111'1000'0000'0001u, false  },
    };

    for (auto&& [rep, expect] : cases) {
        binary16 x;
        x.rep = rep;
        CHECK(binary16_isnan(x) == expect);
    }
}

TEST_CASE("binary16_iszero")
{
    std::vector<std::pair<uint16_t, bool>> cases = {
        // value                | isnan
        { 0b0000'0000'0000'0000u, true   }, // +0.0
        { 0b1000'0000'0000'0000u, true   }, // -0.0

        { 0b0111'1100'1000'0000u, false  }, // nan
        { 0b0111'1100'0100'0000u, false  }, // nan
        { 0b0111'1100'0010'0000u, false  }, // nan
        { 0b0111'1100'0001'0000u, false  }, // nan
        { 0b0111'1100'0000'1000u, false  }, // nan
        { 0b0111'1100'0000'0100u, false  }, // nan
        { 0b0111'1100'0000'0010u, false  }, // nan
        { 0b0111'1100'0000'0001u, false  }, // nan
        { 0b0111'1100'0000'0001u, false  }, // nan
        { 0b1111'1110'0000'0000u, false  }, // nan
        { 0b1111'1101'0000'0000u, false  }, // nan
        { 0b1111'1100'1000'0000u, false  }, // nan
        { 0b1111'1100'0100'0000u, false  }, // nan
        { 0b1111'1100'0010'0000u, false  }, // nan
        { 0b1111'1100'0001'0000u, false  }, // nan
        { 0b1111'1100'0000'1000u, false  }, // nan
        { 0b1111'1100'0000'0100u, false  }, // nan
        { 0b1111'1100'0000'0010u, false  }, // nan
        { 0b1111'1100'0000'0001u, false  }, // nan
        { 0b1111'1100'0000'0001u, false  }, // nan
        { 0b0111'1100'0000'0000u, false  }, // +inf
        { 0b1111'1100'0000'0000u, false  }, // -inf
        { 0b0011'1100'0000'0001u, false  },
        { 0b0101'1100'0000'0001u, false  },
        { 0b0110'1100'0000'0001u, false  },
        { 0b0111'0100'0000'0001u, false  },
        { 0b0111'1000'0000'0001u, false  },
        { 0b1011'1100'0000'0001u, false  },
        { 0b1101'1100'0000'0001u, false  },
        { 0b1110'1100'0000'0001u, false  },
        { 0b1111'0100'0000'0001u, false  },
        { 0b1111'1000'0000'0001u, false  },
    };

    for (auto&& [rep, expect] : cases) {
        binary16 x;
        x.rep = rep;
        CHECK(binary16_iszero(x) == expect);
    }
}

#if 0
TEST_CASE("binary16 add")
{
    SECTION("same exponent: 5.5 + 5.625 = 11.125")
    {
        // 5.500  = 0100010110000000
        // 5.625  = 0100010110100000
        // 11.125 = 0100100110010000
        binary16 a = binary16_fromrep(U16(0b0100'0101'1000'0000));
        binary16 b = binary16_fromrep(U16(0b0100'0101'1010'0000));
        binary16 c = binary16_fromrep(U16(0b0100'1001'1001'0000));
        CHECK(binary16_tofloat(a) ==  5.500);
        CHECK(binary16_tofloat(b) ==  5.625);
        CHECK(binary16_tofloat(c) == 11.125);

        binary16 r = binary16_add(a, b);
        INFO("a = " << dump_u16(a.rep));
        INFO("b = " << dump_u16(b.rep));
        INFO("c = " << dump_u16(c.rep));
        INFO("r = " << dump_u16(r.rep));
        CHECK(binary16_torep(r) == binary16_torep(c));
    }

    SECTION("same exponent: 5.5 + -5.625 = -0.125")
    {
        //   5.500 = 0100010110000000
        // - 5.625 = 1100010110100000
        //   -------------------------
        //  -0.125 = 1011000000000000
        binary16 a = binary16_fromrep(U16(0b0100'0101'1000'0000));
        binary16 b = binary16_fromrep(U16(0b1100'0101'1010'0000));
        binary16 c = binary16_fromrep(U16(0b1011'0000'0000'0000));
        CHECK(binary16_tofloat(a) ==  5.500);
        CHECK(binary16_tofloat(b) == -5.625);
        CHECK(binary16_tofloat(c) == -0.125);

        binary16 r = binary16_add(a, b);
        INFO("a = " << dump_u16(a.rep));
        INFO("b = " << dump_u16(b.rep));
        INFO("c = " << dump_u16(c.rep));
        INFO("r = " << dump_u16(r.rep));
        CHECK(binary16_torep(r) == binary16_torep(c));
    }

    SECTION("diff exponent: 1.5 + 0.125")
    {
        // 1.500 = 0011111000000000
        // 0.125 = 0011000000000000
        // 1.625 = 0011111010000000
        binary16 a = binary16_fromrep(U16(0b0011'1110'0000'0000));
        binary16 b = binary16_fromrep(U16(0b0011'0000'0000'0000));
        binary16 c = binary16_fromrep(U16(0b0011'1110'1000'0000));
        CHECK(binary16_tofloat(a) == 1.500);
        CHECK(binary16_tofloat(b) == 0.125);
        CHECK(binary16_tofloat(c) == 1.625);

        binary16 r = binary16_add(a, b);
        INFO("a = " << dump_u16(a.rep));
        INFO("b = " << dump_u16(b.rep));
        INFO("c = " << dump_u16(c.rep));
        INFO("r = " << dump_u16(r.rep));
        CHECK(binary16_torep(r) == binary16_torep(c));
    }

    SECTION("right-hand identity: 5.5 + 0.0 = 5.5")
    {
        binary16 a = binary16_fromrep(U16(0b0100'0101'1000'0000));
        binary16 b = binary16_fromrep(U16(0b0000'0000'0000'0000));
        binary16 c = a;
        CHECK(binary16_tofloat(a) ==  5.5);
        CHECK(binary16_tofloat(b) ==  0.0);
        CHECK(binary16_tofloat(c) ==  5.5);
        binary16 r = binary16_add(a, b);
        INFO("a = " << dump_u16(a.rep));
        INFO("b = " << dump_u16(b.rep));
        INFO("c = " << dump_u16(c.rep));
        INFO("r = " << dump_u16(r.rep));
        CHECK(binary16_torep(r) == binary16_torep(c));
    }

    SECTION("right-hand identity (negative): -0.125 + 0.0 = -0.125")
    {
        binary16 a = binary16_fromrep(U16(0b1011'0000'0000'0000));
        binary16 b = binary16_fromrep(U16(0b0000'0000'0000'0000));
        binary16 c = a;
        CHECK(binary16_tofloat(a) == -0.125);
        CHECK(binary16_tofloat(b) ==  0.000);
        CHECK(binary16_tofloat(c) == -0.125);
        binary16 r = binary16_add(a, b);
        INFO("a = " << dump_u16(a.rep));
        INFO("b = " << dump_u16(b.rep));
        INFO("c = " << dump_u16(c.rep));
        INFO("r = " << dump_u16(r.rep));
        CHECK(binary16_torep(r) == binary16_torep(c));
    }

    SECTION("left-hand identity: 0.0 + 5.5 = 5.5")
    {
        binary16 a = binary16_fromrep(U16(0b0000'0000'0000'0000));
        binary16 b = binary16_fromrep(U16(0b0100'0101'1000'0000));
        binary16 c = b;
        CHECK(binary16_tofloat(a) ==  0.0);
        CHECK(binary16_tofloat(b) ==  5.5);
        CHECK(binary16_tofloat(c) ==  5.5);
        binary16 r = binary16_add(a, b);
        INFO("a = " << dump_u16(a.rep));
        INFO("b = " << dump_u16(b.rep));
        INFO("c = " << dump_u16(c.rep));
        INFO("r = " << dump_u16(r.rep));
        CHECK(binary16_torep(r) == binary16_torep(c));
    }

    SECTION("left-hand identity (negative): 0.0 + -0.125 = -0.125")
    {
        binary16 a = binary16_fromrep(U16(0b0000'0000'0000'0000));
        binary16 b = binary16_fromrep(U16(0b1011'0000'0000'0000));
        binary16 c = b;
        CHECK(binary16_tofloat(a) ==  0.000);
        CHECK(binary16_tofloat(b) == -0.125);
        CHECK(binary16_tofloat(c) == -0.125);
        binary16 r = binary16_add(a, b);
        INFO("a = " << dump_u16(a.rep));
        INFO("b = " << dump_u16(b.rep));
        INFO("c = " << dump_u16(c.rep));
        INFO("r = " << dump_u16(r.rep));
        CHECK(binary16_torep(r) == binary16_torep(c));
    }
}
#endif

TEST_CASE("add with nan")
{
    SECTION("1.0 + nan = nan")
    {
        binary16 a = binary16_fromfloat(1.0f);
        binary16 b = binary16_fromfloat(NAN);
        binary16 c = binary16_add(a, b);
        INFO("a = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
        INFO("b = " << binary16_tofloat(b) << " = " << dump_u16(b.rep));
        INFO("c = " << binary16_tofloat(c) << " = " << dump_u16(c.rep));
        CHECK(binary16_isnan(a) == false);
        CHECK(binary16_isnan(b) == true);
        CHECK(binary16_isnan(c) == true);
    }

    SECTION("nan + 1.0 = nan")
    {
        binary16 a = binary16_fromfloat(NAN);
        binary16 b = binary16_fromfloat(1.0f);
        binary16 c = binary16_add(a, b);
        INFO("a = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
        INFO("b = " << binary16_tofloat(b) << " = " << dump_u16(b.rep));
        INFO("c = " << binary16_tofloat(c) << " = " << dump_u16(c.rep));
        CHECK(binary16_isnan(a) == true);
        CHECK(binary16_isnan(b) == false);
        CHECK(binary16_isnan(c) == true);
    }

    SECTION("1.0 - nan = nan")
    {
        binary16 a = binary16_fromfloat(1.0f);
        binary16 b = binary16_fromfloat(NAN);
        binary16 c = binary16_sub(a, b);
        INFO("a = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
        INFO("b = " << binary16_tofloat(b) << " = " << dump_u16(b.rep));
        INFO("c = " << binary16_tofloat(c) << " = " << dump_u16(c.rep));
        CHECK(binary16_isnan(a) == false);
        CHECK(binary16_isnan(b) == true);
        CHECK(binary16_isnan(c) == true);
    }

    SECTION("nan - 1.0 = nan")
    {
        binary16 a = binary16_fromfloat(NAN);
        binary16 b = binary16_fromfloat(1.0f);
        binary16 c = binary16_sub(a, b);
        INFO("a = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
        INFO("b = " << binary16_tofloat(b) << " = " << dump_u16(b.rep));
        INFO("c = " << binary16_tofloat(c) << " = " << dump_u16(c.rep));
        CHECK(binary16_isnan(a) == true);
        CHECK(binary16_isnan(b) == false);
        CHECK(binary16_isnan(c) == true);
    }

}

#if 0
TEST_CASE("add to inf")
{
    SECTION("65504 + 65504 = inf")
    {
        binary16 a = binary16_fromfloat(65504.0f);
        binary16 b = binary16_fromfloat(65504.0f);
        binary16 c = binary16_add(a, b);
        INFO("a = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
        INFO("b = " << binary16_tofloat(b) << " = " << dump_u16(b.rep));
        INFO("c = " << binary16_tofloat(c) << " = " << dump_u16(c.rep));
        CHECK(binary16_isinf(c));
    }

    SECTION("65504 + 32753 = inf")
    {
        binary16 a = binary16_fromfloat(65504.0f);
        binary16 b = binary16_fromfloat(32753.0f);
        binary16 c = binary16_add(a, b);
        INFO("a = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
        INFO("b = " << binary16_tofloat(b) << " = " << dump_u16(b.rep));
        INFO("c = " << binary16_tofloat(c) << " = " << dump_u16(c.rep));
        CHECK(binary16_isinf(c));
    }

    SECTION("65504 + 1 = inf")
    {
        binary16 a = binary16_fromfloat(65504.0f);
        binary16 b = binary16_fromfloat(    1.0f);
        binary16 c = binary16_add(a, b);
        INFO("a = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
        INFO("b = " << binary16_tofloat(b) << " = " << dump_u16(b.rep));
        INFO("c = " << binary16_tofloat(c) << " = " << dump_u16(c.rep));
        // CHECK(binary16_isinf(c));
        CHECK(binary16_eq(a, c));
    }

    SECTION("-65503 + -32754 = inf")
    {
        binary16 a = binary16_fromfloat(-65503.0f);
        binary16 b = binary16_fromfloat(-32754.0f);
        binary16 c = binary16_add(a, b);
        INFO("a = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
        INFO("b = " << binary16_tofloat(b) << " = " << dump_u16(b.rep));
        INFO("c = " << binary16_tofloat(c) << " = " << dump_u16(c.rep));
        CHECK(binary16_isinf(c));
        CHECK(binary16_signbit(c));
    }
}
#endif

TEST_CASE("binary16 sub")
{
    SECTION("same exponent: 5.5 - 5.625 = -0.125")
    {
        //   5.500  = 0100010110000000
        // - 5.625  = 0100010110100000
        //   -------------------------
        //  -0.125 = 1011000000000000
        binary16 a = binary16_fromrep(U16(0b0100'0101'1000'0000));
        binary16 b = binary16_fromrep(U16(0b0100'0101'1010'0000));
        binary16 c = binary16_fromrep(U16(0b1011'0000'0000'0000));
        CHECK(binary16_tofloat(a) ==  5.500);
        CHECK(binary16_tofloat(b) ==  5.625);
        CHECK(binary16_tofloat(c) == -0.125);

        binary16 r = binary16_sub(a, b);
        INFO("a = " << dump_u16(a.rep));
        INFO("b = " << dump_u16(b.rep));
        INFO("c = " << dump_u16(c.rep));
        INFO("r = " << dump_u16(r.rep));
        CHECK(binary16_torep(r) == binary16_torep(c));
    }
}

TEST_CASE("binary16_fromfloat")
{
    auto TestCase = [](const float a, const double max_error)
    {
        const binary16 b = binary16_fromfloat(a);
        const float    c = binary16_tofloat(b);
        const auto     d = half_float::half{a};
        const binary16 n = binary16_next(b);
        const binary16 p = binary16_prev(b);
        const double cdiff = fabs(c - a);
        const double ndiff = fabs(binary16_tofloat(n) - a);
        const double pdiff = fabs(binary16_tofloat(p) - a);

        INFO("a = " << dump_f32(a)       << " = " << std::fixed << std::setprecision(12) << a);
        INFO("b = " << dump_u16(b.rep)   << " = " << std::fixed << std::setprecision(12) << c);
        INFO("c = " << dump_f32(c)       << " = " << std::fixed << std::setprecision(12) << c);
        INFO("d = " << dump_u16(d.data_) << " = " << std::fixed << std::setprecision(12) << (float)d);

        INFO("mine = " << dump_u16(b.rep) << " = " << binary16_tofloat(b));
        INFO("next = " << dump_u16(n.rep) << " = " << binary16_tofloat(n));
        INFO("prev = " << dump_u16(p.rep) << " = " << binary16_tofloat(p));
        INFO("orig = " << a);

        CHECK(c == (float)d);
        CHECK(cdiff <= max_error);
        CHECK(cdiff <= ndiff);
        CHECK(pdiff <= pdiff);
    };

    SECTION("range(0, 1, 0.01)")
    {
        const float a = GENERATE(range(0.0f, 1.0f, 0.01f));
        TestCase(a, 0.0005);
    }

    SECTION("range(1, 1.1, 0.001)")
    {
        const float a = GENERATE(range(1.0f, 1.1f, 0.001f));
        TestCase(a, 0.0005);
    }

    SECTION("range(104, 104.1, 0.0001)")
    {
        const float a = GENERATE(range(104.0f, 104.1f, 0.0001f));
        TestCase(a, 0.001*a);
    }

    SECTION("range(17.005, 17.0057, 0.00001)")
    {
        const float a = GENERATE(range(17.005f, 17.0057f, 0.00001f));
        TestCase(a, 0.001*a);
    }

    SECTION("range(-17.005, -17.0057, 0.00001)")
    {
        const float a = GENERATE(range(-17.0057f, -17.0050f, 0.00001f));
        TestCase(a, 0.001*-a);
    }

    SECTION("range(0.00005, 0.000058, 0.000001)")
    {
        const float a = GENERATE(range(0.00005, 0.000058, 0.000001));
        TestCase(a, 0.0001);
    }

    SECTION("range(0.00002, 0.000029, 0.000001)")
    {
        const float a = GENERATE(range(0.00002, 0.000029, 0.000001));
        TestCase(a, 0.0001);
    }
}

TEST_CASE("binary16_add")
{
    auto TestCase = [](const float a, const float b)
    {
        const float c = a + b;

        const auto  x = binary16_fromfloat(a);
        const auto  y = binary16_fromfloat(b);
        const auto  z = binary16_add(x, y);

        const auto  t = half_float::half{a};
        const auto  u = half_float::half{b};
        const auto  v = t + u;

        const auto x_flt = binary16_tofloat(x);
        const auto y_flt = binary16_tofloat(y);
        const auto z_flt = binary16_tofloat(z);

        const auto a_flt = binary16_tofloat(binary16_fromfloat(a));
        const auto b_flt = binary16_tofloat(binary16_fromfloat(b));
        const auto c_flt = a_flt + b_flt;

        const auto zn     = binary16_next(z);
        const auto zp     = binary16_prev(z);
        const auto z_next = binary16_tofloat(zn);
        const auto z_prev = binary16_tofloat(zp);
        // NOTE: because of loss of precision going float -> binary16, we have
        // to compare against the result of `a + b` after both have gone
        // through the float -> binary16 -> float roundtrip.
        const auto z_diff = fabs(z_flt  - c_flt);
        const auto n_diff = fabs(z_next - c_flt);
        const auto p_diff = fabs(z_prev - c_flt);
        const auto z_lo   = z_prev < z_next ? z_prev : z_next;
        const auto z_hi   = z_prev < z_next ? z_next : z_prev;

        INFO("a = " << dump_f32(a)       << " = " << std::fixed << std::setprecision(12) << a);
        INFO("b = " << dump_f32(b)       << " = " << std::fixed << std::setprecision(12) << b);
        INFO("c = " << dump_f32(c)       << " = " << std::fixed << std::setprecision(12) << c);

        INFO("x = " << dump_u16(x.rep)   << " = " << std::fixed << std::setprecision(12) << x_flt);
        INFO("y = " << dump_u16(y.rep)   << " = " << std::fixed << std::setprecision(12) << y_flt);
        INFO("z = " << dump_u16(z.rep)   << " = " << std::fixed << std::setprecision(12) << z_flt);

        INFO("t = " << dump_u16(t.data_) << " = " << std::fixed << std::setprecision(12) << (float)t);
        INFO("u = " << dump_u16(u.data_) << " = " << std::fixed << std::setprecision(12) << (float)u);
        INFO("v = " << dump_u16(v.data_) << " = " << std::fixed << std::setprecision(12) << (float)v);

        INFO("a (post roundtrip) = " << std::fixed << std::setprecision(12) << a_flt);
        INFO("b (post roundtrip) = " << std::fixed << std::setprecision(12) << b_flt);
        INFO("c (post roundtrip) = " << std::fixed << std::setprecision(12) << c_flt);

        INFO("z_next = " << dump_u16(zn.rep)   << " = " << std::fixed << std::setprecision(12) << z_next << " -> " << n_diff);
        INFO("z_mine = " << dump_u16( z.rep)   << " = " << std::fixed << std::setprecision(12) << z_flt  << " -> " << z_diff);
        INFO("z_prev = " << dump_u16(zp.rep)   << " = " << std::fixed << std::setprecision(12) << z_prev << " -> " << p_diff);

        // Check input conversion matches HalfFloat
        CHECK(x_flt == (float)t);
        CHECK(y_flt == (float)u);

        // Check that z is not "very far" from c
        CHECK(z_diff <= 0.001);  // TODO: pick based on inputs
        CHECK(z_diff <= 0.001*c);  // TODO: pick based on inputs

        // Check that never and prev are not closer to `c` than `z`
        CHECK(z_diff <= n_diff);
        CHECK(z_diff <= p_diff);

        // Check that `z_prev` and `z_next` surround c
        CHECK(z_lo < z_hi);
        CHECK(z_lo < c);
        CHECK(c < z_hi);

        // Check that our answer matches HalfFloat
        CHECK(z_flt == (float)v);
    };


    // TODO: still trying to get the rounding correct. I definitely don't
    // understand the "round to even" rule correctly.
    // http://pages.cs.wisc.edu/~david/courses/cs552/S12/handouts/guardbits.pdf

    TestCase(0.900000095367f, 0.500000000000f);

    // TestCase(1.0f, 2.0f);
    // TestCase(0.900000095367f, 0.800000071526f);

    // SECTION("range(0, 1, 0.1) + range(0, 1, 0.1)")
    // {
    //     auto a = GENERATE(range(0.0f, 1.0f, 0.1f));
    //     auto b = GENERATE(range(0.0f, 1.0f, 0.1f));
    //     TestCase(a, b);
    // }
}

#if 0
TEST_CASE("not subnormal")
{
    auto aa = GENERATE(range(0.0f, 1.0f, 0.1f));
    auto bb = GENERATE(range(0.0f, 1.0f, 0.1f));
    auto a = (float)aa;
    auto b = (float)bb;

    const auto c = a + b;
    INFO("Float32:");
    INFO("a         = " << dump_f32(a) << " = " << a);
    INFO("b         = " << dump_f32(b) << " = " << b);
    INFO("a + b = c = " << dump_f32(c) << " = " << c);

    // using half_float
    const auto t = half_float::half{a};
    const auto u = half_float::half{b};
    const auto v = t + u;

    INFO("HalfFloat:");
    INFO("t         = " << dump_u16(t.data_) << " = " << (float)t);
    INFO("u         = " << dump_u16(u.data_) << " = " << (float)u);
    INFO("t + u = v = " << dump_u16(v.data_) << " = " << (float)v);

    const auto x = binary16_fromfloat(a);
    const auto y = binary16_fromfloat(b);
    const auto z = binary16_add(x, y);

    INFO("Binary16:");
    INFO("x         = " << dump_u16(x.rep) << " = " << binary16_tofloat(x));
    INFO("y         = " << dump_u16(y.rep) << " = " << binary16_tofloat(y));
    INFO("x + y = z = " << dump_u16(z.rep) << " = " << binary16_tofloat(z));

    binary16 n = binary16_next(z);
    binary16 p = binary16_prev(z);

    float zf = binary16_tofloat(z);
    float nf = binary16_tofloat(n);
    float pf = binary16_tofloat(p);

    float curr_diff = fabs(zf - c);
    float next_diff = fabs(nf - c);
    float prev_diff = fabs(pf - c);

    INFO("z = " << zf);
    INFO("n = " << nf);
    INFO("p = " << pf);
    INFO("c = " << c);
    INFO("curr_diff = " << curr_diff);
    INFO("next_diff = " << next_diff);
    INFO("prev_diff = " << prev_diff);

    // check against half_float::half implementation
    CHECK(binary16_tofloat(x) == (float)t);
    CHECK(binary16_tofloat(y) == (float)u);
    // CHECK(binary16_tofloat(z) == (float)v);

    CHECK(x.rep == t.data_);
    CHECK(y.rep == u.data_);
    // CHECK(z.rep == z.data_);

    // // check property: a + b = toHalf (fromHalf a + fromHalf b)
    // CHECK(v     == half_float::half((float)t + (float)u));
    // CHECK(z.rep == binary16_fromfloat(binary16_tofloat(x) + binary16_tofloat(y)).rep);
    // CHECK(binary16_tofloat(z) == binary16_tofloat(binary16_fromfloat(binary16_tofloat(x) + binary16_tofloat(y))));
    // CHECK(z.rep == binary16_fromfloat((float)v).rep);

    // TODO: re-enable
    // // check +/- 1 tick isn't closer to float result than my result
    // CHECK(curr_diff <= 0.0002);  // TODO: need to size this based on the magnitude of c
    // CHECK(curr_diff <= next_diff);
    // CHECK(curr_diff <= prev_diff);

    // CHECK(0 == 1);
}
#endif

#if 0
TEST_CASE("test case")
{
	// auto a = 4.4327446996_h;
	// auto b = 2.4424231659_h;

    // auto a = 1.3419085418_h;
    // auto b = 1.0888469460_h;

    // auto a = 9.75_h;
    // auto b = 0.5625_h;

    auto a = 9.75_h;
    auto b = 9.75_h;

    INFO("exponent(a) = " << half_float::ilogb(a));
    INFO("exponent(b) = " << half_float::ilogb(b));

	auto c = a + b;
	auto d = binary16_fromfloat((float)a);
	auto e = binary16_fromfloat((float)b);
	auto f = binary16_add(d, e);

    INFO("a = " << static_cast<float>(a) << " : " << dump_u16(a.data_));
    INFO("b = " << static_cast<float>(b) << " : " << dump_u16(b.data_));
    INFO("c = " << static_cast<float>(c) << " : " << dump_u16(c.data_));

    INFO("d = " << binary16_tofloat(d) << " : " << dump_u16(d.rep));
    INFO("e = " << binary16_tofloat(e) << " : " << dump_u16(e.rep));
    INFO("f = " << binary16_tofloat(f) << " : " << dump_u16(f.rep));

    // a = 4.43359 : 0b0100 0100 0110 1111
    // b = 2.44336 : 0b0100 0000 1110 0011
    // c = 6.875   : 0b0100 0110 1110 0000
    // d = 4.43359 : 0b0100 0100 0110 1111
    // e = 2.44336 : 0b0100 0000 1110 0011
    // f = 6.87891 : 0b0100 0110 1110 0001

	CHECK((float)c == binary16_tofloat(f));
    CHECK(1 == 0);
}
#endif

#if 0
TEST_CASE("Compare against half-float")
{
    SECTION("9.718670567508067 + 3.382734400826103 = 13.0917968750 (13.1014049683)")
    {
        auto a = 9.71094_h;
        auto b = 3.38086_h;
        auto c = a + b;

        auto x = binary16_fromfloat(float(a));
        auto y = binary16_fromfloat(float(b));
        auto z = binary16_add(x, y);

        CHECK((float)c == binary16_tofloat(z));
    }

    SECTION("example 2")
    {
        auto a = 1.10194_h;
        auto b = 2.38086_h;
        auto c = a + b;

        auto x = binary16_fromfloat(float(a));
        auto y = binary16_fromfloat(float(b));
        auto z = binary16_add(x, y);

        CHECK((float)c == binary16_tofloat(z));
    }

    SECTION("example 3")
    {
        auto a = 1.4386475053807080_h;
        auto b = 3.0590948293520572_h;
        auto c = a + b;

        auto x = binary16_fromfloat(float(a));
        auto y = binary16_fromfloat(float(b));
        auto z = binary16_add(x, y);

        CHECK((float)c == binary16_tofloat(z));
    }

    SECTION("5.8134710286 + 1.7948160365")
    {
        auto a = 5.8134710286_h;
        auto b = 1.7948160365_h;
        auto c = a + b;
        auto d = binary16_fromfloat((float)a);
        auto e = binary16_fromfloat((float)b);
        auto f = binary16_add(d, e);
        CHECK((float)c == binary16_tofloat(f));
    }

    SECTION("2.5408605220 + 0.1328990765")
    {
        auto a = 2.5408605220_h;
        auto b = 0.1328990765_h;
        auto c = a + b;
        auto d = binary16_fromfloat((float)a);
        auto e = binary16_fromfloat((float)b);
        auto f = binary16_add(d, e);
        CHECK((float)c == binary16_tofloat(f));
    }

    SECTION("5.6832681629 + 2.3896652459")
    {
        auto a = 5.6832681629_h;
        auto b = 2.3896652459_h;
        auto c = a + b;
        auto d = binary16_fromfloat((float)a);
        auto e = binary16_fromfloat((float)b);
        auto f = binary16_add(d, e);
        CHECK((float)c == binary16_tofloat(f));
    }

    SECTION("1.1278556931 + 5.2208796690")
    {
        auto a = 1.1278556931_h;
        auto b = 5.2208796690_h;
        auto c = a + b;
        auto d = binary16_fromfloat((float)a);
        auto e = binary16_fromfloat((float)b);
        auto f = binary16_add(d, e);
        CHECK((float)c == binary16_tofloat(f));
    }

    SECTION("3.8324842470 + 1.7878200819")
    {
        auto a = 3.8324842470_h;
        auto b = 1.7878200819_h;
        auto c = a + b;
        auto d = binary16_fromfloat((float)a);
        auto e = binary16_fromfloat((float)b);
        auto f = binary16_add(d, e);
        CHECK((float)c == binary16_tofloat(f));
    }

    SECTION("0.9597278424 + 0.5274002307")
    {
        auto a = 0.9597278424_h;
        auto b = 0.5274002307_h;
        auto c = a + b;
        auto d = binary16_fromfloat((float)a);
        auto e = binary16_fromfloat((float)b);
        auto f = binary16_add(d, e);
        CHECK((float)c == binary16_tofloat(f));
    }

    SECTION("3.6887608375 + 4.8552941894")
    {
        auto a = 3.6887608375_h;
        auto b = 4.8552941894_h;
        auto c = a + b;
        auto d = binary16_fromfloat((float)a);
        auto e = binary16_fromfloat((float)b);
        auto f = binary16_add(d, e);
        CHECK((float)c == binary16_tofloat(f));
    }

    SECTION("1.2430969888 + 3.9567962577")
    {
        auto a = 1.2430969888_h;
        auto b = 3.9567962577_h;
        auto c = a + b;
        auto d = binary16_fromfloat((float)a);
        auto e = binary16_fromfloat((float)b);
        auto f = binary16_add(d, e);
        CHECK((float)c == binary16_tofloat(f));
    }

    SECTION("3.7056261187 + 7.9106040059")
    {
        auto a = 3.7056261187_h;
        auto b = 7.9106040059_h;
        auto c = a + b;
        auto d = binary16_fromfloat((float)a);
        auto e = binary16_fromfloat((float)b);
        auto f = binary16_add(d, e);
        CHECK((float)c == binary16_tofloat(f));
    }

    SECTION("4.4327446996 + 2.4424231659")
    {
        auto a = 4.4327446996_h;
        auto b = 2.4424231659_h;
        auto c = a + b;
        auto d = binary16_fromfloat((float)a);
        auto e = binary16_fromfloat((float)b);
        auto f = binary16_add(d, e);
        CHECK((float)c == binary16_tofloat(f));
    }

    SECTION("7.1239105663 + 6.9510344143")
    {
        auto a = 7.1239105663_h;
        auto b = 6.9510344143_h;
        auto c = a + b;
        auto d = binary16_fromfloat((float)a);
        auto e = binary16_fromfloat((float)b);
        auto f = binary16_add(d, e);
        CHECK((float)c == binary16_tofloat(f));
    }

    SECTION("1.9120035492 + 8.0805666466")
    {
        auto a = 1.9120035492_h;
        auto b = 8.0805666466_h;
        auto c = a + b;
        auto d = binary16_fromfloat((float)a);
        auto e = binary16_fromfloat((float)b);
        auto f = binary16_add(d, e);
        CHECK((float)c == binary16_tofloat(f));
    }

    SECTION("2.6743276492 + 1.3179706881")
    {
        auto a = 2.6743276492_h;
        auto b = 1.3179706881_h;
        auto c = a + b;
        auto d = binary16_fromfloat((float)a);
        auto e = binary16_fromfloat((float)b);
        auto f = binary16_add(d, e);
        CHECK((float)c == binary16_tofloat(f));
    }

    SECTION("9.8276096476 + 6.6069371051")
    {
        auto a = 9.8276096476_h;
        auto b = 6.6069371051_h;
        auto c = a + b;
        auto d = binary16_fromfloat((float)a);
        auto e = binary16_fromfloat((float)b);
        auto f = binary16_add(d, e);
        CHECK((float)c == binary16_tofloat(f));
    }

    SECTION("7.9652812383 + 9.0607159644")
    {
        auto a = 7.9652812383_h;
        auto b = 9.0607159644_h;
        auto c = a + b;
        auto d = binary16_fromfloat((float)a);
        auto e = binary16_fromfloat((float)b);
        auto f = binary16_add(d, e);
        CHECK((float)c == binary16_tofloat(f));
    }

    SECTION("4.1405445841 + 9.0028700148")
    {
        auto a = 4.1405445841_h;
        auto b = 9.0028700148_h;
        auto c = a + b;
        auto d = binary16_fromfloat((float)a);
        auto e = binary16_fromfloat((float)b);
        auto f = binary16_add(d, e);
        CHECK((float)c == binary16_tofloat(f));
    }

    SECTION("2.4449129677 + 8.1710458404")
    {
        auto a = 2.4449129677_h;
        auto b = 8.1710458404_h;
        auto c = a + b;
        auto d = binary16_fromfloat((float)a);
        auto e = binary16_fromfloat((float)b);
        auto f = binary16_add(d, e);
        CHECK((float)c == binary16_tofloat(f));
    }

    SECTION("4.2240583115 + 1.5441648961")
    {
        auto a = 4.2240583115_h;
        auto b = 1.5441648961_h;
        auto c = a + b;
        auto d = binary16_fromfloat((float)a);
        auto e = binary16_fromfloat((float)b);
        auto f = binary16_add(d, e);
        CHECK((float)c == binary16_tofloat(f));
    }

    SECTION("1.2304278091 + 9.4591660922")
    {
        auto a = 1.2304278091_h;
        auto b = 9.4591660922_h;
        auto c = a + b;
        auto d = binary16_fromfloat((float)a);
        auto e = binary16_fromfloat((float)b);
        auto f = binary16_add(d, e);
        CHECK((float)c == binary16_tofloat(f));
    }

    SECTION("6.6993048333 + 2.8673394801")
    {
        auto a = 6.6993048333_h;
        auto b = 2.8673394801_h;
        auto c = a + b;
        auto d = binary16_fromfloat((float)a);
        auto e = binary16_fromfloat((float)b);
        auto f = binary16_add(d, e);
        CHECK((float)c == binary16_tofloat(f));
    }

    SECTION("0.9487714625 + 6.6722516627")
    {
        auto a = 0.9487714625_h;
        auto b = 6.6722516627_h;
        auto c = a + b;
        auto d = binary16_fromfloat((float)a);
        auto e = binary16_fromfloat((float)b);
        auto f = binary16_add(d, e);
        CHECK((float)c == binary16_tofloat(f));
    }

    SECTION("7.2074176238 + 8.0756003670")
    {
        auto a = 7.2074176238_h;
        auto b = 8.0756003670_h;
        auto c = a + b;
        auto d = binary16_fromfloat((float)a);
        auto e = binary16_fromfloat((float)b);
        auto f = binary16_add(d, e);
        CHECK((float)c == binary16_tofloat(f));
    }

    SECTION("5.4213433519 + 4.3640426062")
    {
        auto a = 5.4213433519_h;
        auto b = 4.3640426062_h;
        auto c = a + b;
        auto d = binary16_fromfloat((float)a);
        auto e = binary16_fromfloat((float)b);
        auto f = binary16_add(d, e);
        CHECK((float)c == binary16_tofloat(f));
    }

    SECTION("3.9462181197 + 1.3285243950")
    {
        auto a = 3.9462181197_h;
        auto b = 1.3285243950_h;
        auto c = a + b;
        auto d = binary16_fromfloat((float)a);
        auto e = binary16_fromfloat((float)b);
        auto f = binary16_add(d, e);
        CHECK((float)c == binary16_tofloat(f));
    }

    SECTION("0.9430354363 + 9.3421875403")
    {
        auto a = 0.9430354363_h;
        auto b = 9.3421875403_h;
        auto c = a + b;
        auto d = binary16_fromfloat((float)a);
        auto e = binary16_fromfloat((float)b);
        auto f = binary16_add(d, e);
        CHECK((float)c == binary16_tofloat(f));
    }

    SECTION("7.2311156130 + 8.0590248394")
    {
        auto a = 7.2311156130_h;
        auto b = 8.0590248394_h;
        auto c = a + b;
        auto d = binary16_fromfloat((float)a);
        auto e = binary16_fromfloat((float)b);
        auto f = binary16_add(d, e);
        CHECK((float)c == binary16_tofloat(f));
    }

    SECTION("5.3392124002 + 9.2707786046")
    {
        auto a = 5.3392124002_h;
        auto b = 9.2707786046_h;
        auto c = a + b;
        auto d = binary16_fromfloat((float)a);
        auto e = binary16_fromfloat((float)b);
        auto f = binary16_add(d, e);
        CHECK((float)c == binary16_tofloat(f));
    }

    SECTION("8.6423946377 + 3.0317753207")
    {
        auto a = 8.6423946377_h;
        auto b = 3.0317753207_h;
        auto c = a + b;
        auto d = binary16_fromfloat((float)a);
        auto e = binary16_fromfloat((float)b);
        auto f = binary16_add(d, e);
        CHECK((float)c == binary16_tofloat(f));
    }

    SECTION("3.8299161866 + 9.2251531291")
    {
        auto a = 3.8299161866_h;
        auto b = 9.2251531291_h;
        auto c = a + b;
        auto d = binary16_fromfloat((float)a);
        auto e = binary16_fromfloat((float)b);
        auto f = binary16_add(d, e);
        CHECK((float)c == binary16_tofloat(f));
    }

    SECTION("8.4234614742 + 3.9178032557")
    {
        auto a = 8.4234614742_h;
        auto b = 3.9178032557_h;
        auto c = a + b;
        auto d = binary16_fromfloat((float)a);
        auto e = binary16_fromfloat((float)b);
        auto f = binary16_add(d, e);
        CHECK((float)c == binary16_tofloat(f));
    }

    SECTION("0.9457509009 + 5.5225990147")
    {
        auto a = 0.9457509009_h;
        auto b = 5.5225990147_h;
        auto c = a + b;
        auto d = binary16_fromfloat((float)a);
        auto e = binary16_fromfloat((float)b);
        auto f = binary16_add(d, e);
        CHECK((float)c == binary16_tofloat(f));
    }

    SECTION("1.9185868755 + 4.8840341426")
    {
        auto a = 1.9185868755_h;
        auto b = 4.8840341426_h;
        auto c = a + b;
        auto d = binary16_fromfloat((float)a);
        auto e = binary16_fromfloat((float)b);
        auto f = binary16_add(d, e);
        CHECK((float)c == binary16_tofloat(f));
    }

    SECTION("2.8916359268 + 4.1967192643")
    {
        auto a = 2.8916359268_h;
        auto b = 4.1967192643_h;
        auto c = a + b;
        auto d = binary16_fromfloat((float)a);
        auto e = binary16_fromfloat((float)b);
        auto f = binary16_add(d, e);
        CHECK((float)c == binary16_tofloat(f));
    }

    SECTION("9.1214630530 + 6.3011232090")
    {
        auto a = 9.1214630530_h;
        auto b = 6.3011232090_h;
        auto c = a + b;
        auto d = binary16_fromfloat((float)a);
        auto e = binary16_fromfloat((float)b);
        auto f = binary16_add(d, e);
        CHECK((float)c == binary16_tofloat(f));
    }

    SECTION("4.3239131062 + 8.9149776059")
    {
        auto a = 4.3239131062_h;
        auto b = 8.9149776059_h;
        auto c = a + b;
        auto d = binary16_fromfloat((float)a);
        auto e = binary16_fromfloat((float)b);
        auto f = binary16_add(d, e);
        CHECK((float)c == binary16_tofloat(f));
    }

    SECTION("9.1635120377 + 3.0006762274")
    {
        auto a = 9.1635120377_h;
        auto b = 3.0006762274_h;
        auto c = a + b;
        auto d = binary16_fromfloat((float)a);
        auto e = binary16_fromfloat((float)b);
        auto f = binary16_add(d, e);
        CHECK((float)c == binary16_tofloat(f));
    }

    SECTION("4.9759879768 + 8.1310242132")
    {
        auto a = 4.9759879768_h;
        auto b = 8.1310242132_h;
        auto c = a + b;
        auto d = binary16_fromfloat((float)a);
        auto e = binary16_fromfloat((float)b);
        auto f = binary16_add(d, e);
        CHECK((float)c == binary16_tofloat(f));
    }

    SECTION("2.2051356830 + 9.8151317340")
    {
        auto a = 2.2051356830_h;
        auto b = 9.8151317340_h;
        auto c = a + b;
        auto d = binary16_fromfloat((float)a);
        auto e = binary16_fromfloat((float)b);
        auto f = binary16_add(d, e);
        CHECK((float)c == binary16_tofloat(f));
    }

    SECTION("9.8299513325 + 9.5295497035")
    {
        auto a = 9.8299513325_h;
        auto b = 9.5295497035_h;
        auto c = a + b;
        auto d = binary16_fromfloat((float)a);
        auto e = binary16_fromfloat((float)b);
        auto f = binary16_add(d, e);
        CHECK((float)c == binary16_tofloat(f));
    }

    SECTION("6.8139188076 + 9.9886140305")
    {
        auto a = 6.8139188076_h;
        auto b = 9.9886140305_h;
        auto c = a + b;
        auto d = binary16_fromfloat((float)a);
        auto e = binary16_fromfloat((float)b);
        auto f = binary16_add(d, e);
        CHECK((float)c == binary16_tofloat(f));
    }

    SECTION("4.8861995059 + 8.7459542019")
    {
        auto a = 4.8861995059_h;
        auto b = 8.7459542019_h;
        auto c = a + b;
        auto d = binary16_fromfloat((float)a);
        auto e = binary16_fromfloat((float)b);
        auto f = binary16_add(d, e);
        CHECK((float)c == binary16_tofloat(f));
    }

    SECTION("9.4856936152 + 4.1413895104")
    {
        auto a = 9.4856936152_h;
        auto b = 4.1413895104_h;
        auto c = a + b;
        auto d = binary16_fromfloat((float)a);
        auto e = binary16_fromfloat((float)b);
        auto f = binary16_add(d, e);
        CHECK((float)c == binary16_tofloat(f));
    }

    SECTION("8.5238829588 + 2.6266515781")
    {
        auto a = 8.5238829588_h;
        auto b = 2.6266515781_h;
        auto c = a + b;
        auto d = binary16_fromfloat((float)a);
        auto e = binary16_fromfloat((float)b);
        auto f = binary16_add(d, e);
        CHECK((float)c == binary16_tofloat(f));
    }

    SECTION("9.7485256573 + 1.1082424771")
    {
        auto a = 9.7485256573_h;
        auto b = 1.1082424771_h;
        auto c = a + b;
        auto d = binary16_fromfloat((float)a);
        auto e = binary16_fromfloat((float)b);
        auto f = binary16_add(d, e);
        CHECK((float)c == binary16_tofloat(f));
    }

    SECTION("4.0401925412 + 8.2718052707")
    {
        auto a = 4.0401925412_h;
        auto b = 8.2718052707_h;
        auto c = a + b;
        auto d = binary16_fromfloat((float)a);
        auto e = binary16_fromfloat((float)b);
        auto f = binary16_add(d, e);
        CHECK((float)c == binary16_tofloat(f));
    }

    SECTION("4.4280415043 + 1.2631470689")
    {
        auto a = 4.4280415043_h;
        auto b = 1.2631470689_h;
        auto c = a + b;
        auto d = binary16_fromfloat((float)a);
        auto e = binary16_fromfloat((float)b);
        auto f = binary16_add(d, e);
        CHECK((float)c == binary16_tofloat(f));
    }

    SECTION("0.4076227862 + 8.2779509825")
    {
        auto a = 0.4076227862_h;
        auto b = 8.2779509825_h;
        auto c = a + b;
        auto d = binary16_fromfloat((float)a);
        auto e = binary16_fromfloat((float)b);
        auto f = binary16_add(d, e);
        CHECK((float)c == binary16_tofloat(f));
    }

    SECTION("5.2040211103 + 3.6747608113")
    {
        auto a = 5.2040211103_h;
        auto b = 3.6747608113_h;
        auto c = a + b;
        auto d = binary16_fromfloat((float)a);
        auto e = binary16_fromfloat((float)b);
        auto f = binary16_add(d, e);
        CHECK((float)c == binary16_tofloat(f));
    }

    SECTION("0.3509459376 + 6.8028412672")
    {
        auto a = 0.3509459376_h;
        auto b = 6.8028412672_h;
        auto c = a + b;
        auto d = binary16_fromfloat((float)a);
        auto e = binary16_fromfloat((float)b);
        auto f = binary16_add(d, e);
        CHECK((float)c == binary16_tofloat(f));
    }

    SECTION("4.7271280854 + 5.7884325859")
    {
        auto a = 4.7271280854_h;
        auto b = 5.7884325859_h;
        auto c = a + b;
        auto d = binary16_fromfloat((float)a);
        auto e = binary16_fromfloat((float)b);
        auto f = binary16_add(d, e);
        CHECK((float)c == binary16_tofloat(f));
    }

    SECTION("7.8762706704 + 2.5422290319")
    {
        auto a = 7.8762706704_h;
        auto b = 2.5422290319_h;
        auto c = a + b;
        auto d = binary16_fromfloat((float)a);
        auto e = binary16_fromfloat((float)b);
        auto f = binary16_add(d, e);
        CHECK((float)c == binary16_tofloat(f));
    }

    SECTION("2.6999785828 + 8.3743016730")
    {
        auto a = 2.6999785828_h;
        auto b = 8.3743016730_h;
        auto c = a + b;
        auto d = binary16_fromfloat((float)a);
        auto e = binary16_fromfloat((float)b);
        auto f = binary16_add(d, e);
        CHECK((float)c == binary16_tofloat(f));
    }

    SECTION("7.8423459747 + 5.7925514804")
    {
        auto a = 7.8423459747_h;
        auto b = 5.7925514804_h;
        auto c = a + b;
        auto d = binary16_fromfloat((float)a);
        auto e = binary16_fromfloat((float)b);
        auto f = binary16_add(d, e);
        CHECK((float)c == binary16_tofloat(f));
    }

    SECTION("6.4302091871 + 3.6568055641")
    {
        auto a = 6.4302091871_h;
        auto b = 3.6568055641_h;
        auto c = a + b;
        auto d = binary16_fromfloat((float)a);
        auto e = binary16_fromfloat((float)b);
        auto f = binary16_add(d, e);
        CHECK((float)c == binary16_tofloat(f));
    }

    SECTION("6.6231004990 + 1.6996416389")
    {
        auto a = 6.6231004990_h;
        auto b = 1.6996416389_h;
        auto c = a + b;
        auto d = binary16_fromfloat((float)a);
        auto e = binary16_fromfloat((float)b);
        auto f = binary16_add(d, e);
        CHECK((float)c == binary16_tofloat(f));
    }

    SECTION("1.0465442003 + 9.1992721374")
    {
        auto a = 1.0465442003_h;
        auto b = 9.1992721374_h;
        auto c = a + b;
        auto d = binary16_fromfloat((float)a);
        auto e = binary16_fromfloat((float)b);
        auto f = binary16_add(d, e);
        CHECK((float)c == binary16_tofloat(f));
    }

    SECTION("7.7424408434 + 4.3548856517")
    {
        auto a = 7.7424408434_h;
        auto b = 4.3548856517_h;
        auto c = a + b;
        auto d = binary16_fromfloat((float)a);
        auto e = binary16_fromfloat((float)b);
        auto f = binary16_add(d, e);
        CHECK((float)c == binary16_tofloat(f));
    }

    SECTION("4.5610362328 + 9.9796491190")
    {
        auto a = 4.5610362328_h;
        auto b = 9.9796491190_h;
        auto c = a + b;
        auto d = binary16_fromfloat((float)a);
        auto e = binary16_fromfloat((float)b);
        auto f = binary16_add(d, e);
        CHECK((float)c == binary16_tofloat(f));
    }

    SECTION("2.5778224710 + 0.5489388362")
    {
        auto a = 2.5778224710_h;
        auto b = 0.5489388362_h;
        auto c = a + b;
        auto d = binary16_fromfloat((float)a);
        auto e = binary16_fromfloat((float)b);
        auto f = binary16_add(d, e);
        CHECK((float)c == binary16_tofloat(f));
    }

    SECTION("1.2799267934 + 2.2220592140")
    {
        auto a = 1.2799267934_h;
        auto b = 2.2220592140_h;
        auto c = a + b;
        auto d = binary16_fromfloat((float)a);
        auto e = binary16_fromfloat((float)b);
        auto f = binary16_add(d, e);
        CHECK((float)c == binary16_tofloat(f));
    }

    SECTION("1.5212268856 + 8.1352723130")
    {
        auto a = 1.5212268856_h;
        auto b = 8.1352723130_h;
        auto c = a + b;
        auto d = binary16_fromfloat((float)a);
        auto e = binary16_fromfloat((float)b);
        auto f = binary16_add(d, e);
        CHECK((float)c == binary16_tofloat(f));
    }

    SECTION("7.9670201251 + 8.1635013317")
    {
        auto a = 7.9670201251_h;
        auto b = 8.1635013317_h;
        auto c = a + b;
        auto d = binary16_fromfloat((float)a);
        auto e = binary16_fromfloat((float)b);
        auto f = binary16_add(d, e);
        CHECK((float)c == binary16_tofloat(f));
    }

    SECTION("9.8011337005 + 2.9825027295")
    {
        auto a = 9.8011337005_h;
        auto b = 2.9825027295_h;
        auto c = a + b;
        auto d = binary16_fromfloat((float)a);
        auto e = binary16_fromfloat((float)b);
        auto f = binary16_add(d, e);
        CHECK((float)c == binary16_tofloat(f));
    }

    SECTION("5.2383632775 + 3.3173459267")
    {
        auto a = 5.2383632775_h;
        auto b = 3.3173459267_h;
        auto c = a + b;
        auto d = binary16_fromfloat((float)a);
        auto e = binary16_fromfloat((float)b);
        auto f = binary16_add(d, e);
        CHECK((float)c == binary16_tofloat(f));
    }

    SECTION("2.9223006761 + 8.6842768600")
    {
        auto a = 2.9223006761_h;
        auto b = 8.6842768600_h;
        auto c = a + b;
        auto d = binary16_fromfloat((float)a);
        auto e = binary16_fromfloat((float)b);
        auto f = binary16_add(d, e);
        CHECK((float)c == binary16_tofloat(f));
    }

    SECTION("7.4992709573 + 6.8242384819")
    {
        auto a = 7.4992709573_h;
        auto b = 6.8242384819_h;
        auto c = a + b;
        auto d = binary16_fromfloat((float)a);
        auto e = binary16_fromfloat((float)b);
        auto f = binary16_add(d, e);
        CHECK((float)c == binary16_tofloat(f));
    }

    SECTION("8.2394976937 + 6.5435823495")
    {
        auto a = 8.2394976937_h;
        auto b = 6.5435823495_h;
        auto c = a + b;
        auto d = binary16_fromfloat((float)a);
        auto e = binary16_fromfloat((float)b);
        auto f = binary16_add(d, e);
        CHECK((float)c == binary16_tofloat(f));
    }

    SECTION("0.7987647280 + 2.3067628652")
    {
        auto a = 0.7987647280_h;
        auto b = 2.3067628652_h;
        auto c = a + b;
        auto d = binary16_fromfloat((float)a);
        auto e = binary16_fromfloat((float)b);
        auto f = binary16_add(d, e);
        CHECK((float)c == binary16_tofloat(f));
    }

    SECTION("7.2843166498 + 8.6193666676")
    {
        auto a = 7.2843166498_h;
        auto b = 8.6193666676_h;
        auto c = a + b;
        auto d = binary16_fromfloat((float)a);
        auto e = binary16_fromfloat((float)b);
        auto f = binary16_add(d, e);
        CHECK((float)c == binary16_tofloat(f));
    }

    SECTION("5.9356678089 + 0.9168618840")
    {
        auto a = 5.9356678089_h;
        auto b = 0.9168618840_h;
        auto c = a + b;
        auto d = binary16_fromfloat((float)a);
        auto e = binary16_fromfloat((float)b);
        auto f = binary16_add(d, e);
        CHECK((float)c == binary16_tofloat(f));
    }

    SECTION("8.4079184520 + 5.8676162577")
    {
        auto a = 8.4079184520_h;
        auto b = 5.8676162577_h;
        auto c = a + b;
        auto d = binary16_fromfloat((float)a);
        auto e = binary16_fromfloat((float)b);
        auto f = binary16_add(d, e);
        CHECK((float)c == binary16_tofloat(f));
    }

    SECTION("7.9519085847 + 2.2936051841")
    {
        auto a = 7.9519085847_h;
        auto b = 2.2936051841_h;
        auto c = a + b;
        auto d = binary16_fromfloat((float)a);
        auto e = binary16_fromfloat((float)b);
        auto f = binary16_add(d, e);
        CHECK((float)c == binary16_tofloat(f));
    }

    SECTION("0.6485362278 + 9.7647721276")
    {
        auto a = 0.6485362278_h;
        auto b = 9.7647721276_h;
        auto c = a + b;
        auto d = binary16_fromfloat((float)a);
        auto e = binary16_fromfloat((float)b);
        auto f = binary16_add(d, e);
        CHECK((float)c == binary16_tofloat(f));
    }

    SECTION("2.1834452442 + 6.8984122255")
    {
        auto a = 2.1834452442_h;
        auto b = 6.8984122255_h;
        auto c = a + b;
        auto d = binary16_fromfloat((float)a);
        auto e = binary16_fromfloat((float)b);
        auto f = binary16_add(d, e);
        CHECK((float)c == binary16_tofloat(f));
    }

    SECTION("0.8553981507 + 1.2545040913")
    {
        auto a = 0.8553981507_h;
        auto b = 1.2545040913_h;
        auto c = a + b;
        auto d = binary16_fromfloat((float)a);
        auto e = binary16_fromfloat((float)b);
        auto f = binary16_add(d, e);
        CHECK((float)c == binary16_tofloat(f));
    }

    SECTION("8.1617599349 + 7.1593566736")
    {
        auto a = 8.1617599349_h;
        auto b = 7.1593566736_h;
        auto c = a + b;
        auto d = binary16_fromfloat((float)a);
        auto e = binary16_fromfloat((float)b);
        auto f = binary16_add(d, e);
        CHECK((float)c == binary16_tofloat(f));
    }

    SECTION("2.9881942686 + 0.6763172458")
    {
        auto a = 2.9881942686_h;
        auto b = 0.6763172458_h;
        auto c = a + b;
        auto d = binary16_fromfloat((float)a);
        auto e = binary16_fromfloat((float)b);
        auto f = binary16_add(d, e);
        CHECK((float)c == binary16_tofloat(f));
    }

    SECTION("0.6835877845 + 6.4425788138")
    {
        auto a = 0.6835877845_h;
        auto b = 6.4425788138_h;
        auto c = a + b;
        auto d = binary16_fromfloat((float)a);
        auto e = binary16_fromfloat((float)b);
        auto f = binary16_add(d, e);
        CHECK((float)c == binary16_tofloat(f));
    }

    SECTION("3.2784216933 + 6.2013932791")
    {
        auto a = 3.2784216933_h;
        auto b = 6.2013932791_h;
        auto c = a + b;
        auto d = binary16_fromfloat((float)a);
        auto e = binary16_fromfloat((float)b);
        auto f = binary16_add(d, e);
        CHECK((float)c == binary16_tofloat(f));
    }

    SECTION("8.7572339281 + 6.1320262632")
    {
        auto a = 8.7572339281_h;
        auto b = 6.1320262632_h;
        auto c = a + b;
        auto d = binary16_fromfloat((float)a);
        auto e = binary16_fromfloat((float)b);
        auto f = binary16_add(d, e);
        CHECK((float)c == binary16_tofloat(f));
    }

    SECTION("1.4386475054 + 3.0590948294")
    {
        auto a = 1.4386475054_h;
        auto b = 3.0590948294_h;
        auto c = a + b;
        auto d = binary16_fromfloat((float)a);
        auto e = binary16_fromfloat((float)b);
        auto f = binary16_add(d, e);
        CHECK((float)c == binary16_tofloat(f));
    }

    SECTION("9.0990732317 + 6.9284238668")
    {
        auto a = 9.0990732317_h;
        auto b = 6.9284238668_h;
        auto c = a + b;
        auto d = binary16_fromfloat((float)a);
        auto e = binary16_fromfloat((float)b);
        auto f = binary16_add(d, e);
        CHECK((float)c == binary16_tofloat(f));
    }

    SECTION("3.9017292446 + 7.9193426887")
    {
        auto a = 3.9017292446_h;
        auto b = 7.9193426887_h;
        auto c = a + b;
        auto d = binary16_fromfloat((float)a);
        auto e = binary16_fromfloat((float)b);
        auto f = binary16_add(d, e);
        CHECK((float)c == binary16_tofloat(f));
    }

    SECTION("9.7186705675 + 3.3827344008")
    {
        auto a = 9.7186705675_h;
        auto b = 3.3827344008_h;
        auto c = a + b;
        auto d = binary16_fromfloat((float)a);
        auto e = binary16_fromfloat((float)b);
        auto f = binary16_add(d, e);
        CHECK((float)c == binary16_tofloat(f));
    }

    SECTION("7.0215120172 + 0.0627529313")
    {
        auto a = 7.0215120172_h;
        auto b = 0.0627529313_h;
        auto c = a + b;
        auto d = binary16_fromfloat((float)a);
        auto e = binary16_fromfloat((float)b);
        auto f = binary16_add(d, e);
        CHECK((float)c == binary16_tofloat(f));
    }

    SECTION("1.3419085418 + 1.0888469460")
    {
        auto a = 1.3419085418_h;
        auto b = 1.0888469460_h;
        auto c = a + b;
        auto d = binary16_fromfloat((float)a);
        auto e = binary16_fromfloat((float)b);
        auto f = binary16_add(d, e);

		auto g = f; g.rep += 1;
		auto h = f; h.rep -= 1;
		INFO("f = " << binary16_tofloat(f) << ": " << dump_u16(f.rep));
		INFO("g = " << binary16_tofloat(g) << ": " << dump_u16(g.rep));
		INFO("h = " << binary16_tofloat(h) << ": " << dump_u16(h.rep));

        CHECK((float)c == binary16_tofloat(f));
    }
}
#endif
