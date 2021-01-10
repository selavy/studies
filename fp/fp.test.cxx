#include <catch2/catch.hpp>
#include <cstdio>
#include <cstring>

#include "fp.h"

#include "half.hpp"

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

    SECTION("1.7 + 1.1")
    {
        float f1 = 1.7f;
        float f2 = 1.1f;
        float f3 = f1 + f2;
        binary16 a = binary16_fromfloat(f1);
        binary16 b = binary16_fromfloat(f2);
        binary16 c = binary16_add(a, b);
        INFO("a = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
        INFO("b = " << binary16_tofloat(b) << " = " << dump_u16(b.rep));
        INFO("c = " << binary16_tofloat(c) << " = " << dump_u16(c.rep));
        CHECK(c.rep == binary16_fromfloat(f3).rep);
    }
}

#if 0
TEST_CASE("description")
{
    SECTION("subnormal")
    {
        char buf[1024];
        binary16 x = binary16_fromrep(0b0000001111111111u);
        binary16_desc(x, buf, sizeof(buf));
        INFO("buf = " << buf);
        CHECK(1 == 2);
    }

    SECTION("normal")
    {
        char buf[1024];
        binary16 x = binary16_fromrep(0b0011111000000000u);
        binary16_desc(x, buf, sizeof(buf));
        INFO("buf = " << buf);
        CHECK(1 == 2);
    }
}
#endif

