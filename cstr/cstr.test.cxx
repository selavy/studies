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
        "9998",
        "99998",
        "0123456789",
    };

    auto normalize = [](int r) -> int
    {
        if (r < 0) {
            return -1;
        } else if (r == 0) {
            return 0;
        } else {
            return 1;
        }
    };

    for (auto val1 : vals) {
        for (auto val2 : vals) {
            INFO("Comparing \"" << val1 << "\" vs \"" << val2 << '"');
            cstr* s1 = cstr_new(val1.c_str(), val1.size());
            cstr* s2 = cstr_new(val2.c_str(), val2.size());

            cstrview v1 = cstr_view(s1);
            cstrview v2 = cstr_view(s2);

            CHECK(cstr_len(s1) == val1.size());
            CHECK(cstr_len(s2) == val2.size());
            CHECK(cstrview_len(v1) == val1.size());
            CHECK(cstrview_len(v2) == val2.size());
            CHECK(to_string(s1) == val1);
            CHECK(to_string(s2) == val2);
            CHECK(to_string(v1) == val1);
            CHECK(to_string(v2) == val2);

            SECTION("compare")
            {
                int expect = normalize(val1.compare(val2));
                int actual = normalize(cstr_cmp(s1, s2));
                CHECK(actual == expect);

                int actualv = normalize(cstrview_cmp(v1, v2));
                CHECK(actualv == expect);
            }

            SECTION("equals")
            {
                bool expect = val1 == val2;
                bool result = !!cstr_eq(s1, s2);
                CHECK(result == expect);

                int actualv = !!cstrview_eq(v1, v2);
                CHECK(actualv == expect);
            }
            SECTION("not equals")
            {
                bool expect = val1 != val2;
                bool result = !!cstr_neq(s1, s2);
                CHECK(result == expect);

                int actualv = !!cstrview_neq(v1, v2);
                CHECK(actualv == expect);
            }
            SECTION("less than")
            {
                bool expect = val1 < val2;
                bool result = !!cstr_lt(s1, s2);
                CHECK(result == expect);

                int actualv = !!cstrview_lt(v1, v2);
                CHECK(actualv == expect);
            }
            SECTION("less than or equals")
            {
                bool expect = val1 <= val2;
                bool result = !!cstr_lte(s1, s2);
                CHECK(result == expect);

                int actualv = !!cstrview_lte(v1, v2);
                CHECK(actualv == expect);
            }
            SECTION("greater than")
            {
                bool expect = val1 > val2;
                bool result = !!cstr_gt(s1, s2);
                CHECK(result == expect);

                int actualv = !!cstrview_gt(v1, v2);
                CHECK(actualv == expect);
            }
            SECTION("greater than or equals")
            {
                bool expect = val1 >= val2;
                bool result = !!cstr_gte(s1, s2);
                CHECK(result == expect);

                int actualv = !!cstrview_gte(v1, v2);
                CHECK(actualv == expect);
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

    // NOTE: no longer offering this functionality
#if 0
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
#endif
}

TEST_CASE("Substr")
{
    SECTION("SSO")
    {
        const std::string a = "Hello, World";
        cstr a2 = cstr_make(a.c_str(), a.size());
        cstrview a3 = cstr_view(&a2);
        CHECK(cstr_len(&a2)    == a.size());
        CHECK(cstrview_len(a3) == a.size());
        REQUIRE(to_string(&a2) == a);
        REQUIRE(to_string(a3)  == a);
        for (std::size_t pos = 0; pos <= a.size(); ++pos) {
            for (std::size_t len = 0; len <= a.size() + 5; ++len) {
                cstrview v = cstr_substr(&a2, pos, len);
                auto v2 = a.substr(pos, len);
                CHECK(cstrview_len(v) == v2.size());
                CHECK(to_string(v)    == v2);

                cstrview r = cstrview_substr(a3, pos, len);
                CHECK(cstrview_len(r) == v2.size());
                CHECK(to_string(r) == v2);
            }
        }
        cstr_destroy(&a2);
    }

    SECTION("Long string")
    {
        const std::string a = "Hello, World -- this is a long string that won't be SSO";
        cstr a2 = cstr_make(a.c_str(), a.size());
        cstrview a3 = cstr_view(&a2);
        CHECK(cstr_len(&a2)    == a.size());
        CHECK(cstrview_len(a3) == a.size());
        REQUIRE(to_string(&a2) == a);
        REQUIRE(to_string(a3)  == a);
        for (std::size_t pos = 0; pos <= a.size(); ++pos) {
            for (std::size_t len = 0; len <= a.size() + 5; ++len) {
                cstrview v = cstr_substr(&a2, pos, len);
                auto v2 = a.substr(pos, len);
                CHECK(cstrview_len(v) == v2.size());
                CHECK(to_string(v)    == v2);

                cstrview r = cstrview_substr(a3, pos, len);
                CHECK(cstrview_len(r) == v2.size());
                CHECK(to_string(r) == v2);
            }
        }
        cstr_destroy(&a2);
    }
}

TEST_CASE("Startswith")
{
    std::vector<std::tuple<std::string, std::string, bool>> cases = {
        { "Hello, World", "Hello", true   },
        { "Hello, World", "HelloX", false },
        { "Hello, World", "Hello, World", true },
        { "Hello, World", "Hello, Worlj", false },
        { "Hello, World", "Hello, World2", false },
        { "Hello, World", "Hello,XWorld", false },
        { "Hello, World", "", true },
        { "Hello, World", "H", true },
        { "Hello, World", "He", true },
        { "Hello, World", "Hel", true },
        { "Hello, World", "Hello", true },
        { "Hello, World", "Hello, World Way Too Long String", false },
    };

    for (auto&& [val, prefix, expect] : cases) {
        INFO("value=\"" << val << "\" prefix=\"" << prefix << "\"");
        const cstrview v = cstrview_init(val.c_str(), val.size());
        CHECK(cstrview_len(v) == val.size());
        const cstrview p = cstrview_init(prefix.c_str(), prefix.size());
        CHECK(cstrview_len(p) == prefix.size());
        int actual = cstrview_startswith(v, p);
        CHECK(actual == expect);

        cstr s = cstrview_tostr(v);
        cstr p2 = cstrview_tostr(p);
        CHECK(cstr_len(&s) == cstrview_len(v));
        int actual2 = cstr_startswithv(&s, p);
        CHECK(actual2 == expect);
        int actual3 = cstr_startswith(&s, &p2);
        CHECK(actual3 == expect);

        cstr_destroy(&s);
        cstr_destroy(&p2);
    }
}

TEST_CASE("Endswith")
{
    std::vector<std::tuple<std::string, std::string, bool>> cases = {
        { "Hello, World", "World", true },
        { "Hello, World", "orld", true },
        { "Hello, World", "rld", true },
        { "Hello, World", "ld", true },
        { "Hello, World", "d", true },
        { "Hello, World", "", true },
        { "Hello, World", "Hello, World", true },
        { "Hello, World", "Hello, World2", false },
        { "Hello, World", "Hello, World Way too Long String", false },
        { "Hello, World", "Hello, ", false },
        { "Hello, World", "Hello, Worl", false },
        { "file.txt", ".txt", true },
    };

    for (auto&& [val, postfix, expect] : cases) {
        INFO("value=\"" << val << "\" postfix=\"" << postfix << "\"");
        const cstrview v = cstrview_init(val.c_str(), val.size());
        CHECK(cstrview_len(v) == val.size());
        const cstrview p = cstrview_init(postfix.c_str(), postfix.size());
        CHECK(cstrview_len(p) == postfix.size());
        int actual = cstrview_endswith(v, p);
        CHECK(actual == expect);

        cstr s = cstrview_tostr(v);
        cstr p2 = cstrview_tostr(p);
        CHECK(cstr_len(&s) == cstrview_len(v));
        int actual2 = cstr_endswithv(&s, p);
        CHECK(actual2 == expect);
        int actual3 = cstr_endswith(&s, &p2);
        CHECK(actual3 == expect);

        cstr_destroy(&s);
        cstr_destroy(&p2);
    }
}

TEST_CASE("Split API")
{
    const std::string value = "a,b,c,d,e,ff,ggg,hhhh,iiii,,,,Last";
    const std::vector<std::string> expects = {
        "a",
        "b",
        "c",
        "d",
        "e",
        "ff",
        "ggg",
        "hhhh",
        "iiii",
        "",
        "",
        "",
        "Last"
    };

    cstrview v = cstrview_init(value.c_str(), value.size());
    size_t i = 0;
    for (
        cstrview_split_iter it = cstrview_split_start(v, ',');
        !cstrview_split_stop(it);
        it = cstrview_split_next(it, ',')
    ) {
        REQUIRE(i < expects.size());
        const cstrview value = cstrview_split_deref(it);
        const auto& expect = expects[i];
        INFO("i = " << i << " expecting \"" << expect << "\"");
        CHECK(cstrview_len(value) == expect.size());
        CHECK(to_string(value)    == expect);
        ++i;
    }
    CHECK(i == expects.size());
}

TEST_CASE("Split")
{
    const std::string value = "a,b,c,d,e,ff,ggg,hhhh,iiii,,Last";

    cstrview v = cstrview_init(value.c_str(), value.size());
    CHECK(cstrview_len(v) == value.size());
    CHECK(to_string(v) == value);

    cstrview v1 = cstrview_split(v, ',');
    CHECK(to_string(v1) == "a");

    v = cstrview_drop(v, cstrview_len(v1) + 1);
    cstrview v2 = cstrview_split(v, ',');
    CHECK(to_string(v2) == "b");

    v = cstrview_drop(v, cstrview_len(v2) + 1);
    cstrview v3 = cstrview_split(v, ',');
    CHECK(to_string(v3) == "c");

    v = cstrview_drop(v, cstrview_len(v3) + 1);
    cstrview v4 = cstrview_split(v, ',');
    CHECK(to_string(v4) == "d");

    v = cstrview_drop(v, cstrview_len(v4) + 1);
    cstrview v5 = cstrview_split(v, ',');
    CHECK(to_string(v5) == "e");

    v = cstrview_drop(v, cstrview_len(v5) + 1);
    cstrview v6 = cstrview_split(v, ',');
    CHECK(to_string(v6) == "ff");

    v = cstrview_drop(v, cstrview_len(v6) + 1);
    cstrview v7 = cstrview_split(v, ',');
    CHECK(to_string(v7) == "ggg");

    v = cstrview_drop(v, cstrview_len(v7) + 1);
    cstrview v8 = cstrview_split(v, ',');
    CHECK(to_string(v8) == "hhhh");

    v = cstrview_drop(v, cstrview_len(v8) + 1);
    cstrview v9 = cstrview_split(v, ',');
    CHECK(to_string(v9) == "iiii");

    v = cstrview_drop(v, cstrview_len(v9) + 1);
    cstrview v10 = cstrview_split(v, ',');
    CHECK(to_string(v10) == "");

    v = cstrview_drop(v, cstrview_len(v10) + 1);
    cstrview v11 = cstrview_split(v, ',');
    CHECK(to_string(v11) == "Last");

    v = cstrview_drop(v, cstrview_len(v11) + 1);
    CHECK(cstrview_empty(v));
}
