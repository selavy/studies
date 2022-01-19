#include <catch2/catch.hpp>
#include "optional.hpp"
#include <iostream>

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

    SECTION("array")
    {
        storage_for<int[5]> s;
        CHECK(sizeof(s)  == sizeof(int)*5);
        CHECK(alignof(s) == alignof(int));
    }
}

struct Pair {
    Pair(int x_, int y_) : x{x_}, y{y_} {}

    int x;
    int y;
};

TEST_CASE("optional")
{

    SECTION("default construct is not engaged")
    {
        pl::optional<int> o;
        CHECK(!o);

        o.emplace(1);
        CHECK(!!o);
        CHECK(*o == 1);
    }

    SECTION("construct with value is engaged")
    {
        pl::optional<int> o = 1;
        CHECK(o);
        CHECK(*o == 1);
    }

    SECTION("var args construction")
    {
        pl::optional<Pair> o{1, 2};
        CHECK(o);
        CHECK((*o).x == 1);
        CHECK((*o).y == 2);
        CHECK(o->x == 1);
        CHECK(o->y == 2);
    }

    SECTION("var args with make_optional")
    {
        auto o = pl::make_optional<Pair>(1, 2);
        CHECK(o);
        CHECK((*o).x == 1);
        CHECK((*o).y == 2);
    }
}

template <class T>
auto default_size()
{
    return sizeof(T) + std::max(sizeof(bool), alignof(T));
}

TEST_CASE("default sizes")
{
    CHECK(sizeof(pl::optional<int> ) == default_size<int>());
    CHECK(sizeof(pl::optional<int&>) == sizeof(int*));
    CHECK(sizeof(pl::optional<double>) == default_size<double>());
    CHECK(sizeof(pl::optional<char>) == default_size<char>());
    CHECK(sizeof(pl::optional<void>) == sizeof(bool));
}

TEST_CASE("optional<void>")
{
    pl::optional<void> o1;
    CHECK(o1.is_engaged() == false);
    CHECK(!!o1 == false);

    o1.emplace();
    CHECK(o1.is_engaged() == true);
    CHECK(!!o1 == true);
}

TEST_CASE("optional<T&>")
{
    SECTION("optional<int&>")
    {
        pl::optional<int&> o;
        CHECK(!o.is_engaged());
        CHECK(!o);

        int x = 42;
        o.emplace(x);
        CHECK(o.is_engaged());
        CHECK(!!o);
        CHECK(*o == x);
    }
}

#if 0
TEST_CASE("value_or")
{
    pl::optional<int> o1;
    CHECK(o1.value_or(42) == 42);
    o1 = 55;
    CHECK(o1.value_or(42) == 55);
}
#endif
