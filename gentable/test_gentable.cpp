#include <catch2/catch.hpp>
#include "gentable_v1.h"
#include <unordered_map>
#include <iostream>
#include <vector>
#include <utility>

struct KeyEq
{
    uint32_t x, y;

    constexpr KeyEq(uint32_t x_, uint32_t y_) noexcept : x(x_), y(y_) {}

    friend constexpr bool operator==(KeyEq lhs, KeyEq rhs) noexcept
    {
        return lhs.x == rhs.x && lhs.y == rhs.y;
    }

    friend constexpr bool operator!=(KeyEq lhs, KeyEq rhs) noexcept
    {
        return !(lhs == rhs);
    }
};

namespace std {

template <> struct hash<KeyEq>
{
    std::size_t operator()(KeyEq k) const noexcept
    {
        uint64_t x = k.x;
        uint64_t y = k.y;
        return std::hash<uint64_t>()((x << 32) | y);
    }
};

} // namespace std

struct STLTable
{
    bool insert(uint32_t x, uint32_t y, uint64_t z) noexcept
    {
        if (gen != x) {
            gen = x;
            t.clear();
        }
        if (t.size() >= 32) {
            return false;
        }
        t[y] = z;
        return true;
    }

    uint64_t find(uint32_t x, uint32_t y) const noexcept
    {
        if (gen != x) {
            return 0;
        }
        auto iter = t.find(y);
        return iter == t.cend() ? 0 : iter->second;
    }

    int size(uint32_t x) const noexcept
    {
        return static_cast<int>(t.size());
    }

    uint32_t gen = 0;
    std::unordered_map<uint32_t, uint64_t> t = {};
};

TEST_CASE("Insert up to 32 values with same x", "[gentbl]")
{
    gentbl::v1::Table t;
    const std::vector<uint32_t> keys = {
        1,
        2,
        56,
        123491234,
    };

    for (const auto key : keys) {
        for (int i = 0; i < 32; ++i) {
            CHECK(t.size(key) == i);
            auto b = t.insert(key, rand(), rand());
            CHECK(b == true);
            CHECK(t.size(key) == i + 1);
        }
        for (int i = 0; i < 64; ++i) {
            auto b = t.insert(key, rand(), rand());
            CHECK(b == false);
            CHECK(t.size(key) == 32);
        }
    }
}

