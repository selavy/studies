#include <catch2/catch.hpp>
#include <vector>
#include <string>
#include <unordered_map>
#include <set>
#include "dawg.h"

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
};
// clang-format on

TEST_CASE("Verify word lists")
{
    auto dict = std::set<std::string>{};
    for (const auto& word : DICT) {
        dict.insert(word);
    }

    for (const auto& word : MISSING) {
        REQUIRE(dict.count(word) == 0u);
    }
}

TEST_CASE("Datrie2")
{
    Datrie2 dt;

    {
        bool ok = init2(&dt);
        REQUIRE(ok == true);
    }

    {
        const auto& word = DICT[0];
        INFO("Inserting word: " << word);

        bool ok = insert2(&dt, word.c_str());
        REQUIRE(ok == true);
        bool found = isword2(&dt, word.c_str());
        CHECK(found == true);
    }

    for (std::size_t i = 1; i < 5; ++i)
    {
        const auto& word = DICT[i];
        INFO("Inserting word: " << word);

        bool ok = insert2(&dt, word.c_str());
        REQUIRE(ok == true);
        bool found = isword2(&dt, word.c_str());
        CHECK(found == true);
    }

    if (1)
    {
        const auto& word = DICT[1];
        INFO("Inserting word: " << word);
        bool ok = insert2(&dt, word.c_str());
        REQUIRE(ok == true);
        bool found = isword2(&dt, word.c_str());
        CHECK(found == true);
    }

    for (std::size_t i = 0; i < MISSING.size(); ++i)
    {
        const auto& word = MISSING[i];
        INFO("Looking for missing word: " << word);
        bool found = isword2(&dt, word.c_str());
        CHECK(found == false);
    }

#if 0
    for (const auto& word : DICT) {
        bool ok = insert2(&t, word.c_str());
        REQUIRE(ok == true);
    }
    for (const auto& word : DICT) {
        CHECK(isword2(&t, word.c_str()) == true);
    }
    for (const auto& word : MISSING) {
        CHECK(isword2(&t, word.c_str()) == false);
    }
#endif
}
