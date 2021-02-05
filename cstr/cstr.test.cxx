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