TEST_CASE("Insert random values and look them up", "[gentbl]")
{
    const std::vector<uint32_t> keys = {
        0xb716ee84u, 0xdf9a67d3u, 0xad84a1efu, 0xd82256f7u,
        0xf27e6488u, 0x16de899eu, 0x32f94bbdu, 0x551d830au,
        0x77a59b7au, 0xc7f0c29au, 0x509c41cau, 0xf342a2bfu,
        0xa7d2e1aeu, 0x3c82d3ccu, 0x571a4135u, 0x7d887b50u,
        0xe5c271f2u, 0xc8728a6fu, 0xc36a05d3u, 0x89949952u,
        0x30e03847u, 0x31d20cc0u, 0xd71535d3u, 0x50a6d844u,
        0xec885b1du, 0x406b18c4u, 0x5e2ac2acu, 0x874917bcu,
        0x6a1e9690u, 0x6615ddadu, 0x963df142u, 0x817bd22fu,
        0x5525e201u, 0x19da087fu, 0x958b4ee2u, 0x34188c5fu,
        0x2dc92eb8u, 0x04467517u, 0xb1395fabu, 0xe123698bu,
        0xb2724808u, 0x5f19cc63u, 0x7d80dc07u, 0xf2d83380u,
        0x7954bc58u, 0x7b99ee30u, 0x67d7550au, 0x5d7c0a55u,
        0x8dca9486u, 0xf97b92beu, 0xa0295512u, 0x1338be5au,
        0x5fb07720u, 0x79b94cd7u, 0x17436febu, 0x23ed61ceu,
        0x3d3b336au, 0xbefc7065u, 0x63d09d9au, 0xb6433a79u,
        0xd0fb69a6u, 0x8ebbdd01u, 0x63a8ed99u, 0xd51df47cu,
    };

    const std::vector<std::tuple<uint32_t, uint64_t>> vals = {
        { 0xbc4a1950u, 0xae9a74f6266fcb96ull },
        { 0xa10881fcu, 0x69d0781174e2e4e9ull },
        { 0xbbd5da23u, 0x010f367034b63cc3ull },
        { 0x19f0ee65u, 0x59cd1981215f7b99ull },
        { 0xfc15d623u, 0x7aa0ef62fe6ab33eull },
        { 0x76af263du, 0x49e4c3d885859e2eull },
        { 0xd59fc7d6u, 0xf520358a38a35f22ull },
        { 0xd00e893cu, 0x44431bd8cf49ea5eull },
        { 0x53298a28u, 0xfb64c58502022042ull },
        { 0x7840bc71u, 0x9c1638f4fc54d161ull },
        { 0x30862cbeu, 0x16425398aa0dca8aull },
        { 0x7f2df5adu, 0x7018183082fffb11ull },
        { 0xa961531cu, 0xc1bce2322d57c6d3ull },
        { 0x82e8b699u, 0x5199247eb9ba78d7ull },
        { 0xaa2d6f37u, 0xfeb1c9df10fbb9e3ull },
        { 0xf25a35f6u, 0x069bdc1f850837c0ull },
        { 0x6b91882fu, 0xe1cb52beba671317ull },
        { 0xd8787979u, 0x84a12c60a6179d2dull },
        { 0xdca46460u, 0x73a08641c741c99dull },
        { 0x066bd1a6u, 0x5d8fad03537ae0d6ull },
        { 0x13a8a338u, 0xed7db4ed21f33c02ull },
        { 0x372c32dau, 0xf00ce71a99c00758ull },
        { 0x86f36bc3u, 0xd26ce45a127f1458ull },
        { 0xa09d275bu, 0x370824cc9d5dcbf6ull },
        { 0x1c11cf5cu, 0xbb2d67c708b4d1e0ull },
        { 0xe80365f0u, 0x3cc3bf509e5d207aull },
        { 0x9a027236u, 0xbcdefd8ed88f81d9ull },
        { 0xd5b63f65u, 0x47f9f7971f84c93aull },
        { 0x6175d814u, 0x4d9026428d57ec4dull },
        { 0x38edba26u, 0xb194e3c14bbc8e9bull },
        { 0x3601e0c0u, 0x3879ff5288e127a9ull },
        { 0x09d36f68u, 0x00a38350b80c4743ull },
    };

    const std::vector<uint32_t> miss = {
        0x05acd140, 0xfa1688d0, 0x9a595bec, 0x5ba458dd,
        0xe4daae98, 0x7cc3c06b, 0xe32a3655, 0x43a534b0,
        0x98970994, 0x5f2c1e18, 0xb04945c7, 0x3c8fa0bf,
        0xa923aae7, 0x997c4ec6, 0xce79f58a, 0x7dd4d6ea,
        0x75639eed, 0x85fb20fb, 0x4f70ce97, 0x2dcb5a2d,
        0x28fb3b16, 0xf2cf3693, 0xc5777eca, 0x271ad41c,
        0xfa0358b9, 0x8aabf9fd, 0xa49a69f2, 0xa8cd8118,
        0x5a1c693b, 0x97730443, 0x8ee2996a, 0x0ae5f5cd,
        0x05ce12c1, 0x089219fd, 0x6be0d5a3, 0xee44c74f,
        0x8bb8b409, 0xd752c10c, 0xcda0a567, 0x8e2a7d33,
        0xd597f1a7, 0xdc40d801, 0x8a4a3f5c, 0xba7cbc24,
        0x318c432b, 0x0e33f72f, 0x19ff149a, 0x95654489,
        0x7467bb95, 0xcc240d5e, 0x5b66e979, 0x04081103,
        0x09ea4ae1, 0x03953849, 0x9ccb25c0, 0x787ce2ae,
        0x72a56ffe, 0xb1e1bed9, 0xbb91ccbe, 0xa90b2fa7,
        0x6650602f, 0x59b34aad, 0x8a0132a1, 0x59c23517,
    };

    auto RunTest = [&](auto t)
    {
        for (const auto x : keys) {
            for (auto&& [y, z] : vals) {
                bool result = t.insert(x, y, z);
                CHECK(result == true);

                auto found = t.find(x, y);
                CHECK(found == z);
            }

            for (auto&& [y, z] : vals) {
                auto found = t.find(x, y);
                CHECK(found == z);
            }

            for (auto y : miss) {
                bool result = t.find(x, y);
                CHECK(result == false);
            }
        }
    };

    SECTION("STLTable")
    {
        STLTable t;
        RunTest(t);
    }

    SECTION("Table v1")
    {
        gentbl::v1::Table t;
        RunTest(t);
    }
}
