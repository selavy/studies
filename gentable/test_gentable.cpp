#include <catch2/catch.hpp>
#include "gentable_v1.h"
#include <unordered_map>
#include <vector>
#include <utility>


struct KeyEq
{
    u32 x, y;

    constexpr KeyEq(u32 x_, u32 y_) noexcept : x(x_), y(y_) {}

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
    std::size_t operator()(KeyEq k) noexcept
    {
        return std::hash<u64>()((static_cast<u64>(k.x) << 32) | k.y);
    }
};

} // namespace std

TEST_CASE("Insert up to 32 values with same x", "gentable")
{
    gentbl::v1::Table t;
    const std::vector<u32> keys = {
        1,
        2,
        56,
        123491234,
    };

    for (const auto key : keys) {
        for (int i = 0; i < 32; ++i) {
            auto b = t.insert(key, rand(), rand());
            CHECK(b == true);
        }
        for (int i = 0; i < 64; ++i) {
            auto b = t.insert(key, rand(), rand());
            CHECK(b == false);
        }
    }

    // const std::vector<std::tuple<u32, u32, u64>> values{
    //     { 0x664c5519, 0xc975ce44, 0x4bf5e1598b50f6f8 },
    //     { 0xaba282a5, 0xa6b6c348, 0xe4a1e31d154ce3d5 },
    //     { 0x0faa1406, 0xca8bb57a, 0xbc421174550c8baa },
    //     { 0x98383040, 0x52b948c2, 0x43830badb06003d9 },
    //     { 0x2744eedf, 0x0b3b31af, 0x6d0c101e5448de48 },
    //     { 0x1d53e80f, 0x0cc5d942, 0xdd6b091b64071aed },
    //     { 0x05f9dec6, 0x445d95a1, 0x05f7fad66354bd83 },
    //     { 0x75831659, 0x781ca030, 0xb70895da981c205c },
    //     { 0xf096fd9f, 0xc1f8e119, 0x4f39f80d5d8de9be },
    //     { 0x0eb7c637, 0x9d76df35, 0xd7a5d7251e48d964 },
    //     { 0xe088b9f9, 0x44580bef, 0xb94208101ce2ecbc },
    //     { 0x04b0e401, 0x6485fbfd, 0x5e335b8b94a7a864 },
    //     { 0xdee17fe4, 0xf55a9416, 0xeb2cf48a880cae78 },
    //     { 0x0ed0e3ba, 0x94c6c3f5, 0xe8fdcc9a4dd5c2fb },
    //     { 0x3ab68534, 0xc61f5230, 0x664b59bb6dd9c143 },
    //     { 0x6ac994c8, 0x9f20ef3c, 0xc92159ab2eb96ec8 },
    //     { 0xf2faa510, 0x18939b07, 0x08f203222e136191 },
    //     { 0x4fd12ed9, 0xc6622f04, 0x409f66b475925bc6 },
    //     { 0x740a4881, 0xb596d003, 0xb1b014d99d8b493c },
    //     { 0x50032245, 0xe17ef6ce, 0xf5244b53e86fc952 },
    //     { 0xad700fe1, 0xeff9e046, 0x3db49b8f1459a987 },
    //     { 0x00bdf8e4, 0xfcb076a3, 0x07de4e6f9705e14e },
    //     { 0x42e7a2ea, 0xba63bc56, 0xdb9f0ea288b7b6e3 },
    //     { 0x2771854b, 0xd25654f6, 0xe5b1891a95b8831a },
    //     { 0x30373c9f, 0x945da1a4, 0xe37cbdd84c3f94fa },
    //     { 0xbeb8504d, 0xabcd722c, 0x670c10cac7c74593 },
    //     { 0x8334ad5c, 0xac86425f, 0xb7e12de3ab435ff1 },
    //     { 0xe95a3a47, 0x098f62d8, 0x765dd7c423a787d6 },
    //     { 0xe0a69f87, 0xad7e934e, 0xed39deed769c79f2 },
    //     { 0xf4dc1bd6, 0xc50e239d, 0x5463b654de163e3a },
    //     { 0xd1e6a924, 0x41b684ce, 0x3c5897eaf1359d02 },
    //     { 0x8413a5dc, 0x5eca22a5, 0xe3b858302fd98be6 },
    // };

    // std::unordered_map<KeyEq, u64> umap;
    // for (auto&& [x, y, z] : values) {
    //     umap[KeyEq(x, y)] = z;
    // }
}
