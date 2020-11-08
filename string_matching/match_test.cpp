#include <catch2/catch.hpp>
#include "naive.h"

TEST_CASE("Naive -- Empty text")
{
    const std::string text = "";
    const std::vector<std::string> pats = {
        "learning",
        "lemming",
        "",
    };

    SECTION("Forward Iterator")
    {
        for (auto&& pat : pats) {
            auto actual = naive::search(text.begin(), text.end(),
                    pat.begin(), pat.end(),
                    std::forward_iterator_tag{}) != text.end();
            auto expect = std::search(text.begin(), text.end(),
                    pat.begin(), pat.end()) != text.end();
            INFO("Searching for pattern: \"" << pat << "\" in empty text");
            CHECK(actual == expect);
        }
    }

    SECTION("Random Access Iterator")
    {
        for (auto&& pat : pats) {
            auto actual = naive::search(text.begin(), text.end(),
                    pat.begin(), pat.end(),
                    std::random_access_iterator_tag{}) != text.end();
            auto expect = std::search(text.begin(), text.end(),
                    pat.begin(), pat.end()) != text.end();
            INFO("Searching for pattern: \"" << pat << "\" in empty text");
            CHECK(actual == expect);
        }
    }

    SECTION("Tag Dispatch")
    {
        for (auto&& pat : pats) {
            auto actual = naive::search(text.begin(), text.end(),
                    pat.begin(), pat.end()) != text.end();
            auto expect = std::search(text.begin(), text.end(),
                    pat.begin(), pat.end()) != text.end();
            INFO("Searching for pattern: \"" << pat << "\" in empty text");
            CHECK(actual == expect);
        }
    }
}

TEST_CASE("Naive -- Not empty text")
{
    const std::string text =
        "why waste time learning, when ignorance is instantaneous?";

    const std::vector<std::string> pats = {
        "learning",
        "lemming",
        "learning, when",
        "why waste",
        "time",
        "instantaneous?",
        "instantaneous?ly",
        "asdf?",
        "asdf?asdf;lkajsdfl;kjasdf",
        "why waste time learning, when ignorance is instantaneous?",
        "why waste time learning, when ignorance is instantaneous? With extra at end",
        "why why",
        "why a",
        "",
    };

    SECTION("Forward Iterator")
    {
        for (auto&& pat : pats) {
            auto actual = naive::search(text.begin(), text.end(),
                    pat.begin(), pat.end(),
                    std::forward_iterator_tag{}) != text.end();
            auto expect = std::search(text.begin(), text.end(),
                    pat.begin(), pat.end()) != text.end();
            INFO("Searching for pattern: \"" << pat << "\"");
            CHECK(actual == expect);
        }
    }

    SECTION("Random Access Iterator")
    {
        for (auto&& pat : pats) {
            auto actual = naive::search(text.begin(), text.end(),
                    pat.begin(), pat.end(),
                    std::random_access_iterator_tag{}) != text.end();
            auto expect = std::search(text.begin(), text.end(),
                    pat.begin(), pat.end()) != text.end();
            INFO("Searching for pattern: \"" << pat << "\"");
            CHECK(actual == expect);
        }
    }

    SECTION("Tag Dispatch")
    {
        for (auto&& pat : pats) {
            auto actual = naive::search(text.begin(), text.end(),
                    pat.begin(), pat.end()) != text.end();
            auto expect = std::search(text.begin(), text.end(),
                    pat.begin(), pat.end()) != text.end();
            INFO("Searching for pattern: \"" << pat << "\"");
            CHECK(actual == expect);
        }
    }
}
