#include <catch2/catch.hpp>
#include <iostream>
#include <vector>
#include <unordered_set>
#include "darray.h"
#include "darray2.h"
#include "darray3.h"
#include "tarray.h"
#include "tarraysep.h"
#include "mafsa.h"

// clang-format off
const std::vector<std::string> DICT = {
    "AA",
    "AAH",
    "AAHED",
    "AAHING",
    "AAHS",
    "AAL",
    "AALII",
    "AALIIS",
    "AALS",
    "AARDVARK",
    "AARDVARKS",
    "AARDWOLF",
    "AARDWOLVES",
    "CEANOTHUS",
    "CEANOTHUSES",
    "CEAS",
    "CEASE",
    "CEASED",
    "CEASEFIRE",
    "CEASEFIRES",
    "CEASELESS",
    "MIDWIVING",
    "MIDYEAR",
    "MIDYEARS",
    "MIELIE",
    "MIELIES",
    "GAUJES",
    "GAULEITER",
    "GAULEITERS",
    "THEATERLANDS",
    "THEATERS",
    "THEATRAL",
    "TYPEY",
    "TYPHACEOUS",
    "TYPHLITIC",
    "TYPHLITIS",
    "UPDATING",
    "UPDIVE",
    "UPDIVED",
    "VICTIMIZE",
    "VOICER",
    "VOICERS",
    "VOICES",
    "VOICING",
    "WILDCATTINGS",
    "WILDEBEEST",
    "WILDEBEESTS",
    "WILDED",
    "WOOFTAH",
    "WOOFTAHS",
    "WOOFTER",
    "YAMPY",
    "YAMS",
    "YAMULKA",
    "ZYMOSANS",
    "ZYMOSES",
    "ZYMOSIMETER",
    "ZYMOSIMETERS",
    "ZYMOSIS",
};
// clang-format on

const std::unordered_set<std::string> WORDSET{DICT.begin(), DICT.end()};

bool isword(const std::string& s) noexcept
{
    return WORDSET.count(s) != 0;
}

// clang-format off
const std::vector<std::string> MISSING = {
    "ABIDDEN",
    "ABIDE",
    "ABIDED",
    "ABIDER",
    "ACCOUSTREMENTS",
    "ACCOUTER",
    "ACCOUTERED",
    "FUTURISTIC",
    "FUTURISTICALLY",
    "FUTURISTICS",
    "XIPHOIDAL",
    "XIPHOIDS",
    "XIPHOPAGI",
    "ZZZ",
    "ZAZAZ",
    "AAAA",
    "AZZZZ",
    "YZY",

    "AALA",
    "AALB",
    "AALC",
    "AALD",
    "AALE",
    "AALF",
    "AALG",
    "AALH",
    "AALI",
    "AALJ",
    "AALK",
    "AALL",
    "AALM",
    "AALN",
    "AALO",
    "AALP",
    "AALQ",
    "AALR",
    "AALT",
    "AALU",
    "AALV",
    "AALW",
    "AALX",
    "AALY",
    "AALZ",

    "YAMA",
    "YAMB",
    "YAMC",
    "YAMD",
    "YAME",
    "YAMF",
    "YAMG",
    "YAMH",
    "YAMI",
    "YAMJ",
    "YAMK",
    "YAML",
    "YAMM",
    "YAMN",
    "YAMO",
    "YAMP",
    "YAMQ",
    "YAMR",
    "YAMT",
    "YAMU",
    "YAMV",
    "YAMW",
    "YAMZ",

    "MIDYA",
    "MIDYB",
    "MIDYC",
    "MIDYD",
    "MIDYF",
    "MIDYG",
    "MIDYH",
    "MIDYI",
    "MIDYJ",
    "MIDYK",
    "MIDYL",
    "MIDYM",
    "MIDYN",
    "MIDYO",
    "MIDYP",
    "MIDYQ",
    "MIDYR",
    "MIDYS",
    "MIDYT",
    "MIDYU",
    "MIDYV",
    "MIDYW",
    "MIDYX",
    "MIDYY",
    "MIDYZ",
};
// clang-format on

TEST_CASE("Darray")
{
    Darray d;
    for (const auto& word : DICT) {
        d.insert(word);
    }

    for (const auto& word : DICT) {
        CHECK(d.isword(word) == true);
    }

    for (const auto& word : MISSING) {
        CHECK(d.isword(word) == false);
    }

    for (const auto& word : DICT) {
        d.insert(word);
    }

    for (const auto& word : DICT) {
        CHECK(d.isword(word) == true);
    }

    for (const auto& word : MISSING) {
        CHECK(d.isword(word) == false);
    }


    for (const auto& word_ : DICT) {
        auto word = word_;

        // add letter to end of word
        for (char c = 'A'; c <= 'Z'; ++c) {
            word += c;
            CHECK(d.isword(word) == isword(word));
            word.pop_back();
        }

        // remove last letter of word
        word.pop_back();
        for (char c = 'A'; c <= 'Z'; ++c) {
            word += c;
            CHECK(d.isword(word) == isword(word));
            word.pop_back();
        }
    }

    SECTION("Trim works")
    {
        // std::cout << "size before: " << d.bases.size() << "\n";
        d.trim();
        // std::cout << "size after : " << d.bases.size() << "\n";

        for (const auto& word : DICT) {
            CHECK(d.isword(word) == true);
        }

        for (const auto& word : MISSING) {
            INFO("Checking missing word: " << word);
            CHECK(d.isword(word) == false);
        }
    }
}

