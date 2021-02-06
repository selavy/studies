#include <catch2/catch.hpp>
#include <cstr.h>
#include <string>
#include <cstring>
#include <vector>


TEST_CASE("string length")
{
    std::vector<std::string> cases = {
        "",
        "Hello, World",
        "aaaaaaaaaaaaaaaaaaaaaaaaaaaa;laskjdfl;kjasl;dkfjaskl;jdfkl;jasdf",
        "  asdf Ha;lskdfn",
        "\001FIX4.2\00142=Value\001",
    };

    for (const auto expect : cases) {
        cstr* s = cstr_new(expect.c_str(), expect.size());
        REQUIRE(s != NULL);
        CHECK(cstr_size(s)    == expect.size());
        CHECK(cstr_len(s)     == expect.size());
        CHECK(cstr_length(s)  == expect.size());
        CHECK(cstr_capacity(s) >= expect.size());
        CHECK(strlen(cstr_str(s)) == expect.size());
        CHECK(strcmp(cstr_str(s), expect.c_str()) == 0);
        cstr_del(s);
    }
}

TEST_CASE("String comparisons")
{
    std::vector<std::string> vals = {
        "a",
        "Hello, World",
        "\001FIX4.2\00142=Value\001",
        "This is a test",
        "ZZZZ",
        "zAzA",
        "Pizza",
        "12345",
        "023",
        "9999",
    };

    for (auto v1 : vals) {
        for (auto v2 : vals) {
            INFO("Comparing \"" << v1 << "\" vs \"" << v2 << '"');
            cstr* s1 = cstr_new(v1.c_str(), v1.size());
            cstr* s2 = cstr_new(v2.c_str(), v2.size());

            CHECK(cstr_str(s1) == v1);
            CHECK(cstr_str(s2) == v2);

            SECTION("equals")
            {
                bool expect = v1 == v2;
                bool result = !!cstr_eq(s1, s2);
                CHECK(expect == result);
            }
            SECTION("not equals")
            {
                bool expect = v1 != v2;
                bool result = !!cstr_neq(s1, s2);
                CHECK(expect == result);
            }
            SECTION("less than")
            {
                bool expect = v1 < v2;
                bool result = !!cstr_lt(s1, s2);
                CHECK(expect == result);
            }
            SECTION("less than or equals")
            {
                bool expect = v1 <= v2;
                bool result = !!cstr_lte(s1, s2);
                CHECK(expect == result);
            }
            SECTION("greater than")
            {
                bool expect = v1 > v2;
                bool result = !!cstr_gt(s1, s2);
                CHECK(expect == result);
            }
            SECTION("greater than or equals")
            {
                bool expect = v1 >= v2;
                bool result = !!cstr_gte(s1, s2);
                CHECK(expect == result);
            }

            cstr_del(s1);
            cstr_del(s2);
        }
    }
}

TEST_CASE("SSO")
{
    auto* no_calloc = +[](size_t nmemb, size_t size) -> void*
    {
        INFO("Tried to call calloc with nmemb=" << nmemb << " size=" << size);
        CHECK(false);
        return nullptr;
    };
    auto* no_realloc = +[](void*, size_t size) -> void*
    {
        INFO("Tried to call realloc with size=" << size);
        CHECK(false);
        return nullptr;
    };
    auto* no_free = +[](void*, size_t size) -> void
    {
        INFO("Tried to call free with size=" << size);
        CHECK(false);
    };
    cstr_set_allocator(cstr_alloc_t{ no_calloc, no_realloc, no_free });

    std::vector<std::string> cases = {
        "",
        "1234567890123456789",
        "asdf asdf FFF\00189",
        "Hello, World",
        "Bye Bye",
    };

    CHECK(sizeof(cstr) == 24);

    for (auto expect : cases) {
        cstr s = cstr_make(expect.c_str(), expect.size());
        CHECK(cstr_isinline_(&s) != 0);
        CHECK(cstr_len(&s) == expect.size());
        CHECK(std::string{cstr_str(&s)} == expect);
        cstr_destroy(&s);
    }

    cstr_reset_allocator_to_default_();
}