TEST_CASE("Autogenerated add tests")
{
    SECTION("1.0 + 1.0 = 2.0000000000 (2.0000000000)")
    {
         binary16 x = binary16_fromrep(0b0011110000000000); // 1.0
         binary16 y = binary16_fromrep(0b0011110000000000); // 1.0
         binary16 z = binary16_fromrep(0b0100000000000000); // 2.0
         binary16 a = binary16_add(x, y);
         INFO("x = 1.0000000000 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 1.0000000000 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 2.0000000000 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("1.5 + 1.0 = 2.5000000000 (2.5000000000)")
    {
         binary16 x = binary16_fromrep(0b0011111000000000); // 1.5
         binary16 y = binary16_fromrep(0b0011110000000000); // 1.0
         binary16 z = binary16_fromrep(0b0100000100000000); // 2.5
         binary16 a = binary16_add(x, y);
         INFO("x = 1.5000000000 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 1.0000000000 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 2.5000000000 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("0.5 + 1.0 = 1.5000000000 (1.5000000000)")
    {
         binary16 x = binary16_fromrep(0b0011100000000000); // 0.5
         binary16 y = binary16_fromrep(0b0011110000000000); // 1.0
         binary16 z = binary16_fromrep(0b0011111000000000); // 1.5
         binary16 a = binary16_add(x, y);
         INFO("x = 0.5000000000 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 1.0000000000 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 1.5000000000 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("0.5 + 10.1 = 10.5937500000 (10.6000000000)")
    {
         binary16 x = binary16_fromrep(0b0011100000000000); // 0.5
         binary16 y = binary16_fromrep(0b0100100100001100); // 10.1
         binary16 z = binary16_fromrep(0b0100100101001100); // 10.59375
         binary16 a = binary16_add(x, y);
         INFO("x = 0.5000000000 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 10.1000000000 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 10.5937500000 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("1.5 + 10.1 = 11.5937500000 (11.6000000000)")
    {
         binary16 x = binary16_fromrep(0b0011111000000000); // 1.5
         binary16 y = binary16_fromrep(0b0100100100001100); // 10.1
         binary16 z = binary16_fromrep(0b0100100111001100); // 11.59375
         binary16 a = binary16_add(x, y);
         INFO("x = 1.5000000000 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 10.1000000000 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 11.5937500000 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("1.5 + 100.1 = 101.5625000000 (101.6000000000)")
    {
         binary16 x = binary16_fromrep(0b0011111000000000); // 1.5
         binary16 y = binary16_fromrep(0b0101011001000001); // 100.1
         binary16 z = binary16_fromrep(0b0101011001011001); // 101.5625
         binary16 a = binary16_add(x, y);
         INFO("x = 1.5000000000 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 100.1000000000 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 101.5625000000 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("1.5 + 100.01 = 101.5000000000 (101.5100000000)")
    {
         binary16 x = binary16_fromrep(0b0011111000000000); // 1.5
         binary16 y = binary16_fromrep(0b0101011001000000); // 100.01
         binary16 z = binary16_fromrep(0b0101011001011000); // 101.5
         binary16 a = binary16_add(x, y);
         INFO("x = 1.5000000000 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 100.0100000000 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 101.5000000000 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("0.01 + 100.01 = 100.0099945068 (100.0200000000)")
    {
         binary16 x = binary16_fromrep(0b0010000100011110); // 0.01
         binary16 y = binary16_fromrep(0b0101011001000000); // 100.01
         binary16 z = binary16_fromrep(0b0101011001000000); // 100.00999450683594
         binary16 a = binary16_add(x, y);
         INFO("x = 0.0100000000 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 100.0100000000 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 100.0099945068 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("1e-05 + 1e-05 = 0.0000199080 (0.0000200000)")
    {
         binary16 x = binary16_fromrep(0b0000000010100111); // 1e-05
         binary16 y = binary16_fromrep(0b0000000010100111); // 1e-05
         binary16 z = binary16_fromrep(0b0000000101001110); // 1.990795135498047e-05
         binary16 a = binary16_add(x, y);
         INFO("x = 0.0000100000 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 0.0000100000 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 0.0000199080 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("1.00001 + 5.00001 = 6.0000000000 (6.0000200000)")
    {
         binary16 x = binary16_fromrep(0b0011110000000000); // 1.00001
         binary16 y = binary16_fromrep(0b0100010100000000); // 5.00001
         binary16 z = binary16_fromrep(0b0100011000000000); // 6.0
         binary16 a = binary16_add(x, y);
         INFO("x = 1.0000100000 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 5.0000100000 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 6.0000000000 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("124.1234 + 1.1829237 = 125.2451171875 (125.3063237000)")
    {
         binary16 x = binary16_fromrep(0b0101011111000001); // 124.1234
         binary16 y = binary16_fromrep(0b0011110010111011); // 1.1829237
         binary16 z = binary16_fromrep(0b0101011111010011); // 125.2451171875
         binary16 a = binary16_add(x, y);
         INFO("x = 124.1234000000 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 1.1829237000 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 125.2451171875 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("0.27272 + 0.72728 = 0.9997558594 (1.0000000000)")
    {
         binary16 x = binary16_fromrep(0b0011010001011101); // 0.27272
         binary16 y = binary16_fromrep(0b0011100111010001); // 0.72728
         binary16 z = binary16_fromrep(0b0011101111111111); // 0.999755859375
         binary16 a = binary16_add(x, y);
         INFO("x = 0.2727200000 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 0.7272800000 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 0.9997558594 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("6.828282 + 7.18282346 = 14.0078125000 (14.0111054600)")
    {
         binary16 x = binary16_fromrep(0b0100011011010100); // 6.828282
         binary16 y = binary16_fromrep(0b0100011100101110); // 7.18282346
         binary16 z = binary16_fromrep(0b0100101100000001); // 14.0078125
         binary16 a = binary16_add(x, y);
         INFO("x = 6.8282820000 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 7.1828234600 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 14.0078125000 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("0.6106367660844092 + 6.6719760034299656 = 7.2822265625 (7.2826127695)")
    {
         binary16 x = binary16_fromrep(0b0011100011100010); // 0.6106367660844092
         binary16 y = binary16_fromrep(0b0100011010101100); // 6.6719760034299656
         binary16 z = binary16_fromrep(0b0100011101001000); // 7.2822265625
         binary16 a = binary16_add(x, y);
         INFO("x = 0.6106367661 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 6.6719760034 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 7.2822265625 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("2.2861758147090603 + 0.16159267716543746 = 2.4466552734 (2.4477684919)")
    {
         binary16 x = binary16_fromrep(0b0100000010010010); // 2.2861758147090603
         binary16 y = binary16_fromrep(0b0011000100101011); // 0.16159267716543746
         binary16 z = binary16_fromrep(0b0100000011100100); // 2.4466552734375
         binary16 a = binary16_add(x, y);
         INFO("x = 2.2861758147 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 0.1615926772 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 2.4466552734 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("8.20404181868691 + 2.0506431085778454 = 10.2519531250 (10.2546849273)")
    {
         binary16 x = binary16_fromrep(0b0100100000011010); // 8.20404181868691
         binary16 y = binary16_fromrep(0b0100000000011001); // 2.0506431085778454
         binary16 z = binary16_fromrep(0b0100100100100000); // 10.251953125
         binary16 a = binary16_add(x, y);
         INFO("x = 8.2040418187 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 2.0506431086 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 10.2519531250 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("1.403190716388657 + 6.754682258257178 = 8.1562500000 (8.1578729746)")
    {
         binary16 x = binary16_fromrep(0b0011110110011100); // 1.403190716388657
         binary16 y = binary16_fromrep(0b0100011011000001); // 6.754682258257178
         binary16 z = binary16_fromrep(0b0100100000010100); // 8.15625
         binary16 a = binary16_add(x, y);
         INFO("x = 1.4031907164 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 6.7546822583 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 8.1562500000 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("7.1794438939076155 + 1.1029659927015423 = 8.2783203125 (8.2824098866)")
    {
         binary16 x = binary16_fromrep(0b0100011100101101); // 7.1794438939076155
         binary16 y = binary16_fromrep(0b0011110001101001); // 1.1029659927015423
         binary16 z = binary16_fromrep(0b0100100000100011); // 8.2783203125
         binary16 a = binary16_add(x, y);
         INFO("x = 7.1794438939 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 1.1029659927 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 8.2783203125 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("8.523769848129529 + 8.181144383615834 = 16.7031250000 (16.7049142317)")
    {
         binary16 x = binary16_fromrep(0b0100100001000011); // 8.523769848129529
         binary16 y = binary16_fromrep(0b0100100000010111); // 8.181144383615834
         binary16 z = binary16_fromrep(0b0100110000101101); // 16.703125
         binary16 a = binary16_add(x, y);
         INFO("x = 8.5237698481 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 8.1811443836 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 16.7031250000 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("6.143277738144254 + 8.071632816893628 = 14.2109375000 (14.2149105550)")
    {
         binary16 x = binary16_fromrep(0b0100011000100100); // 6.143277738144254
         binary16 y = binary16_fromrep(0b0100100000001001); // 8.071632816893628
         binary16 z = binary16_fromrep(0b0100101100011011); // 14.2109375
         binary16 a = binary16_add(x, y);
         INFO("x = 6.1432777381 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 8.0716328169 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 14.2109375000 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("2.64524055453468 + 7.881881442259013 = 10.5234375000 (10.5271219968)")
    {
         binary16 x = binary16_fromrep(0b0100000101001010); // 2.64524055453468
         binary16 y = binary16_fromrep(0b0100011111100001); // 7.881881442259013
         binary16 z = binary16_fromrep(0b0100100101000011); // 10.5234375
         binary16 a = binary16_add(x, y);
         INFO("x = 2.6452405545 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 7.8818814423 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 10.5234375000 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("9.166856781183295 + 1.0453784602375926 = 10.2089843750 (10.2122352414)")
    {
         binary16 x = binary16_fromrep(0b0100100010010101); // 9.166856781183295
         binary16 y = binary16_fromrep(0b0011110000101110); // 1.0453784602375926
         binary16 z = binary16_fromrep(0b0100100100011010); // 10.208984375
         binary16 a = binary16_add(x, y);
         INFO("x = 9.1668567812 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 1.0453784602 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 10.2089843750 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("8.882494763446667 + 1.5221511804768373 = 10.3964843750 (10.4046459439)")
    {
         binary16 x = binary16_fromrep(0b0100100001110000); // 8.882494763446667
         binary16 y = binary16_fromrep(0b0011111000010110); // 1.5221511804768373
         binary16 z = binary16_fromrep(0b0100100100110010); // 10.396484375
         binary16 a = binary16_add(x, y);
         INFO("x = 8.8824947634 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 1.5221511805 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 10.3964843750 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("5.341212652873246 + 9.590488562289336 = 14.9257812500 (14.9317012152)")
    {
         binary16 x = binary16_fromrep(0b0100010101010111); // 5.341212652873246
         binary16 y = binary16_fromrep(0b0100100011001011); // 9.590488562289336
         binary16 z = binary16_fromrep(0b0100101101110110); // 14.92578125
         binary16 a = binary16_add(x, y);
         INFO("x = 5.3412126529 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 9.5904885623 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 14.9257812500 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("3.401313317146272 + 2.7432610263196056 = 6.1425781250 (6.1445743435)")
    {
         binary16 x = binary16_fromrep(0b0100001011001101); // 3.401313317146272
         binary16 y = binary16_fromrep(0b0100000101111100); // 2.7432610263196056
         binary16 z = binary16_fromrep(0b0100011000100100); // 6.142578125
         binary16 a = binary16_add(x, y);
         INFO("x = 3.4013133171 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 2.7432610263 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 6.1425781250 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("8.39049720064664 + 9.939016342446314 = 18.3203125000 (18.3295135431)")
    {
         binary16 x = binary16_fromrep(0b0100100000110001); // 8.39049720064664
         binary16 y = binary16_fromrep(0b0100100011111000); // 9.939016342446314
         binary16 z = binary16_fromrep(0b0100110010010100); // 18.3203125
         binary16 a = binary16_add(x, y);
         INFO("x = 8.3904972006 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 9.9390163424 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 18.3203125000 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("6.505235201173271 + 1.608063278794859 = 8.1113281250 (8.1132984800)")
    {
         binary16 x = binary16_fromrep(0b0100011010000001); // 6.505235201173271
         binary16 y = binary16_fromrep(0b0011111001101110); // 1.608063278794859
         binary16 z = binary16_fromrep(0b0100100000001110); // 8.111328125
         binary16 a = binary16_add(x, y);
         INFO("x = 6.5052352012 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 1.6080632788 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 8.1113281250 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("6.221317127244332 + 4.10331717473793 = 10.3203125000 (10.3246343020)")
    {
         binary16 x = binary16_fromrep(0b0100011000111000); // 6.221317127244332
         binary16 y = binary16_fromrep(0b0100010000011010); // 4.10331717473793
         binary16 z = binary16_fromrep(0b0100100100101001); // 10.3203125
         binary16 a = binary16_add(x, y);
         INFO("x = 6.2213171272 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 4.1033171747 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 10.3203125000 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("8.158594913768384 + 2.5320079536202056 = 10.6875000000 (10.6906028674)")
    {
         binary16 x = binary16_fromrep(0b0100100000010100); // 8.158594913768384
         binary16 y = binary16_fromrep(0b0100000100010000); // 2.5320079536202056
         binary16 z = binary16_fromrep(0b0100100101011000); // 10.6875
         binary16 a = binary16_add(x, y);
         INFO("x = 8.1585949138 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 2.5320079536 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 10.6875000000 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("8.36988498187779 + 3.340256306094762 = 11.7070312500 (11.7101412880)")
    {
         binary16 x = binary16_fromrep(0b0100100000101111); // 8.36988498187779
         binary16 y = binary16_fromrep(0b0100001010101110); // 3.340256306094762
         binary16 z = binary16_fromrep(0b0100100111011010); // 11.70703125
         binary16 a = binary16_add(x, y);
         INFO("x = 8.3698849819 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 3.3402563061 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 11.7070312500 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("6.754872214132247 + 7.416609620024208 = 14.1679687500 (14.1714818342)")
    {
         binary16 x = binary16_fromrep(0b0100011011000001); // 6.754872214132247
         binary16 y = binary16_fromrep(0b0100011101101010); // 7.416609620024208
         binary16 z = binary16_fromrep(0b0100101100010101); // 14.16796875
         binary16 a = binary16_add(x, y);
         INFO("x = 6.7548722141 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 7.4166096200 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 14.1679687500 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("5.694040523321809 + 7.150540780676204 = 12.8398437500 (12.8445813040)")
    {
         binary16 x = binary16_fromrep(0b0100010110110001); // 5.694040523321809
         binary16 y = binary16_fromrep(0b0100011100100110); // 7.150540780676204
         binary16 z = binary16_fromrep(0b0100101001101011); // 12.83984375
         binary16 a = binary16_add(x, y);
         INFO("x = 5.6940405233 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 7.1505407807 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 12.8398437500 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("6.558853304306172 + 9.680459397007473 = 16.2382812500 (16.2393127013)")
    {
         binary16 x = binary16_fromrep(0b0100011010001111); // 6.558853304306172
         binary16 y = binary16_fromrep(0b0100100011010111); // 9.680459397007473
         binary16 z = binary16_fromrep(0b0100110000001111); // 16.23828125
         binary16 a = binary16_add(x, y);
         INFO("x = 6.5588533043 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 9.6804593970 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 16.2382812500 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("3.4720967802762406 + 7.988764463292517 = 11.4589843750 (11.4608612436)")
    {
         binary16 x = binary16_fromrep(0b0100001011110001); // 3.4720967802762406
         binary16 y = binary16_fromrep(0b0100011111111101); // 7.988764463292517
         binary16 z = binary16_fromrep(0b0100100110111010); // 11.458984375
         binary16 a = binary16_add(x, y);
         INFO("x = 3.4720967803 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 7.9887644633 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 11.4589843750 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("9.004121154999671 + 8.193431821536505 = 17.1875000000 (17.1975529765)")
    {
         binary16 x = binary16_fromrep(0b0100100010000000); // 9.004121154999671
         binary16 y = binary16_fromrep(0b0100100000011000); // 8.193431821536505
         binary16 z = binary16_fromrep(0b0100110001001100); // 17.1875
         binary16 a = binary16_add(x, y);
         INFO("x = 9.0041211550 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 8.1934318215 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 17.1875000000 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("8.723423420359563 + 2.428140361113348 = 11.1464843750 (11.1515637815)")
    {
         binary16 x = binary16_fromrep(0b0100100001011100); // 8.723423420359563
         binary16 y = binary16_fromrep(0b0100000011011011); // 2.428140361113348
         binary16 z = binary16_fromrep(0b0100100110010010); // 11.146484375
         binary16 a = binary16_add(x, y);
         INFO("x = 8.7234234204 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 2.4281403611 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 11.1464843750 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("9.905791422977714 + 8.400112437347328 = 18.2968750000 (18.3059038603)")
    {
         binary16 x = binary16_fromrep(0b0100100011110011); // 9.905791422977714
         binary16 y = binary16_fromrep(0b0100100000110011); // 8.400112437347328
         binary16 z = binary16_fromrep(0b0100110010010011); // 18.296875
         binary16 a = binary16_add(x, y);
         INFO("x = 9.9057914230 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 8.4001124373 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 18.2968750000 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("5.813471028574286 + 1.7948160364680354 = 7.6064453125 (7.6082870650)")
    {
         binary16 x = binary16_fromrep(0b0100010111010000); // 5.813471028574286
         binary16 y = binary16_fromrep(0b0011111100101101); // 1.7948160364680354
         binary16 z = binary16_fromrep(0b0100011110011011); // 7.6064453125
         binary16 a = binary16_add(x, y);
         INFO("x = 5.8134710286 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 1.7948160365 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 7.6064453125 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("2.540860522023185 + 0.13289907654805755 = 2.6718750000 (2.6737595986)")
    {
         binary16 x = binary16_fromrep(0b0100000100010100); // 2.540860522023185
         binary16 y = binary16_fromrep(0b0011000001000000); // 0.13289907654805755
         binary16 z = binary16_fromrep(0b0100000101011000); // 2.671875
         binary16 a = binary16_add(x, y);
         INFO("x = 2.5408605220 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 0.1328990765 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 2.6718750000 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("5.683268162879667 + 2.389665245888332 = 8.0683593750 (8.0729334088)")
    {
         binary16 x = binary16_fromrep(0b0100010110101110); // 5.683268162879667
         binary16 y = binary16_fromrep(0b0100000011000111); // 2.389665245888332
         binary16 z = binary16_fromrep(0b0100100000001000); // 8.068359375
         binary16 a = binary16_add(x, y);
         INFO("x = 5.6832681629 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 2.3896652459 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 8.0683593750 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("1.1278556931333394 + 5.220879669046953 = 6.3457031250 (6.3487353622)")
    {
         binary16 x = binary16_fromrep(0b0011110010000010); // 1.1278556931333394
         binary16 y = binary16_fromrep(0b0100010100111000); // 5.220879669046953
         binary16 z = binary16_fromrep(0b0100011001011000); // 6.345703125
         binary16 a = binary16_add(x, y);
         INFO("x = 1.1278556931 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 5.2208796690 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 6.3457031250 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("3.832484246994843 + 1.787820081897341 = 5.6191406250 (5.6203043289)")
    {
         binary16 x = binary16_fromrep(0b0100001110101010); // 3.832484246994843
         binary16 y = binary16_fromrep(0b0011111100100110); // 1.787820081897341
         binary16 z = binary16_fromrep(0b0100010110011110); // 5.619140625
         binary16 a = binary16_add(x, y);
         INFO("x = 3.8324842470 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 1.7878200819 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 5.6191406250 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("0.9597278424132882 + 0.5274002306716874 = 1.4868164062 (1.4871280731)")
    {
         binary16 x = binary16_fromrep(0b0011101110101101); // 0.9597278424132882
         binary16 y = binary16_fromrep(0b0011100000111000); // 0.5274002306716874
         binary16 z = binary16_fromrep(0b0011110111110010); // 1.48681640625
         binary16 a = binary16_add(x, y);
         INFO("x = 0.9597278424 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 0.5274002307 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 1.4868164062 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("3.6887608375490477 + 4.855294189368947 = 8.5390625000 (8.5440550269)")
    {
         binary16 x = binary16_fromrep(0b0100001101100000); // 3.6887608375490477
         binary16 y = binary16_fromrep(0b0100010011011010); // 4.855294189368947
         binary16 z = binary16_fromrep(0b0100100001000101); // 8.5390625
         binary16 a = binary16_add(x, y);
         INFO("x = 3.6887608375 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 4.8552941894 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 8.5390625000 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("1.2430969887852061 + 3.956796257709235 = 5.1972656250 (5.1998932465)")
    {
         binary16 x = binary16_fromrep(0b0011110011111000); // 1.2430969887852061
         binary16 y = binary16_fromrep(0b0100001111101001); // 3.956796257709235
         binary16 z = binary16_fromrep(0b0100010100110010); // 5.197265625
         binary16 a = binary16_add(x, y);
         INFO("x = 1.2430969888 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 3.9567962577 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 5.1972656250 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("3.7056261186502395 + 7.910604005939947 = 11.6152343750 (11.6162301246)")
    {
         binary16 x = binary16_fromrep(0b0100001101101001); // 3.7056261186502395
         binary16 y = binary16_fromrep(0b0100011111101001); // 7.910604005939947
         binary16 z = binary16_fromrep(0b0100100111001110); // 11.615234375
         binary16 a = binary16_add(x, y);
         INFO("x = 3.7056261187 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 7.9106040059 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 11.6152343750 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("4.432744699567087 + 2.4424231658806237 = 6.8710937500 (6.8751678654)")
    {
         binary16 x = binary16_fromrep(0b0100010001101110); // 4.432744699567087
         binary16 y = binary16_fromrep(0b0100000011100010); // 2.4424231658806237
         binary16 z = binary16_fromrep(0b0100011011011111); // 6.87109375
         binary16 a = binary16_add(x, y);
         INFO("x = 4.4327446996 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 2.4424231659 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 6.8710937500 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("7.123910566305342 + 6.951034414294303 = 14.0703125000 (14.0749449806)")
    {
         binary16 x = binary16_fromrep(0b0100011100011111); // 7.123910566305342
         binary16 y = binary16_fromrep(0b0100011011110011); // 6.951034414294303
         binary16 z = binary16_fromrep(0b0100101100001001); // 14.0703125
         binary16 a = binary16_add(x, y);
         INFO("x = 7.1239105663 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 6.9510344143 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 14.0703125000 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("1.9120035492457976 + 8.080566646608954 = 9.9892578125 (9.9925701959)")
    {
         binary16 x = binary16_fromrep(0b0011111110100101); // 1.9120035492457976
         binary16 y = binary16_fromrep(0b0100100000001010); // 8.080566646608954
         binary16 z = binary16_fromrep(0b0100100011111110); // 9.9892578125
         binary16 a = binary16_add(x, y);
         INFO("x = 1.9120035492 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 8.0805666466 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 9.9892578125 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("2.6743276491644097 + 1.3179706881486564 = 3.9912109375 (3.9922983373)")
    {
         binary16 x = binary16_fromrep(0b0100000101011001); // 2.6743276491644097
         binary16 y = binary16_fromrep(0b0011110101000101); // 1.3179706881486564
         binary16 z = binary16_fromrep(0b0100001111111011); // 3.9912109375
         binary16 a = binary16_add(x, y);
         INFO("x = 2.6743276492 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 1.3179706881 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 3.9912109375 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("9.82760964764326 + 6.606937105104304 = 16.4257812500 (16.4345467527)")
    {
         binary16 x = binary16_fromrep(0b0100100011101001); // 9.82760964764326
         binary16 y = binary16_fromrep(0b0100011010011011); // 6.606937105104304
         binary16 z = binary16_fromrep(0b0100110000011011); // 16.42578125
         binary16 a = binary16_add(x, y);
         INFO("x = 9.8276096476 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 6.6069371051 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 16.4257812500 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("7.9652812383242555 + 9.060715964449786 = 17.0195312500 (17.0259972028)")
    {
         binary16 x = binary16_fromrep(0b0100011111110111); // 7.9652812383242555
         binary16 y = binary16_fromrep(0b0100100010000111); // 9.060715964449786
         binary16 z = binary16_fromrep(0b0100110001000001); // 17.01953125
         binary16 a = binary16_add(x, y);
         INFO("x = 7.9652812383 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 9.0607159644 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 17.0195312500 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("4.140544584074098 + 9.002870014841363 = 13.1367187500 (13.1434145989)")
    {
         binary16 x = binary16_fromrep(0b0100010000100011); // 4.140544584074098
         binary16 y = binary16_fromrep(0b0100100010000000); // 9.002870014841363
         binary16 z = binary16_fromrep(0b0100101010010001); // 13.13671875
         binary16 a = binary16_add(x, y);
         INFO("x = 4.1405445841 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 9.0028700148 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 13.1367187500 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("2.444912967670836 + 8.171045840371058 = 10.6074218750 (10.6159588080)")
    {
         binary16 x = binary16_fromrep(0b0100000011100011); // 2.444912967670836
         binary16 y = binary16_fromrep(0b0100100000010101); // 8.171045840371058
         binary16 z = binary16_fromrep(0b0100100101001101); // 10.607421875
         binary16 a = binary16_add(x, y);
         INFO("x = 2.4449129677 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 8.1710458404 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 10.6074218750 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("4.224058311535385 + 1.5441648960574739 = 5.7666015625 (5.7682232076)")
    {
         binary16 x = binary16_fromrep(0b0100010000111001); // 4.224058311535385
         binary16 y = binary16_fromrep(0b0011111000101101); // 1.5441648960574739
         binary16 z = binary16_fromrep(0b0100010111000100); // 5.7666015625
         binary16 a = binary16_add(x, y);
         INFO("x = 4.2240583115 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 1.5441648961 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 5.7666015625 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("1.2304278090782284 + 9.459166092208292 = 10.6826171875 (10.6895939013)")
    {
         binary16 x = binary16_fromrep(0b0011110011101011); // 1.2304278090782284
         binary16 y = binary16_fromrep(0b0100100010111010); // 9.459166092208292
         binary16 z = binary16_fromrep(0b0100100101010111); // 10.6826171875
         binary16 a = binary16_add(x, y);
         INFO("x = 1.2304278091 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 9.4591660922 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 10.6826171875 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("6.699304833299328 + 2.867339480086609 = 9.5664062500 (9.5666443134)")
    {
         binary16 x = binary16_fromrep(0b0100011010110011); // 6.699304833299328
         binary16 y = binary16_fromrep(0b0100000110111100); // 2.867339480086609
         binary16 z = binary16_fromrep(0b0100100011001000); // 9.56640625
         binary16 a = binary16_add(x, y);
         INFO("x = 6.6993048333 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 2.8673394801 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 9.5664062500 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("0.9487714624515897 + 6.672251662708161 = 7.6206054688 (7.6210231252)")
    {
         binary16 x = binary16_fromrep(0b0011101110010111); // 0.9487714624515897
         binary16 y = binary16_fromrep(0b0100011010101100); // 6.672251662708161
         binary16 z = binary16_fromrep(0b0100011110011110); // 7.62060546875
         binary16 a = binary16_add(x, y);
         INFO("x = 0.9487714625 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 6.6722516627 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 7.6206054688 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("7.207417623787604 + 8.075600366972065 = 15.2773437500 (15.2830179908)")
    {
         binary16 x = binary16_fromrep(0b0100011100110101); // 7.207417623787604
         binary16 y = binary16_fromrep(0b0100100000001001); // 8.075600366972065
         binary16 z = binary16_fromrep(0b0100101110100011); // 15.27734375
         binary16 a = binary16_add(x, y);
         INFO("x = 7.2074176238 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 8.0756003670 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 15.2773437500 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("5.4213433519372405 + 4.364042606239749 = 9.7812500000 (9.7853859582)")
    {
         binary16 x = binary16_fromrep(0b0100010101101011); // 5.4213433519372405
         binary16 y = binary16_fromrep(0b0100010001011101); // 4.364042606239749
         binary16 z = binary16_fromrep(0b0100100011100100); // 9.78125
         binary16 a = binary16_add(x, y);
         INFO("x = 5.4213433519 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 4.3640426062 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 9.7812500000 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("3.946218119651075 + 1.3285243949855619 = 5.2734375000 (5.2747425146)")
    {
         binary16 x = binary16_fromrep(0b0100001111100100); // 3.946218119651075
         binary16 y = binary16_fromrep(0b0011110101010000); // 1.3285243949855619
         binary16 z = binary16_fromrep(0b0100010101000110); // 5.2734375
         binary16 a = binary16_add(x, y);
         INFO("x = 3.9462181197 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 1.3285243950 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 5.2734375000 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("0.9430354363401361 + 9.34218754032393 = 10.2788085938 (10.2852229767)")
    {
         binary16 x = binary16_fromrep(0b0011101110001011); // 0.9430354363401361
         binary16 y = binary16_fromrep(0b0100100010101011); // 9.34218754032393
         binary16 z = binary16_fromrep(0b0100100100100011); // 10.27880859375
         binary16 a = binary16_add(x, y);
         INFO("x = 0.9430354363 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 9.3421875403 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 10.2788085938 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("7.231115613009331 + 8.059024839399797 = 15.2851562500 (15.2901404524)")
    {
         binary16 x = binary16_fromrep(0b0100011100111011); // 7.231115613009331
         binary16 y = binary16_fromrep(0b0100100000000111); // 8.059024839399797
         binary16 z = binary16_fromrep(0b0100101110100100); // 15.28515625
         binary16 a = binary16_add(x, y);
         INFO("x = 7.2311156130 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 8.0590248394 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 15.2851562500 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("5.339212400180333 + 9.270778604597332 = 14.6015625000 (14.6099910048)")
    {
         binary16 x = binary16_fromrep(0b0100010101010110); // 5.339212400180333
         binary16 y = binary16_fromrep(0b0100100010100010); // 9.270778604597332
         binary16 z = binary16_fromrep(0b0100101101001101); // 14.6015625
         binary16 a = binary16_add(x, y);
         INFO("x = 5.3392124002 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 9.2707786046 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 14.6015625000 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("8.64239463771132 + 3.0317753206950027 = 11.6718750000 (11.6741699584)")
    {
         binary16 x = binary16_fromrep(0b0100100001010010); // 8.64239463771132
         binary16 y = binary16_fromrep(0b0100001000010000); // 3.0317753206950027
         binary16 z = binary16_fromrep(0b0100100111010110); // 11.671875
         binary16 a = binary16_add(x, y);
         INFO("x = 8.6423946377 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 3.0317753207 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 11.6718750000 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("3.829916186601986 + 9.22515312906231 = 13.0468750000 (13.0550693157)")
    {
         binary16 x = binary16_fromrep(0b0100001110101000); // 3.829916186601986
         binary16 y = binary16_fromrep(0b0100100010011100); // 9.22515312906231
         binary16 z = binary16_fromrep(0b0100101010000110); // 13.046875
         binary16 a = binary16_add(x, y);
         INFO("x = 3.8299161866 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 9.2251531291 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 13.0468750000 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("8.423461474230646 + 3.9178032556608198 = 12.3378906250 (12.3412647299)")
    {
         binary16 x = binary16_fromrep(0b0100100000110110); // 8.423461474230646
         binary16 y = binary16_fromrep(0b0100001111010101); // 3.9178032556608198
         binary16 z = binary16_fromrep(0b0100101000101011); // 12.337890625
         binary16 a = binary16_add(x, y);
         INFO("x = 8.4234614742 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 3.9178032557 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 12.3378906250 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("0.9457509008945764 + 5.522599014680592 = 6.4648437500 (6.4683499156)")
    {
         binary16 x = binary16_fromrep(0b0011101110010000); // 0.9457509008945764
         binary16 y = binary16_fromrep(0b0100010110000101); // 5.522599014680592
         binary16 z = binary16_fromrep(0b0100011001110111); // 6.46484375
         binary16 a = binary16_add(x, y);
         INFO("x = 0.9457509009 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 5.5225990147 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 6.4648437500 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("1.9185868754899482 + 4.884034142571129 = 6.8007812500 (6.8026210181)")
    {
         binary16 x = binary16_fromrep(0b0011111110101100); // 1.9185868754899482
         binary16 y = binary16_fromrep(0b0100010011100010); // 4.884034142571129
         binary16 z = binary16_fromrep(0b0100011011001101); // 6.80078125
         binary16 a = binary16_add(x, y);
         INFO("x = 1.9185868755 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 4.8840341426 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 6.8007812500 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("2.8916359268029135 + 4.196719264314891 = 7.0859375000 (7.0883551911)")
    {
         binary16 x = binary16_fromrep(0b0100000111001000); // 2.8916359268029135
         binary16 y = binary16_fromrep(0b0100010000110010); // 4.196719264314891
         binary16 z = binary16_fromrep(0b0100011100010110); // 7.0859375
         binary16 a = binary16_add(x, y);
         INFO("x = 2.8916359268 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 4.1967192643 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 7.0859375000 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("9.12146305298389 + 6.301123209029043 = 15.4179687500 (15.4225862620)")
    {
         binary16 x = binary16_fromrep(0b0100100010001111); // 9.12146305298389
         binary16 y = binary16_fromrep(0b0100011001001101); // 6.301123209029043
         binary16 z = binary16_fromrep(0b0100101110110101); // 15.41796875
         binary16 a = binary16_add(x, y);
         INFO("x = 9.1214630530 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 6.3011232090 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 15.4179687500 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("4.323913106210915 + 8.914977605920015 = 13.2343750000 (13.2388907121)")
    {
         binary16 x = binary16_fromrep(0b0100010001010010); // 4.323913106210915
         binary16 y = binary16_fromrep(0b0100100001110101); // 8.914977605920015
         binary16 z = binary16_fromrep(0b0100101010011110); // 13.234375
         binary16 a = binary16_add(x, y);
         INFO("x = 4.3239131062 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 8.9149776059 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 13.2343750000 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("9.16351203772515 + 3.0006762274279364 = 12.1562500000 (12.1641882652)")
    {
         binary16 x = binary16_fromrep(0b0100100010010100); // 9.16351203772515
         binary16 y = binary16_fromrep(0b0100001000000000); // 3.0006762274279364
         binary16 z = binary16_fromrep(0b0100101000010100); // 12.15625
         binary16 a = binary16_add(x, y);
         INFO("x = 9.1635120377 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 3.0006762274 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 12.1562500000 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("4.975987976802281 + 8.131024213222735 = 13.0976562500 (13.1070121900)")
    {
         binary16 x = binary16_fromrep(0b0100010011111001); // 4.975987976802281
         binary16 y = binary16_fromrep(0b0100100000010000); // 8.131024213222735
         binary16 z = binary16_fromrep(0b0100101010001100); // 13.09765625
         binary16 a = binary16_add(x, y);
         INFO("x = 4.9759879768 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 8.1310242132 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 13.0976562500 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("2.205135682955647 + 9.815131734018323 = 12.0175781250 (12.0202674170)")
    {
         binary16 x = binary16_fromrep(0b0100000001101001); // 2.205135682955647
         binary16 y = binary16_fromrep(0b0100100011101000); // 9.815131734018323
         binary16 z = binary16_fromrep(0b0100101000000010); // 12.017578125
         binary16 a = binary16_add(x, y);
         INFO("x = 2.2051356830 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 9.8151317340 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 12.0175781250 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("9.829951332514142 + 9.529549703542127 = 19.3515625000 (19.3595010361)")
    {
         binary16 x = binary16_fromrep(0b0100100011101010); // 9.829951332514142
         binary16 y = binary16_fromrep(0b0100100011000011); // 9.529549703542127
         binary16 z = binary16_fromrep(0b0100110011010110); // 19.3515625
         binary16 a = binary16_add(x, y);
         INFO("x = 9.8299513325 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 9.5295497035 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 19.3515625000 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("6.813918807614644 + 9.98861403049913 = 16.7968750000 (16.8025328381)")
    {
         binary16 x = binary16_fromrep(0b0100011011010000); // 6.813918807614644
         binary16 y = binary16_fromrep(0b0100100011111110); // 9.98861403049913
         binary16 z = binary16_fromrep(0b0100110000110011); // 16.796875
         binary16 a = binary16_add(x, y);
         INFO("x = 6.8139188076 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 9.9886140305 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 16.7968750000 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("4.886199505936616 + 8.745954201947688 = 13.6250000000 (13.6321537079)")
    {
         binary16 x = binary16_fromrep(0b0100010011100010); // 4.886199505936616
         binary16 y = binary16_fromrep(0b0100100001011111); // 8.745954201947688
         binary16 z = binary16_fromrep(0b0100101011010000); // 13.625
         binary16 a = binary16_add(x, y);
         INFO("x = 4.8861995059 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 8.7459542019 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 13.6250000000 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("9.485693615204106 + 4.141389510397957 = 13.6250000000 (13.6270831256)")
    {
         binary16 x = binary16_fromrep(0b0100100010111110); // 9.485693615204106
         binary16 y = binary16_fromrep(0b0100010000100100); // 4.141389510397957
         binary16 z = binary16_fromrep(0b0100101011010000); // 13.625
         binary16 a = binary16_add(x, y);
         INFO("x = 9.4856936152 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 4.1413895104 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 13.6250000000 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("8.523882958848766 + 2.626651578130291 = 11.1484375000 (11.1505345370)")
    {
         binary16 x = binary16_fromrep(0b0100100001000011); // 8.523882958848766
         binary16 y = binary16_fromrep(0b0100000101000000); // 2.626651578130291
         binary16 z = binary16_fromrep(0b0100100110010011); // 11.1484375
         binary16 a = binary16_add(x, y);
         INFO("x = 8.5238829588 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 2.6266515781 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 11.1484375000 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("9.748525657273445 + 1.1082424770524246 = 10.8496093750 (10.8567681343)")
    {
         binary16 x = binary16_fromrep(0b0100100011011111); // 9.748525657273445
         binary16 y = binary16_fromrep(0b0011110001101110); // 1.1082424770524246
         binary16 z = binary16_fromrep(0b0100100101101100); // 10.849609375
         binary16 a = binary16_add(x, y);
         INFO("x = 9.7485256573 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 1.1082424771 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 10.8496093750 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("4.040192541184586 + 8.271805270692454 = 12.3046875000 (12.3119978119)")
    {
         binary16 x = binary16_fromrep(0b0100010000001010); // 4.040192541184586
         binary16 y = binary16_fromrep(0b0100100000100010); // 8.271805270692454
         binary16 z = binary16_fromrep(0b0100101000100111); // 12.3046875
         binary16 a = binary16_add(x, y);
         INFO("x = 4.0401925412 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 8.2718052707 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 12.3046875000 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("4.4280415042630095 + 1.2631470689274416 = 5.6884765625 (5.6911885732)")
    {
         binary16 x = binary16_fromrep(0b0100010001101101); // 4.4280415042630095
         binary16 y = binary16_fromrep(0b0011110100001101); // 1.2631470689274416
         binary16 z = binary16_fromrep(0b0100010110110000); // 5.6884765625
         binary16 a = binary16_add(x, y);
         INFO("x = 4.4280415043 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 1.2631470689 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 5.6884765625 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("0.4076227861739401 + 8.27795098254882 = 8.6809082031 (8.6855737687)")
    {
         binary16 x = binary16_fromrep(0b0011011010000101); // 0.4076227861739401
         binary16 y = binary16_fromrep(0b0100100000100011); // 8.27795098254882
         binary16 z = binary16_fromrep(0b0100100001010111); // 8.680908203125
         binary16 a = binary16_add(x, y);
         INFO("x = 0.4076227862 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 8.2779509825 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 8.6809082031 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("5.204021110297916 + 3.6747608113049415 = 8.8769531250 (8.8787819216)")
    {
         binary16 x = binary16_fromrep(0b0100010100110100); // 5.204021110297916
         binary16 y = binary16_fromrep(0b0100001101011001); // 3.6747608113049415
         binary16 z = binary16_fromrep(0b0100100001110000); // 8.876953125
         binary16 a = binary16_add(x, y);
         INFO("x = 5.2040211103 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 3.6747608113 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 8.8769531250 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("0.35094593755608083 + 6.802841267167068 = 7.1516113281 (7.1537872047)")
    {
         binary16 x = binary16_fromrep(0b0011010110011101); // 0.35094593755608083
         binary16 y = binary16_fromrep(0b0100011011001101); // 6.802841267167068
         binary16 z = binary16_fromrep(0b0100011100100110); // 7.151611328125
         binary16 a = binary16_add(x, y);
         INFO("x = 0.3509459376 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 6.8028412672 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 7.1516113281 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("4.727128085380359 + 5.788432585913147 = 10.5117187500 (10.5155606713)")
    {
         binary16 x = binary16_fromrep(0b0100010010111010); // 4.727128085380359
         binary16 y = binary16_fromrep(0b0100010111001001); // 5.788432585913147
         binary16 z = binary16_fromrep(0b0100100101000001); // 10.51171875
         binary16 a = binary16_add(x, y);
         INFO("x = 4.7271280854 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 5.7884325859 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 10.5117187500 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("7.876270670371564 + 2.5422290319218535 = 10.4160156250 (10.4184997023)")
    {
         binary16 x = binary16_fromrep(0b0100011111100000); // 7.876270670371564
         binary16 y = binary16_fromrep(0b0100000100010101); // 2.5422290319218535
         binary16 z = binary16_fromrep(0b0100100100110101); // 10.416015625
         binary16 a = binary16_add(x, y);
         INFO("x = 7.8762706704 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 2.5422290319 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 10.4160156250 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("2.6999785828495213 + 8.374301672954314 = 11.0664062500 (11.0742802558)")
    {
         binary16 x = binary16_fromrep(0b0100000101100110); // 2.6999785828495213
         binary16 y = binary16_fromrep(0b0100100000101111); // 8.374301672954314
         binary16 z = binary16_fromrep(0b0100100110001000); // 11.06640625
         binary16 a = binary16_add(x, y);
         INFO("x = 2.6999785828 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 8.3743016730 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 11.0664062500 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("7.842345974670425 + 5.79255148035964 = 13.6289062500 (13.6348974550)")
    {
         binary16 x = binary16_fromrep(0b0100011111010111); // 7.842345974670425
         binary16 y = binary16_fromrep(0b0100010111001010); // 5.79255148035964
         binary16 z = binary16_fromrep(0b0100101011010000); // 13.62890625
         binary16 a = binary16_add(x, y);
         INFO("x = 7.8423459747 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 5.7925514804 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 13.6289062500 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("6.430209187056483 + 3.656805564147395 = 10.0859375000 (10.0870147512)")
    {
         binary16 x = binary16_fromrep(0b0100011001101110); // 6.430209187056483
         binary16 y = binary16_fromrep(0b0100001101010000); // 3.656805564147395
         binary16 z = binary16_fromrep(0b0100100100001011); // 10.0859375
         binary16 a = binary16_add(x, y);
         INFO("x = 6.4302091871 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 3.6568055641 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 10.0859375000 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("6.623100499049828 + 1.6996416388783753 = 8.3203125000 (8.3227421379)")
    {
         binary16 x = binary16_fromrep(0b0100011010011111); // 6.623100499049828
         binary16 y = binary16_fromrep(0b0011111011001100); // 1.6996416388783753
         binary16 z = binary16_fromrep(0b0100100000101001); // 8.3203125
         binary16 a = binary16_add(x, y);
         INFO("x = 6.6231004990 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 1.6996416389 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 8.3203125000 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("1.0465442002891645 + 9.19927213735875 = 10.2412109375 (10.2458163376)")
    {
         binary16 x = binary16_fromrep(0b0011110000101111); // 1.0465442002891645
         binary16 y = binary16_fromrep(0b0100100010011001); // 9.19927213735875
         binary16 z = binary16_fromrep(0b0100100100011110); // 10.2412109375
         binary16 a = binary16_add(x, y);
         INFO("x = 1.0465442003 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 9.1992721374 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 10.2412109375 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("7.742440843431932 + 4.354885651735978 = 12.0937500000 (12.0973264952)")
    {
         binary16 x = binary16_fromrep(0b0100011110111110); // 7.742440843431932
         binary16 y = binary16_fromrep(0b0100010001011010); // 4.354885651735978
         binary16 z = binary16_fromrep(0b0100101000001100); // 12.09375
         binary16 a = binary16_add(x, y);
         INFO("x = 7.7424408434 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 4.3548856517 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 12.0937500000 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("4.561036232827599 + 9.979649118990652 = 14.5351562500 (14.5406853518)")
    {
         binary16 x = binary16_fromrep(0b0100010010001111); // 4.561036232827599
         binary16 y = binary16_fromrep(0b0100100011111101); // 9.979649118990652
         binary16 z = binary16_fromrep(0b0100101101000100); // 14.53515625
         binary16 a = binary16_add(x, y);
         INFO("x = 4.5610362328 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 9.9796491190 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 14.5351562500 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("2.5778224710262574 + 0.5489388361659997 = 3.1250000000 (3.1267613072)")
    {
         binary16 x = binary16_fromrep(0b0100000100100111); // 2.5778224710262574
         binary16 y = binary16_fromrep(0b0011100001100100); // 0.5489388361659997
         binary16 z = binary16_fromrep(0b0100001001000000); // 3.125
         binary16 a = binary16_add(x, y);
         INFO("x = 2.5778224710 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 0.5489388362 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 3.1250000000 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("1.279926793408288 + 2.2220592139590654 = 3.5000000000 (3.5019860074)")
    {
         binary16 x = binary16_fromrep(0b0011110100011110); // 1.279926793408288
         binary16 y = binary16_fromrep(0b0100000001110001); // 2.2220592139590654
         binary16 z = binary16_fromrep(0b0100001100000000); // 3.5
         binary16 a = binary16_add(x, y);
         INFO("x = 1.2799267934 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 2.2220592140 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 3.5000000000 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("1.5212268855612487 + 8.135272313049043 = 9.6533203125 (9.6564991986)")
    {
         binary16 x = binary16_fromrep(0b0011111000010101); // 1.5212268855612487
         binary16 y = binary16_fromrep(0b0100100000010001); // 8.135272313049043
         binary16 z = binary16_fromrep(0b0100100011010011); // 9.6533203125
         binary16 a = binary16_add(x, y);
         INFO("x = 1.5212268856 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 8.1352723130 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 9.6533203125 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("7.967020125071931 + 8.163501331746229 = 16.1210937500 (16.1305214568)")
    {
         binary16 x = binary16_fromrep(0b0100011111110111); // 7.967020125071931
         binary16 y = binary16_fromrep(0b0100100000010100); // 8.163501331746229
         binary16 z = binary16_fromrep(0b0100110000000111); // 16.12109375
         binary16 a = binary16_add(x, y);
         INFO("x = 7.9670201251 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 8.1635013317 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 16.1210937500 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("9.801133700497937 + 2.982502729459969 = 12.7792968750 (12.7836364300)")
    {
         binary16 x = binary16_fromrep(0b0100100011100110); // 9.801133700497937
         binary16 y = binary16_fromrep(0b0100000111110111); // 2.982502729459969
         binary16 z = binary16_fromrep(0b0100101001100011); // 12.779296875
         binary16 a = binary16_add(x, y);
         INFO("x = 9.8011337005 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 2.9825027295 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 12.7792968750 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("5.238363277546543 + 3.3173459267488568 = 8.5546875000 (8.5557092043)")
    {
         binary16 x = binary16_fromrep(0b0100010100111101); // 5.238363277546543
         binary16 y = binary16_fromrep(0b0100001010100010); // 3.3173459267488568
         binary16 z = binary16_fromrep(0b0100100001000111); // 8.5546875
         binary16 a = binary16_add(x, y);
         INFO("x = 5.2383632775 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 3.3173459267 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 8.5546875000 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("2.922300676149617 + 8.684276860010069 = 11.6015625000 (11.6065775362)")
    {
         binary16 x = binary16_fromrep(0b0100000111011000); // 2.922300676149617
         binary16 y = binary16_fromrep(0b0100100001010111); // 8.684276860010069
         binary16 z = binary16_fromrep(0b0100100111001101); // 11.6015625
         binary16 a = binary16_add(x, y);
         INFO("x = 2.9223006761 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 8.6842768600 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 11.6015625000 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("7.499270957287259 + 6.824238481869434 = 14.3203125000 (14.3235094392)")
    {
         binary16 x = binary16_fromrep(0b0100011101111111); // 7.499270957287259
         binary16 y = binary16_fromrep(0b0100011011010011); // 6.824238481869434
         binary16 z = binary16_fromrep(0b0100101100101001); // 14.3203125
         binary16 a = binary16_add(x, y);
         INFO("x = 7.4992709573 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 6.8242384819 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 14.3203125000 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("8.23949769370187 + 6.543582349535624 = 14.7773437500 (14.7830800432)")
    {
         binary16 x = binary16_fromrep(0b0100100000011110); // 8.23949769370187
         binary16 y = binary16_fromrep(0b0100011010001011); // 6.543582349535624
         binary16 z = binary16_fromrep(0b0100101101100011); // 14.77734375
         binary16 a = binary16_add(x, y);
         INFO("x = 8.2394976937 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 6.5435823495 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 14.7773437500 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("0.7987647279903498 + 2.306762865191583 = 3.1049804688 (3.1055275932)")
    {
         binary16 x = binary16_fromrep(0b0011101001100011); // 0.7987647279903498
         binary16 y = binary16_fromrep(0b0100000010011101); // 2.306762865191583
         binary16 z = binary16_fromrep(0b0100001000110101); // 3.10498046875
         binary16 a = binary16_add(x, y);
         INFO("x = 0.7987647280 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 2.3067628652 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 3.1049804688 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("7.284316649810512 + 8.619366667588897 = 15.8984375000 (15.9036833174)")
    {
         binary16 x = binary16_fromrep(0b0100011101001000); // 7.284316649810512
         binary16 y = binary16_fromrep(0b0100100001001111); // 8.619366667588897
         binary16 z = binary16_fromrep(0b0100101111110011); // 15.8984375
         binary16 a = binary16_add(x, y);
         INFO("x = 7.2843166498 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 8.6193666676 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 15.8984375000 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("5.935667808889239 + 0.9168618840439369 = 6.8500976562 (6.8525296929)")
    {
         binary16 x = binary16_fromrep(0b0100010111101111); // 5.935667808889239
         binary16 y = binary16_fromrep(0b0011101101010101); // 0.9168618840439369
         binary16 z = binary16_fromrep(0b0100011011011001); // 6.85009765625
         binary16 a = binary16_add(x, y);
         INFO("x = 5.9356678089 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 0.9168618840 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 6.8500976562 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("8.407918452001974 + 5.867616257651377 = 14.2734375000 (14.2755347097)")
    {
         binary16 x = binary16_fromrep(0b0100100000110100); // 8.407918452001974
         binary16 y = binary16_fromrep(0b0100010111011110); // 5.867616257651377
         binary16 z = binary16_fromrep(0b0100101100100011); // 14.2734375
         binary16 a = binary16_add(x, y);
         INFO("x = 8.4079184520 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 5.8676162577 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 14.2734375000 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("7.951908584729143 + 2.29360518409771 = 10.2421875000 (10.2455137688)")
    {
         binary16 x = binary16_fromrep(0b0100011111110011); // 7.951908584729143
         binary16 y = binary16_fromrep(0b0100000010010110); // 2.29360518409771
         binary16 z = binary16_fromrep(0b0100100100011111); // 10.2421875
         binary16 a = binary16_add(x, y);
         INFO("x = 7.9519085847 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 2.2936051841 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 10.2421875000 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("0.648536227777281 + 9.76477212761575 = 10.4062500000 (10.4133083554)")
    {
         binary16 x = binary16_fromrep(0b0011100100110000); // 0.648536227777281
         binary16 y = binary16_fromrep(0b0100100011100001); // 9.76477212761575
         binary16 z = binary16_fromrep(0b0100100100110100); // 10.40625
         binary16 a = binary16_add(x, y);
         INFO("x = 0.6485362278 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 9.7647721276 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 10.4062500000 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("2.1834452441892847 + 6.8984122255004845 = 9.0761718750 (9.0818574697)")
    {
         binary16 x = binary16_fromrep(0b0100000001011101); // 2.1834452441892847
         binary16 y = binary16_fromrep(0b0100011011100101); // 6.8984122255004845
         binary16 z = binary16_fromrep(0b0100100010001001); // 9.076171875
         binary16 a = binary16_add(x, y);
         INFO("x = 2.1834452442 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 6.8984122255 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 9.0761718750 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("0.8553981507314545 + 1.2545040913184824 = 2.1088867188 (2.1099022420)")
    {
         binary16 x = binary16_fromrep(0b0011101011010111); // 0.8553981507314545
         binary16 y = binary16_fromrep(0b0011110100000100); // 1.2545040913184824
         binary16 z = binary16_fromrep(0b0100000000110111); // 2.10888671875
         binary16 a = binary16_add(x, y);
         INFO("x = 0.8553981507 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 1.2545040913 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 2.1088867188 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("8.161759934874306 + 7.159356673582287 = 15.3125000000 (15.3211166085)")
    {
         binary16 x = binary16_fromrep(0b0100100000010100); // 8.161759934874306
         binary16 y = binary16_fromrep(0b0100011100101000); // 7.159356673582287
         binary16 z = binary16_fromrep(0b0100101110101000); // 15.3125
         binary16 a = binary16_add(x, y);
         INFO("x = 8.1617599349 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 7.1593566736 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 15.3125000000 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("2.9881942686393836 + 0.6763172457652156 = 3.6625976562 (3.6645115144)")
    {
         binary16 x = binary16_fromrep(0b0100000111111001); // 2.9881942686393836
         binary16 y = binary16_fromrep(0b0011100101101001); // 0.6763172457652156
         binary16 z = binary16_fromrep(0b0100001101010011); // 3.66259765625
         binary16 a = binary16_add(x, y);
         INFO("x = 2.9881942686 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 0.6763172458 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 3.6625976562 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("0.683587784537274 + 6.442578813761456 = 7.1245117188 (7.1261665983)")
    {
         binary16 x = binary16_fromrep(0b0011100101110111); // 0.683587784537274
         binary16 y = binary16_fromrep(0b0100011001110001); // 6.442578813761456
         binary16 z = binary16_fromrep(0b0100011100011111); // 7.12451171875
         binary16 a = binary16_add(x, y);
         INFO("x = 0.6835877845 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 6.4425788138 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 7.1245117188 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("3.2784216933399115 + 6.201393279087654 = 9.4765625000 (9.4798149724)")
    {
         binary16 x = binary16_fromrep(0b0100001010001110); // 3.2784216933399115
         binary16 y = binary16_fromrep(0b0100011000110011); // 6.201393279087654
         binary16 z = binary16_fromrep(0b0100100010111101); // 9.4765625
         binary16 a = binary16_add(x, y);
         INFO("x = 3.2784216933 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 6.2013932791 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 9.4765625000 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("8.757233928138737 + 6.132026263163074 = 14.8789062500 (14.8892601913)")
    {
         binary16 x = binary16_fromrep(0b0100100001100000); // 8.757233928138737
         binary16 y = binary16_fromrep(0b0100011000100001); // 6.132026263163074
         binary16 z = binary16_fromrep(0b0100101101110000); // 14.87890625
         binary16 a = binary16_add(x, y);
         INFO("x = 8.7572339281 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 6.1320262632 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 14.8789062500 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("1.438647505380708 + 3.0590948293520572 = 4.4970703125 (4.4977423347)")
    {
         binary16 x = binary16_fromrep(0b0011110111000001); // 1.438647505380708
         binary16 y = binary16_fromrep(0b0100001000011110); // 3.0590948293520572
         binary16 z = binary16_fromrep(0b0100010001111111); // 4.4970703125
         binary16 a = binary16_add(x, y);
         INFO("x = 1.4386475054 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 3.0590948294 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 4.4970703125 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("9.099073231667063 + 6.928423866846466 = 16.0195312500 (16.0274970985)")
    {
         binary16 x = binary16_fromrep(0b0100100010001100); // 9.099073231667063
         binary16 y = binary16_fromrep(0b0100011011101101); // 6.928423866846466
         binary16 z = binary16_fromrep(0b0100110000000001); // 16.01953125
         binary16 a = binary16_add(x, y);
         INFO("x = 9.0990732317 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 6.9284238668 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 16.0195312500 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("3.9017292445940086 + 7.919342688654005 = 11.8183593750 (11.8210719332)")
    {
         binary16 x = binary16_fromrep(0b0100001111001101); // 3.9017292445940086
         binary16 y = binary16_fromrep(0b0100011111101011); // 7.919342688654005
         binary16 z = binary16_fromrep(0b0100100111101000); // 11.818359375
         binary16 a = binary16_add(x, y);
         INFO("x = 3.9017292446 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 7.9193426887 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 11.8183593750 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("9.718670567508067 + 3.382734400826103 = 13.0917968750 (13.1014049683)")
    {
         binary16 x = binary16_fromrep(0b0100100011011011); // 9.718670567508067
         binary16 y = binary16_fromrep(0b0100001011000011); // 3.382734400826103
         binary16 z = binary16_fromrep(0b0100101010001011); // 13.091796875
         binary16 a = binary16_add(x, y);
         INFO("x = 9.7186705675 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 3.3827344008 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 13.0917968750 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("7.021512017239591 + 0.06275293133955695 = 7.0822753906 (7.0842649486)")
    {
         binary16 x = binary16_fromrep(0b0100011100000101); // 7.021512017239591
         binary16 y = binary16_fromrep(0b0010110000000100); // 0.06275293133955695
         binary16 z = binary16_fromrep(0b0100011100010101); // 7.082275390625
         binary16 a = binary16_add(x, y);
         INFO("x = 7.0215120172 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 0.0627529313 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 7.0822753906 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
    SECTION("1.341908541792367 + 1.0888469459712302 = 2.4296875000 (2.4307554878)")
    {
         binary16 x = binary16_fromrep(0b0011110101011110); // 1.341908541792367
         binary16 y = binary16_fromrep(0b0011110001011010); // 1.0888469459712302
         binary16 z = binary16_fromrep(0b0100000011011100); // 2.4296875
         binary16 a = binary16_add(x, y);
         INFO("x = 1.3419085418 = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));
         INFO("y = 1.0888469460 = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));
         INFO("z = 2.4296875000 = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));
         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));
         CHECK(a.rep == z.rep);
    }
}

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

#if 0
    SECTION("float + float")
    {
        float a = std::numeric_limits<float>::max();
        float b = 1.0f;
        float c = a + b;
        CHECK(std::isinf(c));
        CHECK(!std::signbit(c));
        CHECK(c == a);
    }
#endif
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

using namespace half_float::literal;

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

#if 0
    const std::vector<std::tuple<float, float>> vs = {
        { 9.75f  , 0.5625f },
        { 0.5625f, 9.75f   },
        { 9.75f  , 9.75f   },
        { 0.5625f, 0.5625f },
        { 101.5f , 101.5f  },
    };

    for (auto [a, b] : vs) {
        const auto c = a + b;
        INFO("Float32:");
        INFO("a         = " << a);
        INFO("b         = " << b);
        INFO("a + b = c = " << c);

        // using half_float
        const auto t = half_float::half{a};
        const auto u = half_float::half{b};
        const auto v = t + u;

        INFO("HalfFloat:");
        INFO("t         = " << (float)t);
        INFO("u         = " << (float)u);
        INFO("t + u = v = " << (float)v);

        const auto x = binary16_fromfloat(a);
        const auto y = binary16_fromfloat(b);
        const auto z = binary16_add(x, y);

        INFO("Binary16:");
        INFO("x         = " << binary16_tofloat(x));
        INFO("y         = " << binary16_tofloat(y));
        INFO("x + y = z = " << binary16_tofloat(z));

        CHECK((float)t == a);
        CHECK((float)u == b);
        CHECK(binary16_tofloat(x) == (float)a);
        CHECK(binary16_tofloat(y) == (float)b);
        CHECK(binary16_tofloat(z) == (float)v);

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
        INFO("curr_diff = " << curr_diff);
        INFO("next_diff = " << next_diff);
        INFO("prev_diff = " << prev_diff);

        CHECK(curr_diff <= 0.001);  // TODO: need to size this based on the magnitude of c
        CHECK(curr_diff <= next_diff);
        CHECK(curr_diff <= prev_diff);

        CHECK(0 == 1);
    }
#endif
}

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