TEST_CASE("Darray2")
{
    Darray2 d;
    for (const auto& word : DICT) {
        d.insert(word);
    }

    for (const auto& word : DICT) {
        INFO("Checking present word: " << word);
        CHECK(d.isword(word) == true);
    }

    for (const auto& word : MISSING) {
        INFO("Checking missing word: " << word);
        CHECK(d.isword(word) == false);
    }

    for (const auto& word : MISSING) {
        auto w = word;
        for (char c = 'A'; c <= 'Z'; ++c) {
            w += c;
            INFO("Checking " << w);
            CHECK(d.isword(w) == isword(w));
            w.pop_back();
        }
    }

    for (const auto& word : DICT) {
        d.insert(word);
    }

    for (const auto& word : DICT) {
        CHECK(d.isword(word) == true);
    }

    for (const auto& word : MISSING) {
        INFO("Checking missing word: " << word);
        CHECK(d.isword(word) == false);
    }

    SECTION("Trim works")
    {
        // std::cout << "size before: " << d.bases.size() << "\n";
        d.trim();
        // std::cout << "size after : " << d.bases.size() << "\n";

        for (const auto& word : DICT) {
            CHECK(d.isword(word) == true);
        }

        for (const auto& word : MISSING) {
            INFO("Checking missing word: " << word);
            CHECK(d.isword(word) == false);
        }
    }
}

#if 0
TEST_CASE("Darray3")
{
    Darray3 d;
    for (const auto& word : DICT) {
        d.insert(word);
    }

    for (const auto& word : DICT) {
        CHECK(d.isword(word) == true);
    }

    for (const auto& word : MISSING) {
        CHECK(d.isword(word) == false);
    }

    for (const auto& word : DICT) {
        d.insert(word);
    }

    for (const auto& word : DICT) {
        CHECK(d.isword(word) == true);
    }

    for (const auto& word : MISSING) {
        CHECK(d.isword(word) == false);
    }


    for (const auto& word_ : DICT) {
        auto word = word_;

        // add letter to end of word
        for (char c = 'A'; c <= 'Z'; ++c) {
            word += c;
            CHECK(d.isword(word) == isword(word));
            word.pop_back();
        }

        // remove last letter of word
        word.pop_back();
        for (char c = 'A'; c <= 'Z'; ++c) {
            word += c;
            CHECK(d.isword(word) == isword(word));
            word.pop_back();
        }
    }

    SECTION("Trim works")
    {
        // std::cout << "size before: " << d.bases.size() << "\n";
        d.trim();
        // std::cout << "size after : " << d.bases.size() << "\n";

        for (const auto& word : DICT) {
            CHECK(d.isword(word) == true);
        }

        for (const auto& word : MISSING) {
            INFO("Checking missing word: " << word);
            CHECK(d.isword(word) == false);
        }
    }
}
#endif

TEST_CASE("Mafsa")
{
    Mafsa d;
    for (const auto& word : DICT) {
        d.insert(word);
    }

    for (const auto& word : DICT) {
        CHECK(d.isword(word) == true);
    }

    for (const auto& word : MISSING) {
        CHECK(d.isword(word) == false);
    }

    for (const auto& word : DICT) {
        d.insert(word);
    }

    for (const auto& word : DICT) {
        CHECK(d.isword(word) == true);
    }

    for (const auto& word : MISSING) {
        CHECK(d.isword(word) == false);
    }
}

TEST_CASE("Tarray")
{
    Mafsa m;
    for (const auto& word : DICT) {
        m.insert(word);
    }

    const auto d = m.make_tarray();

    for (const auto& word : DICT) {
        CHECK(d.isword(word) == true);
    }

    for (const auto& word : MISSING) {
        CHECK(d.isword(word) == false);
    }

    const auto tarray = Tarray::make(d.bases.begin(), d.bases.end(), d.checks.begin(), d.checks.end(), d.nexts.begin(), d.nexts.end());
    for (const auto& word : DICT) {
        CHECK(tarray.isword(word) == true);
    }

    for (const auto& word : MISSING) {
        CHECK(tarray.isword(word) == false);
    }
}
