#include <catch2/catch.hpp>
#include <cstdio>
#include <cstring>

#include "fp.h"

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

struct FP
{
    struct {
        uint32_t sig : 23;
        uint32_t exp : 8;
        uint32_t sgn : 1;
    } __attribute__((packed)) r;

    // union {
    //     struct {
    //         uint32_t sgn : 1;
    //         uint32_t exp : 8;
    //         uint32_t sig : 23;
    //     } r;
    //     float    flt;
    //     uint32_t uint;
    // } u;
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

#if 0
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
#endif

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
