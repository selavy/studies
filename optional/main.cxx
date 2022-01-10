#include <catch2/catch.hpp>
#include "optional.hpp"

template <std::size_t N>
struct Data
{
    char buf[N];
};

TEST_CASE("storage_for")
{
    using namespace pl::detail;

    SECTION("int")
    {
        storage_for<int> s;
        CHECK(sizeof(s) == sizeof(int));
        CHECK(alignof(s) == alignof(int));
    }

    SECTION("double")
    {
        storage_for<double> s;
        CHECK(sizeof(s) == sizeof(double));
        CHECK(alignof(s) == alignof(double));
    }

    SECTION("user defined")
    {
        using D3 = Data<3>;
        using D5 = Data<5>;

        CHECK(sizeof(D3) == 3);
        CHECK(sizeof(storage_for<D3>) == sizeof(D3));
        CHECK(alignof(storage_for<D3>) == alignof(D3));

        CHECK(sizeof(D5) == 5);
        CHECK(sizeof(storage_for<D5>) == sizeof(D5));
        CHECK(alignof(storage_for<D5>) == alignof(D5));
    }
}

TEST_CASE("storage")
{
}
