#include <catch2/catch.hpp>
#include "plsearch.h"
#include <iostream>


TEST_CASE("String Matching")
{
    std::string s = "a a";
    s[1] = (char)-97;

    std::vector<std::string> patterns = {
        "",
        "abab",
        "ababab",
        "abcabc",
        "aaaaaaaa",
        "\00111=156aasdf78\001",
        "ZZZZZ",
        "pizza",
        "This is a long pattern",
        "ababaca",
        s,
        "a",
        "aa",
        "aaa",
        "ab",
        "cd",
        "abcd",
        "abcdabcd",
        "abcabcd",
        "aaab",
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
        "learning",
        "lemming",
        "AnOther Pattern",
    };

    std::vector<std::string> texts = {
        "",
        "ababababababa",
        "bbbbbbbbbabababbbb",
        "abababacab",
        s,
        std::string{"a String within"} + s + " another string",
        "a",
        "aa",
        "aaa",
        "ab",
        "cd",
        "abcd",
        "abcdabcd",
        "abcabcd",
        "aaaaaaa",
        "aabaa",
        "aaacab",
        "cdabcdab",
        "abcdabcd",
        "xyzabcdxyz",
        "aaaaab",
        "why waste time learning, when ignorance is instantaneous?",
        "learning",
        "lemming",
        "AnOther Pattern",
        "\00135=A\00134=29\00137=12341234\00111=156aasdf78\00110=234",
        "bbbbbbaaaaaaaaaaaaaaaaaaaaaaaaaaaaabbbbbb",
        "ababababasdfl;kasjdfpo8iuzxcvlkj",
        "asl;asl;jklasdfaaaaaaaaaaaaaaaaaaaaasdfl;kasdfl",
        "ABDFASEUazsdflkkjZZZZZZASDf;oiju",
    };

    for (auto&& pat : patterns) {
        auto s0 = std::default_searcher{pat.cbegin(), pat.cend()};
        auto s1 = pl::NaiveSearch{pat.cbegin(), pat.cend()};
        auto s2 = pl::RabinKarp{pat.cbegin(), pat.cend()};
        auto s3 = pl::FiniteAutomata{pat.cbegin(), pat.cend()};
        auto s4 = pl::KMPMatcher{pat.cbegin(), pat.cend()};
        for (auto&& text : texts) {
            auto expect1 = s0(text.cbegin(), text.cend());
            auto actual1 = s1(text.cbegin(), text.cend());
            auto actual2 = s2(text.cbegin(), text.cend());
            auto actual3 = s3(text.cbegin(), text.cend());
            auto actual4 = s4(text.cbegin(), text.cend());

            CHECK(expect1.first  == actual1.first);
            CHECK(expect1.first  == actual2.first);
            CHECK(expect1.first  == actual3.first);
            CHECK(expect1.first  == actual4.first);
            CHECK(expect1.second == actual1.second);
            CHECK(expect1.second == actual2.second);
            CHECK(expect1.second == actual3.second);
            CHECK(expect1.second == actual4.second);

            auto expect2 = std::search(text.begin(), text.end(),
                    pat.begin(), pat.end());
            auto actual5 = pl::search(text.begin(), text.end(), pat.begin(), pat.end(),
                    std::forward_iterator_tag{});
            auto actual6 = pl::search(text.begin(), text.end(), pat.begin(), pat.end(),
                    std::bidirectional_iterator_tag{});
            auto actual7 = pl::search(text.begin(), text.end(), pat.begin(), pat.end(),
                    std::random_access_iterator_tag{});
            auto actual8 = pl::search(text.begin(), text.end(), pat.begin(), pat.end());

            CHECK(expect2 == actual5);
            CHECK(expect2 == actual6);
            CHECK(expect2 == actual7);
            CHECK(expect2 == actual8);
        }
    }
}
