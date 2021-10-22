#include <catch2/catch.hpp>
#include "idvec.h"

#include "small_vector.h"

struct GatewayId
{
    GatewayId() {}
    GatewayId(unsigned id_) : id{id_} {}
    operator unsigned() const { return id; }

    unsigned id = 0;
};


TEST_CASE("IdMap")
{
    auto id = GatewayId{42};
    CHECK(id == 42u);
}

TEST_CASE("SmallVector")
{
    SmallVector<int> v;
    v.push_back(1);
    v.push_back(2);
    v.push_back(3);
    CHECK(v.size() == 3u);

    v.clear();

    std::size_t N = 64;
    for (std::size_t i = 0; i < N; ++i) {
        CHECK(v.size() == i);
        v.push_back(i);
        CHECK(v.size() == i + 1);
    }

    for (std::size_t i = 0; i < N; ++i) {
        REQUIRE(!v.empty());
        CHECK(v.back()  == N - i - 1);
        CHECK(v.front() == 0);
        v.pop_back();
    }
    CHECK(v.empty());
}

// TODO: test resize()
