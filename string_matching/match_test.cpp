#include <catch2/catch.hpp>
#include "plsearch.h"
#include <iostream>


TEST_CASE("Empty text or patterns")
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
            auto actual = pl::search(text.begin(), text.end(),
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
            auto actual = pl::search(text.begin(), text.end(),
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
            auto actual = pl::search(text.begin(), text.end(),
                    pat.begin(), pat.end()) != text.end();
            auto expect = std::search(text.begin(), text.end(),
                    pat.begin(), pat.end()) != text.end();
            INFO("Searching for pattern: \"" << pat << "\" in empty text");
            CHECK(actual == expect);
        }
    }

    SECTION("NaiveSearch")
    {
        for (auto&& pat : pats) {
            pl::NaiveSearch searcher(pat.begin(), pat.end());
            auto actual = pl::search(text.begin(), text.end(),
                    searcher) != text.end();
            auto expect = std::search(text.begin(), text.end(),
                    pat.begin(), pat.end()) != text.end();
            INFO("Searching for pattern: \"" << pat << "\" in empty text");
            CHECK(actual == expect);
        }
    }

    SECTION("RabinKarp")
    {
        for (auto&& pat : pats) {
            pl::RabinKarp searcher(pat.begin(), pat.end());
            auto actual = pl::search(text.begin(), text.end(),
                    searcher) != text.end();
            auto expect = std::search(text.begin(), text.end(),
                    pat.begin(), pat.end()) != text.end();
            INFO("Searching for pattern: \"" << pat << "\" in empty text");
            CHECK(actual == expect);
        }
    }

    SECTION("FMA")
    {
        for (auto&& pat : pats) {
            pl::FiniteAutomata searcher{pat.begin(), pat.end()};
            auto actual = pl::search(text.begin(), text.end(),
                    searcher) != text.end();
            auto expect = std::search(text.begin(), text.end(),
                    pat.begin(), pat.end()) != text.end();
            INFO("Searching for pattern: \"" << pat << "\" in empty text");
            CHECK(actual == expect);
        }
    }
}

TEST_CASE("Not empty text")
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
            auto actual = pl::search(text.begin(), text.end(),
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
            auto actual = pl::search(text.begin(), text.end(),
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
            auto actual = pl::search(text.begin(), text.end(),
                    pat.begin(), pat.end()) != text.end();
            auto expect = std::search(text.begin(), text.end(),
                    pat.begin(), pat.end()) != text.end();
            INFO("Searching for pattern: \"" << pat << "\"");
            CHECK(actual == expect);
        }
    }

    SECTION("Naive Searcher")
    {
        for (auto&& pat : pats) {
            pl::NaiveSearch searcher(pat.begin(), pat.end());
            auto actual = pl::search(text.begin(), text.end(),
                    searcher) != text.end();
            auto expect = std::search(text.begin(), text.end(),
                    pat.begin(), pat.end()) != text.end();
            INFO("Searching for pattern: \"" << pat << "\"");
            CHECK(actual == expect);
        }
    }

    SECTION("Rabin Karp")
    {
        for (auto&& pat : pats) {
            pl::RabinKarp searcher(pat.begin(), pat.end());
            auto actual = pl::search(text.begin(), text.end(),
                    searcher) != text.end();
            auto expect = std::search(text.begin(), text.end(),
                    pat.begin(), pat.end()) != text.end();
            INFO("Searching for pattern: \"" << pat << "\"");
            CHECK(actual == expect);
        }
    }

    SECTION("Finite Automata")
    {
        for (auto&& pat : pats) {
            pl::FiniteAutomata searcher(pat.begin(), pat.end());
            auto actual = pl::search(text.begin(), text.end(),
                    searcher) != text.end();
            auto expect = std::search(text.begin(), text.end(),
                    pat.begin(), pat.end()) != text.end();
            INFO("Searching for pattern: \"" << pat << "\"");
            CHECK(actual == expect);
        }
    }
}

