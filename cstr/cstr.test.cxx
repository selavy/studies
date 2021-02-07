#include <catch2/catch.hpp>
#include <cstr.h>
#include <string>
#include <cstring>
#include <vector>

std::string to_string(const cstr* s)
{
    return std::string{cstr_str(s), cstr_len(s)};
}

std::string to_string(const cstr s)
{
    return std::string{cstr_str(&s), cstr_len(&s)};
}

std::string to_string(cstrview v)
{
    return std::string{cstrview_str(v), cstrview_len(v)};
}

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
    auto* no_realloc = +[](void*, size_t nmemb, size_t size) -> void*
    {
        INFO("Tried to call realloc with nmemb=" << nmemb << " size=" << size);
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

TEST_CASE("Append")
{
    std::vector<std::string> cases = {
        "Hello",
        "World",
        "a",
        "1234",
        "asdf;lkajsdf",
        "\001FIX4.2\001",
        "This is a VerY LoNG STRING With Some \001 Embeeded stuff \004!!!",
        "Anothasdefl;k;lkj asd;fklja;slkdjf;lkasjdf;lkj asl;df lkjq23r-09127u35knvzsxlckn asdf'piojas'dfkljas[df9ohnknzxcvl'kjzxcdfzXCLfkjas'dlfjas'iodfjasdf]9823489-71280612349076123496812349-6123-498123-4987612-98347-8917234-9871234-9876y2134-98712395478612-93456-986123-4987612-394876123-9846y12038746890-7y",
        "",
        "1",
    };

    for (const auto a : cases) {
        for (const auto b : cases) {
            INFO("a = \"" << a << "\", b = \"" << b << '"');
            cstr a2 = cstr_make(a.c_str(), a.size());
            cstr b2 = cstr_make(b.c_str(), b.size());

            auto expect = a;
            expect += b;

            cstr* actual = cstr_append(&a2, &b2);

            REQUIRE(actual != NULL);
            CHECK(cstr_len(actual)  == expect.size());
            CHECK(to_string(actual) == expect);

            cstr_destroy(&b2);
            cstr_destroy(&a2);
        }
    }

}

TEST_CASE("Prepend")
{
    std::vector<std::string> cases = {
        "Hello",
        "World",
        "a",
        "1234",
        "asdf;lkajsdf",
        "\001FIX4.2\001",
        "This is a VerY LoNG STRING With Some \001 Embeeded stuff \004!!!",
        "Anothasdefl;k;lkj asd;fklja;slkdjf;lkasjdf;lkj asl;df lkjq23r-09127u35knvzsxlckn asdf'piojas'dfkljas[df9ohnknzxcvl'kjzxcdfzXCLfkjas'dlfjas'iodfjasdf]9823489-71280612349076123496812349-6123-498123-4987612-98347-8917234-9871234-9876y2134-98712395478612-93456-986123-4987612-394876123-9846y12038746890-7y",
        "",
        "1",
    };

    for (const auto a : cases) {
        for (const auto b : cases) {
            cstr a2 = cstr_make(a.c_str(), a.size());
            cstr b2 = cstr_make(b.c_str(), b.size());
            auto expect = b + a;
            INFO("a=\"" << a << "\" b=\"" << b << "\" expect=\"" << expect << '"');
            cstr* actual = cstr_prepend(&a2, &b2);
            REQUIRE(actual != NULL);
            CHECK(cstr_len(actual)  == expect.size());
            CHECK(to_string(actual) == expect);
            cstr_destroy(&b2);
            cstr_destroy(&a2);
        }
    }

}

TEST_CASE("Take")
{
    SECTION("SSO")
    {
        auto* no_calloc = +[](size_t nmemb, size_t size) -> void*
        {
            INFO("Tried to call calloc with nmemb=" << nmemb << " size=" << size);
            CHECK(false);
            return nullptr;
        };
        auto* no_realloc = +[](void*, size_t nmemb, size_t size) -> void*
        {
            INFO("Tried to call realloc with nmemb=" << nmemb << " size=" << size);
            CHECK(false);
            return nullptr;
        };
        auto* no_free = +[](void*, size_t size) -> void
        {
            INFO("Tried to call free with size=" << size);
            CHECK(false);
        };
        cstr_set_allocator(cstr_alloc_t{ no_calloc, no_realloc, no_free });

        std::string val = "Hello, World";
        cstr a = cstr_make(val.c_str(), val.size());
        REQUIRE(cstr_isinline_(&a));
        CHECK(cstr_size(&a) == val.size());
        CHECK(to_string(a) == val);

        // take less than there
        cstr* r = cstr_take(&a, 5);
        REQUIRE(r != NULL);
        CHECK(to_string(a) == "Hello");

        // take more than there
        r = cstr_take(&a, 7);
        REQUIRE(r != NULL);
        CHECK(to_string(a) == "Hello");

        r = cstr_take(&a, 0);
        REQUIRE(r != NULL);
        CHECK(to_string(a) == "");

        cstr_destroy(&a);

        cstr_reset_allocator_to_default_();
    }

    SECTION("Long string")
    {
        std::string val = "This is a very long string that won't fit in SSO";
        cstr a = cstr_make(val.c_str(), val.size());
        REQUIRE(!cstr_isinline_(&a));
        CHECK(cstr_size(&a) == val.size());
        CHECK(to_string(a) == val);

        // take less than there
        cstr* r = cstr_take(&a, 23);
        REQUIRE(r != NULL);
        CHECK(to_string(a) == "This is a very long str");

        // take more than there
        r = cstr_take(&a, 100);
        REQUIRE(r != NULL);
        CHECK(to_string(a) == "This is a very long str");

        // take exactly what is there
        r = cstr_take(&a, 23);
        REQUIRE(r != NULL);
        CHECK(to_string(a) == "This is a very long str");

        r = cstr_take(&a, 10);
        REQUIRE(r != NULL);
        CHECK(to_string(a) == "This is a ");

        r = cstr_take(&a, 5);
        REQUIRE(r != NULL);
        CHECK(to_string(a) == "This ");

        r = cstr_take(&a, 1);
        REQUIRE(r != NULL);
        CHECK(to_string(a) == "T");

        r = cstr_take(&a, 0);
        REQUIRE(r != NULL);
        CHECK(to_string(a) == "");

        cstr_destroy(&a);
    }
}

TEST_CASE("Drop")
{
    SECTION("SSO")
    {
        auto* no_calloc = +[](size_t nmemb, size_t size) -> void*
        {
            INFO("Tried to call calloc with nmemb=" << nmemb << " size=" << size);
            CHECK(false);
            return nullptr;
        };
        auto* no_realloc = +[](void*, size_t nmemb, size_t size) -> void*
        {
            INFO("Tried to call realloc with nmemb=" << nmemb << " size=" << size);
            CHECK(false);
            return nullptr;
        };
        auto* no_free = +[](void*, size_t size) -> void
        {
            INFO("Tried to call free with size=" << size);
            CHECK(false);
        };

        cstr_set_allocator(cstr_alloc_t{ no_calloc, no_realloc, no_free });
        std::string val = "Hello, World";
        cstr a = cstr_make(val.c_str(), val.size());
        REQUIRE(cstr_isinline_(&a));
        CHECK(cstr_size(&a) == val.size());
        CHECK(to_string(&a) == val);

        cstrview v = cstrview_init(val.c_str(), val.size());
        CHECK(cstrview_len(v) == val.size());
        CHECK(to_string(v)    == val);

        // take less than there
        cstr* r = cstr_drop(&a, 1);
        REQUIRE(r != NULL);
        CHECK(to_string(a) == "ello, World");
        CHECK(cstr_size(&a) == 11);

        v = cstrview_drop(v, 1);
        CHECK(cstrview_len(v) == cstr_len(&a));
        CHECK(to_string(v)    == to_string(a));

        // take less than there
        r = cstr_drop(&a, 5);
        REQUIRE(r != NULL);
        CHECK(to_string(a) == " World");
        CHECK(cstr_size(&a) == 6);

        v = cstrview_drop(v, 5);
        CHECK(cstrview_len(v) == cstr_len(&a));
        CHECK(to_string(v)    == to_string(a));

        // take more than there
        r = cstr_drop(&a, 1000);
        REQUIRE(r != NULL);
        CHECK(to_string(a) == "");
        CHECK(cstr_size(&a) == 0);

        v = cstrview_drop(v, 1000);
        CHECK(cstrview_len(v) == cstr_len(&a));
        CHECK(to_string(v)    == to_string(a));

        cstr_destroy(&a);

        cstr_reset_allocator_to_default_();
    }

    SECTION("Long String")
    {
        std::string val = "This is a very long string that won't fit in SSO";
        cstr a = cstr_make(val.c_str(), val.size());
        REQUIRE(!cstr_isinline_(&a));
        CHECK(cstr_size(&a) == val.size());
        CHECK(to_string(a) == val);

        // take less than there
        cstr* r = cstr_drop(&a, 1);
        REQUIRE(r != NULL);
        CHECK(to_string(a) == "his is a very long string that won't fit in SSO");
        CHECK(cstr_size(&a) == 47);

        // take less than there
        r = cstr_drop(&a, 5);
        REQUIRE(r != NULL);
        CHECK(to_string(a) == "s a very long string that won't fit in SSO");
        CHECK(cstr_size(&a) == 42);

        // take less than there
        r = cstr_drop(&a, 7);
        REQUIRE(r != NULL);
        CHECK(to_string(a) == "y long string that won't fit in SSO");
        CHECK(cstr_size(&a) == 35);

        // take less than there
        r = cstr_drop(&a, 12);
        REQUIRE(r != NULL);
        CHECK(to_string(a) == "g that won't fit in SSO");
        CHECK(cstr_size(&a) == 23);

        // drop nothing
        r = cstr_drop(&a, 0);
        REQUIRE(r != NULL);
        CHECK(to_string(a) == "g that won't fit in SSO");
        CHECK(cstr_size(&a) == 23);

        // take more than there
        r = cstr_drop(&a, 1000);
        REQUIRE(r != NULL);
        CHECK(to_string(a) == "");
        CHECK(cstr_size(&a) == 0);

        cstr_destroy(&a);
    }
}

TEST_CASE("Insert")
{
    SECTION("SSO")
    {
        const std::string a = "Hello";
        const std::string b = ", World!";
        cstr b2 = cstr_make(b.c_str(), b.size());
        for (std::size_t pos = 0; pos <= a.size(); ++pos) {
            INFO("a = \"" << a << "\" pos=" << pos);
            std::string expect = a;
            cstr        actual = cstr_make(a.c_str(), a.size());

            CHECK(cstr_isinline_(&actual));
            CHECK(cstr_isinline_(&b2));

            expect.insert(pos, b);
            cstr* r = cstr_insert(&actual, pos, &b2);
            REQUIRE(r != nullptr);

            CHECK(cstr_isinline_(&actual));

            CHECK(cstr_len(&actual)  == expect.size());
            CHECK(to_string(&actual) == expect);

            cstr_destroy(&actual);
        }
        cstr_destroy(&b2);
    }

    SECTION("SSO string with long string")
    {
        const std::string a = "Hello";
        const std::string b = "This is a very long string that won't fit in SSO";
        cstr b2 = cstr_make(b.c_str(), b.size());
        for (std::size_t pos = 0; pos <= a.size(); ++pos) {
            INFO("a = \"" << a << "\" pos=" << pos);
            std::string expect = a;
            cstr        actual = cstr_make(a.c_str(), a.size());

            CHECK(cstr_isinline_(&actual));
            CHECK(!cstr_isinline_(&b2));

            expect.insert(pos, b);
            cstr* r = cstr_insert(&actual, pos, &b2);
            REQUIRE(r != nullptr);

            CHECK(!cstr_isinline_(&actual));

            CHECK(cstr_len(&actual)  == expect.size());
            CHECK(to_string(&actual) == expect);

            cstr_destroy(&actual);
        }
        cstr_destroy(&b2);
    }

    SECTION("Long string with long string")
    {
        const std::string a = "Hello, this is also a very long string !!!!!";
        const std::string b = "This is a very long string that won't fit in SSO";
        cstr b2 = cstr_make(b.c_str(), b.size());
        for (std::size_t pos = 0; pos <= a.size(); ++pos) {
            INFO("a = \"" << a << "\" pos=" << pos);
            std::string expect = a;
            cstr        actual = cstr_make(a.c_str(), a.size());

            CHECK(!cstr_isinline_(&actual));
            CHECK(!cstr_isinline_(&b2));

            expect.insert(pos, b);
            cstr* r = cstr_insert(&actual, pos, &b2);
            REQUIRE(r != nullptr);

            CHECK(!cstr_isinline_(&actual));

            CHECK(cstr_len(&actual)  == expect.size());
            CHECK(to_string(&actual) == expect);

            cstr_destroy(&actual);
        }
        cstr_destroy(&b2);
    }
}

TEST_CASE("Allocator fallbacks")
{
    auto* my_free = +[](void* p, size_t) -> void
    {
        free(p);
    };

    SECTION("No calloc")
    {
        cstr_set_allocator(cstr_alloc_t{ NULL, &reallocarray, my_free });

        const std::string a = "Hello, this is also a very long string !!!!!";
        const std::string b = "This is a very long string that won't fit in SSO";
        cstr b2 = cstr_make(b.c_str(), b.size());
        for (std::size_t pos = 0; pos <= a.size(); ++pos) {
            INFO("a = \"" << a << "\" pos=" << pos);
            std::string expect = a;
            cstr        actual = cstr_make(a.c_str(), a.size());

            CHECK(!cstr_isinline_(&actual));
            CHECK(!cstr_isinline_(&b2));

            expect.insert(pos, b);
            cstr* r = cstr_insert(&actual, pos, &b2);
            REQUIRE(r != nullptr);

            CHECK(!cstr_isinline_(&actual));

            CHECK(cstr_len(&actual)  == expect.size());
            CHECK(to_string(&actual) == expect);

            cstr_destroy(&actual);
        }
        cstr_destroy(&b2);

        cstr_reset_allocator_to_default_();
    }

    SECTION("No reallocarray")
    {
        cstr_set_allocator(cstr_alloc_t{ &calloc, NULL, my_free });

        const std::string a = "Hello, this is also a very long string !!!!!";
        const std::string b = "This is a very long string that won't fit in SSO";
        cstr b2 = cstr_make(b.c_str(), b.size());
        for (std::size_t pos = 0; pos <= a.size(); ++pos) {
            INFO("a = \"" << a << "\" pos=" << pos);
            std::string expect = a;
            cstr        actual = cstr_make(a.c_str(), a.size());

            CHECK(!cstr_isinline_(&actual));
            CHECK(!cstr_isinline_(&b2));

            expect.insert(pos, b);
            cstr* r = cstr_insert(&actual, pos, &b2);
            REQUIRE(r != nullptr);

            CHECK(!cstr_isinline_(&actual));

            CHECK(cstr_len(&actual)  == expect.size());
            CHECK(to_string(&actual) == expect);

            cstr_destroy(&actual);
        }
        cstr_destroy(&b2);

        cstr_reset_allocator_to_default_();
    }
}

TEST_CASE("Substr")
{
    SECTION("SSO")
    {
        const std::string a = "Hello, World";
        cstr a2 = cstr_make(a.c_str(), a.size());
        for (std::size_t pos = 0; pos <= a.size(); ++pos) {
            for (std::size_t len = 0; len <= a.size(); ++len) {
                cstrview v = cstr_substr(&a2, pos, len);
                auto v2 = a.substr(pos, len);
                CHECK(cstrview_len(v) == v2.size());
                CHECK(to_string(v)    == v2);
            }
        }
        cstr_destroy(&a2);
    }

    SECTION("Long string")
    {
        const std::string a = "Hello, World -- this is a long string that won't be SSO";
        cstr a2 = cstr_make(a.c_str(), a.size());
        for (std::size_t pos = 0; pos <= a.size(); ++pos) {
            for (std::size_t len = 0; len <= a.size(); ++len) {
                cstrview v = cstr_substr(&a2, pos, len);
                auto v2 = a.substr(pos, len);
                CHECK(cstrview_len(v) == v2.size());
                CHECK(to_string(v)    == v2);
            }
        }
        cstr_destroy(&a2);
    }
}
