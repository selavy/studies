#include <catch2/catch.hpp>
#include "darray.h"
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
}

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
}