TEST_CASE("Different Texts")
{
    std::string s = "a a";
    s[1] = (char)-97;
    std::vector<std::string> needles = {
        s, "", "a", "aa", "aaa", "ab", "cd", "abcd", "abcdabcd", "abcabcd", "aaab",
    };
    std::vector<std::string> haystacks = {
        s, "", "a", "aa", "aaa", "ab", "cd", "abcd", "abcdabcd", "abcabcd",
        "aaaaaaa", "aabaa", "aaacab", "cdabcdab", "abcdabcd", "xyzabcdxyz",
        "aaaaab",
    };

    SECTION("Forward Iterator")
    {
        for (auto&& pat : needles) {
            for (auto&& text : haystacks) {
                auto actual = pl::search(text.begin(), text.end(),
                        pat.begin(), pat.end(),
                        std::forward_iterator_tag{}) != text.end();
                auto expect = std::search(text.begin(), text.end(),
                        pat.begin(), pat.end()) != text.end();
                INFO("Searching for pattern: \"" << pat << "\"");
                CHECK(actual == expect);
            }
        }
    }

    SECTION("Random Access Iterator")
    {
        for (auto&& pat : needles) {
            for (auto&& text : haystacks) {
                auto actual = pl::search(text.begin(), text.end(),
                        pat.begin(), pat.end()) != text.end();
                auto expect = std::search(text.begin(), text.end(),
                        pat.begin(), pat.end()) != text.end();
                INFO("Searching for pattern: \"" << pat << "\"");
                CHECK(actual == expect);
            }
        }
    }

    SECTION("Tag Dispatch")
    {
        for (auto&& pat : needles) {
            for (auto&& text : haystacks) {
                auto actual = pl::search(text.begin(), text.end(),
                        pat.begin(), pat.end()) != text.end();
                auto expect = std::search(text.begin(), text.end(),
                        pat.begin(), pat.end()) != text.end();
                INFO("Searching for pattern: \"" << pat << "\"");
                CHECK(actual == expect);
            }
        }
    }

    SECTION("Naive Searcher")
    {
        for (auto&& pat : needles) {
            for (auto&& text : haystacks) {
                pl::NaiveSearch searcher(pat.begin(), pat.end());
                auto actual = pl::search(text.begin(), text.end(),
                        searcher) != text.end();
                auto expect = std::search(text.begin(), text.end(),
                        pat.begin(), pat.end()) != text.end();
                INFO("Searching for pattern: \"" << pat << "\"");
                CHECK(actual == expect);
            }
        }
    }

    SECTION("RabinKarp")
    {
        for (auto&& pat : needles) {
            for (auto&& text : haystacks) {
                pl::RabinKarp searcher(pat.begin(), pat.end());
                auto actual = pl::search(text.begin(), text.end(),
                        searcher) != text.end();
                auto expect = std::search(text.begin(), text.end(),
                        pat.begin(), pat.end()) != text.end();
                INFO("Searching for pattern: \"" << pat << "\"");
                CHECK(actual == expect);
            }
        }
    }

    SECTION("FiniteAutomata")
    {
        for (auto&& pat : needles) {
            for (auto&& text : haystacks) {
                pl::FiniteAutomata searcher(pat.begin(), pat.end());
                auto actual = pl::search(text.begin(), text.end(),
                        searcher) != text.end();
                auto expect = std::search(text.begin(), text.end(),
                        pat.begin(), pat.end()) != text.end();
                INFO("Searching for pattern: \"" << pat << "\"");
                CHECK(actual == expect);
            }
        }
    }
}

TEST_CASE("All cases")
{
    const std::string text = "why aabaa waste ttttime learning, when ignorance is instanceous?";
    auto len = text.size();

    SECTION("Naive Searcher")
    {
        for (auto i = 0u; i != len; ++i) {
            for (auto j = i+1; j != len; ++j) {
                auto cnt = j - i;
                auto pat = text.substr(i, cnt);
                pl::NaiveSearch searcher(pat.cbegin(), pat.cend());
                auto actual = pl::search(text.cbegin(), text.cend(), searcher);
                auto expect = std::search(text.cbegin(), text.cend(),
                        pat.cbegin(), pat.cend());
                INFO("Searching for pattern: \"" << pat << "\"");
                CHECK(actual == expect);
            }
        }
    }

    SECTION("RabinKarp")
    {
        for (auto i = 0u; i != len; ++i) {
            for (auto j = i+1; j != len; ++j) {
                auto cnt = j - i;
                auto pat = text.substr(i, cnt);
                pl::RabinKarp searcher(pat.cbegin(), pat.cend());
                auto actual = pl::search(text.cbegin(), text.cend(), searcher);
                auto expect = std::search(text.cbegin(), text.cend(),
                        pat.cbegin(), pat.cend());
                INFO("Searching for pattern: \"" << pat << "\"");
                CHECK(actual == expect);
            }
        }
    }

#if 0
    SECTION("FiniteAutomata")
    {
        for (auto i = 0u; i != len; ++i) {
            for (auto j = i+1; j != len; ++j) {
                auto cnt = j - i;
                auto pat = text.substr(i, cnt);
                pl::FiniteAutomata searcher(pat.cbegin(), pat.cend());
                auto actual = pl::search(text.cbegin(), text.cend(), searcher);
                auto expect = std::search(text.cbegin(), text.cend(),
                        pat.cbegin(), pat.cend());
                INFO("Searching for pattern: \"" << pat << "\" in text \""
                                                 << text << "\"");
                REQUIRE(actual == expect);
            }
        }
    }
#endif
}
