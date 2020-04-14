#include <catch2/catch.hpp>
#include <vector>
#include <string>
#include <unordered_map>
#include <set>
#include <iostream>
#include "dawg.h"

std::ostream& operator<<(std::ostream& os, Tristate t)
{
    os << tristate_to_str(t);
    return os;
}

#if 0
const std::vector<std::string> DICT = {
    "AB",
    "ABBB",
    // "AABBBC",
    "ABA",
};
#endif

#if 1
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
    // "AARDWOLVES",
    // "CEANOTHUS",
    // "CEANOTHUSES",
    // "CEAS",
    // "CEASE",
    // "CEASED",
    // "CEASEFIRE",
    // "CEASEFIRES",
    // "CEASELESS",
    // "MIDWIVING",
    // "MIDYEAR",
    // "MIDYEARS",
    // "MIELIE",
    // "MIELIES",
    // "GAUJES",
    // "GAULEITER",
    // "GAULEITERS",
    // "THEATERLANDS",
    // "THEATERS",
    // "THEATRAL",
    // "TYPEY",
    // "TYPHACEOUS",
    // "TYPHLITIC",
    // "TYPHLITIS",
    // "UPDATING",
    // "UPDIVE",
    // "UPDIVED",
    // "VICTIMIZE",
    // "VOICER",
    // "VOICERS",
    // "VOICES",
    // "VOICING",
    // "WILDCATTINGS",
    // "WILDEBEEST",
    // "WILDEBEESTS",
    // "WILDED",
    // "WOOFTAH",
    // "WOOFTAHS",
    // "WOOFTER",
    // "YAMPY",
    // "YAMS",
    // "YAMULKA",
    // "ZYMOSANS",
    // "ZYMOSES",
    // "ZYMOSIMETER",
    // "ZYMOSIMETERS",
    // "ZYMOSIS",
};
// clang-format on
#endif

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

void print_state(const Datrie2* dt)
{
    const std::size_t N = 7;
    printf("-----------------------------------------\n");
    printf("INDX :");
    for (std::size_t i = 0; i < N; ++i) {
        printf(" %2zu |", i);
    }
    printf("\n");
    printf("-----------------------------------------\n");

    printf("BASE :");
    for (std::size_t i = 0; i < N; ++i) {
        printf(" %2d |", dt->base[i]);
    }
    printf("\n");

    printf("CHCK :");
    for (std::size_t i = 0; i < N; ++i) {
        if (dt->chck[i] == -1) {
            printf("    |");
        } else {
            printf(" %2d |", dt->chck[i]);
        }
    }
    printf("\n");

    printf("TERM :");
    for (std::size_t i = 0; i < N; ++i) {
        printf(" %2d |", dt->term[i]);
    }
    printf("\n");
    printf("-----------------------------------------\n");
    printf("\n");
}

TEST_CASE("Datrie2")
{
    Datrie2 dt;

    {
        bool ok = init2(&dt);
        REQUIRE(ok == true);
    }

    for (std::size_t i = 0; i < DICT.size(); ++i)
    {
        const auto& word = DICT[i];
        INFO("Inserting word: " << word);
        printf("insert2: %s\n", word.c_str());
        const bool ok = insert2(&dt, word.c_str());
        REQUIRE(ok == true);

        print_state(&dt);

#if 1
        const auto found = isword2(&dt, word.c_str());
        CHECK(found == Tristate::eWord);

        // verify potential movement of entries didn't cause us to not
        // find words already inserted
        for (std::size_t j = 0; j < i; ++j) {
            const auto& word2  = DICT[j];
            INFO("After inserting " << word << ", checking " << word2);
            const auto  found2 = isword2(&dt, word2.c_str());
            CHECK(found2 == Tristate::eWord);
        }
        printf("\n\n");
#endif
    }

#if 0
    for (std::size_t i = 0; i < MISSING.size(); ++i)
    {
        const auto& word = MISSING[i];
        INFO("Looking for missing word: " << word);
        auto found = isword2(&dt, word.c_str());
        CHECK(found != Tristate::eWord);
    }
#endif

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
