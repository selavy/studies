#include <catch2/catch.hpp>
#include "naive.h"

TEST_CASE("Naive string matching")
{
    std::string text = "why waste time learning, when ignorance is instantaneous?";

    std::vector<std::string> pats = {
        "why waste",
        "time",
        "instantaneous?",
        "instantaneous?ly",
        "asdf?",
        "asdf?asdf;lkajsdfl;kjasdf",
        "why waste time learning, when ignorance is instantaneous?",
        "why waste time learning, when ignorance is instantaneous? With extra at end",
    };

    for (auto&& pat : pats) {
        bool actual = naive::search(text.begin(), text.end(), pat.begin(), pat.end()) != text.end();
        bool expect = std::search(text.begin(), text.end(), pat.begin(), pat.end()) != text.end();
        INFO("Searching for pattern: \"" << pat << "\"");
        CHECK(actual == expect);
    }
}
