#include <catch2/catch.hpp>
#include "plsearch.h"
#include <iostream>


TEST_CASE("Empty text")
{
    const std::string text = "";
    const std::vector<std::string> pats = {
        "learning",
        "lemming",
        "",
        "AnOther Pattern",
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

    auto test_searcher = [&](auto make_searcher)
    {
        for (auto&& pat : pats) {
            auto s1 = make_searcher(pat);
            auto s2 = std::default_searcher{pat.cbegin(), pat.cend()};
            auto actual = s1(text.cbegin(), text.cend());
            auto expect = s2(text.cbegin(), text.cend());
            INFO("Searching for pattern: \"" << pat << "\" in empty text");
            CHECK(actual.first  == expect.first);
            CHECK(actual.second == expect.second);
        }
    };

    SECTION("NaiveSearch")
    {
        test_searcher([](const std::string& pat) {
            return pl::NaiveSearch{pat.cbegin(), pat.cend()};
        });
    }

    SECTION("RabinKarp")
    {
        test_searcher([](const std::string& pat) {
            return pl::RabinKarp{pat.cbegin(), pat.cend()};
        });
    }

    SECTION("FMA")
    {
        test_searcher([](const std::string& pat) {
            return pl::FiniteAutomata{pat.cbegin(), pat.cend()};
        });
    }

    SECTION("KMP")
    {
        test_searcher([](const std::string& pat) {
            return pl::KMPMatcher{pat.cbegin(), pat.cend()};
        });
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

    auto test_searcher = [&](auto make_searcher)
    {
        for (auto&& pat : pats) {
            auto s1 = make_searcher(pat);
            auto s2 = std::default_searcher{pat.cbegin(), pat.cend()};
            auto actual = s1(text.cbegin(), text.cend());
            auto expect = s2(text.cbegin(), text.cend());
            INFO("Searching for pattern: \"" << pat << "\" in text \"" << text << "\"");
            CHECK(actual.first  == expect.first);
            INFO("Distance " << std::distance(actual.second, expect.second));
            CHECK(actual.second == expect.second);
        }
    };

    SECTION("NaiveSearch")
    {
        test_searcher([](const std::string& pat) {
            return pl::NaiveSearch{pat.cbegin(), pat.cend()};
        });
    }

    SECTION("RabinKarp")
    {
        test_searcher([](const std::string& pat) {
            return pl::RabinKarp{pat.cbegin(), pat.cend()};
        });
    }

    SECTION("FMA")
    {
        test_searcher([](const std::string& pat) {
            return pl::FiniteAutomata{pat.cbegin(), pat.cend()};
        });
    }

    SECTION("KMP")
    {
        test_searcher([](const std::string& pat) {
            return pl::KMPMatcher{pat.cbegin(), pat.cend()};
        });
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
    auto test_searcher = [&](auto make_searcher)
    {
        for (auto&& pat : needles) {
            for (auto&& text : haystacks) {
                auto s1 = make_searcher(pat);
                auto s2 = std::default_searcher{pat.cbegin(), pat.cend()};
                auto actual = s1(text.cbegin(), text.cend());
                auto expect = s2(text.cbegin(), text.cend());
                INFO("Searching for pattern: \"" << pat << "\" in empty text");
                CHECK(actual.first  == expect.first);
                CHECK(actual.second == expect.second);
            }
        }
    };

    SECTION("NaiveSearch")
    {
        test_searcher([](const std::string& pat) {
            return pl::NaiveSearch{pat.cbegin(), pat.cend()};
        });
    }

    SECTION("RabinKarp")
    {
        test_searcher([](const std::string& pat) {
            return pl::RabinKarp{pat.cbegin(), pat.cend()};
        });
    }

    SECTION("FMA")
    {
        test_searcher([](const std::string& pat) {
            return pl::FiniteAutomata{pat.cbegin(), pat.cend()};
        });
    }

    SECTION("KMP")
    {
        test_searcher([](const std::string& pat) {
            return pl::KMPMatcher{pat.cbegin(), pat.cend()};
        });
    }
}

TEST_CASE("All cases")
{
    const std::string text = "why aabaa waste ttttime learning, when ignorance is instanceous?";
    auto len = text.size();

    auto test_searcher = [&](auto make_searcher)
    {
        for (auto i = 0u; i != len; ++i) {
            for (auto j = i+1; j != len; ++j) {
                auto cnt = j - i;
                auto pat = text.substr(i, cnt);
                auto s1 = make_searcher(pat);
                auto s2 = std::default_searcher{pat.cbegin(), pat.cend()};
                auto actual = s1(text.cbegin(), text.cend());
                auto expect = s2(text.cbegin(), text.cend());
                INFO("Searching for pattern: \"" << pat << "\"");
                CHECK(actual.first  == expect.first);
                CHECK(actual.second == expect.second);
            }
        }
    };

    SECTION("NaiveSearch")
    {
        test_searcher([](const std::string& pat) {
            return pl::NaiveSearch{pat.cbegin(), pat.cend()};
        });
    }

    SECTION("RabinKarp")
    {
        test_searcher([](const std::string& pat) {
            return pl::RabinKarp{pat.cbegin(), pat.cend()};
        });
    }

#if 0
    SECTION("FMA")
    {
        test_searcher([](const std::string& pat) {
            return pl::FiniteAutomata{pat.cbegin(), pat.cend()};
        });
    }
#endif

    SECTION("KMP")
    {
        test_searcher([](const std::string& pat) {
            return pl::KMPMatcher{pat.cbegin(), pat.cend()};
        });
    }
}

#if 0
TEST_CASE("KMP")
{
    // const std::string text = "ababaabcbab";
    const std::string text = "abababacab";
    const std::string pat =    "ababaca";
    pl::KMPMatcher s1{pat.cbegin(), pat.cend()};
    std::default_searcher s2{pat.cbegin(), pat.cend()};
    s1.dump(std::cout);
    auto actual = s1(text.cbegin(), text.cend());
    auto expect = s2(text.cbegin(), text.cend());

    if (actual.first != text.cend()) {
        std::cout << "First = " << std::distance(text.cbegin(), actual.first) << " (" << *actual.first  << ")\n";
    } else {
        std::cout << "Matcher didn't find the pattern\n";
    }

    CHECK(actual.first  == expect.first);
    CHECK(actual.second == expect.second);
}
#endif
