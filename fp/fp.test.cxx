#include <catch2/catch.hpp>
#include <cstdio>
#include <cstring>

#include "fp.h"

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
        5.5,
        -5.5,
        1.0,
        -1.0,
        2.0,
        -2.0,
        0.00006103515625,
        -0.00006103515625,
        65504,
        -65504,
        1.00097656,
        -1.00097656,
        0.33325195,
        0.0,
        -0.0,
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

TEST_CASE("binary16 add cleanly representable")
{
    std::vector<std::tuple<double, double>> vs = {
        {   0.125       ,   0.125        },
        {   0.125       ,   0.501953125  },
        { 101.5         , 234.5          },
        {  10.9375      ,   0.28125      },
        {   1.0009765625,   1.0009765625 },
        // { 32768        , 1.0009765625 },
    };

    for (auto [x, y] : vs) {
        auto z = x + y;
        binary16 a = binary16_fromfloat(x);
        binary16 b = binary16_fromfloat(y);
        binary16 c = binary16_fromfloat(z);
        CHECK(binary16_tofloat(a) == x);
        CHECK(binary16_tofloat(b) == y);
        CHECK(binary16_tofloat(c) == z);
        binary16 r = binary16_add(a, b);
        INFO("a = " << binary16_tofloat(a) << " => " << dump_u16(a.rep));
        INFO("b = " << binary16_tofloat(b) << " => " << dump_u16(b.rep));
        INFO("c = " << binary16_tofloat(c) << " => " << dump_u16(c.rep));
        INFO("r = " << binary16_tofloat(r) << " => " << dump_u16(r.rep));
        CHECK(binary16_torep(r) == binary16_torep(c));
    }
}

TEST_CASE("binary16 not cleanly representable")
{
    SECTION("0.5 + 0.6 = 1.1")
    {
        binary16 a = binary16_fromrep(U16(0b0011'1000'0000'0000)); //  0.5
        binary16 b = binary16_fromrep(U16(0b0011'1000'1100'1100)); // ~0.6
        binary16 c = binary16_fromrep(U16(0b0011'1100'0110'0110)); // ~1.1

        binary16 r = binary16_add(a, b);
        INFO("a = " << dump_u16(a.rep));
        INFO("b = " << dump_u16(b.rep));
        INFO("c = " << dump_u16(c.rep));
        INFO("r = " << dump_u16(r.rep));
        CHECK(binary16_torep(r) == binary16_torep(c));
    }

    SECTION("1.5 + 0.000060975552")
    {
        binary16 a = binary16_fromrep(U16(0b0000'0011'1111'1111)); // 0.000060975552
        binary16 b = binary16_fromrep(U16(0b0011'1110'0000'0000)); // 1.5
        binary16 c = binary16_fromrep(U16(0b0011'1110'0000'0000)); // 1.5
        binary16 r = binary16_add(a, b);
        INFO("a = " << dump_u16(a.rep));
        INFO("b = " << dump_u16(b.rep));
        INFO("c = " << dump_u16(c.rep));
        INFO("r = " << dump_u16(r.rep));
        INFO("a(float) = " << dump_f32(binary16_tofloat(a)));
        INFO("b(float) = " << dump_f32(binary16_tofloat(b)));
        CHECK(binary16_tofloat(a) == 0.000060975552f);
        CHECK(binary16_tofloat(b) == 1.5);
        CHECK(binary16_torep(r) == binary16_torep(c));
    }

    SECTION("1.5 + 0.000000059604645")
    {
        binary16 a = binary16_fromrep(U16(0b0000'0000'0000'0001)); // 0.000000059604645
        binary16 b = binary16_fromrep(U16(0b0011'1110'0000'0000)); // 1.5
        binary16 c = binary16_fromrep(U16(0b0011'1110'0000'0000)); // 1.5
        binary16 r = binary16_add(a, b);
        INFO("a = " << dump_u16(a.rep));
        INFO("b = " << dump_u16(b.rep));
        INFO("c = " << dump_u16(c.rep));
        INFO("r = " << dump_u16(r.rep));
        INFO("a(float) = " << dump_f32(binary16_tofloat(a)));
        INFO("b(float) = " << dump_f32(binary16_tofloat(b)));
        CHECK(binary16_tofloat(a) == 0.000000059604645f);
        CHECK(binary16_tofloat(b) == 1.5f);
        CHECK(binary16_torep(r) == binary16_torep(c));
    }
}

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
