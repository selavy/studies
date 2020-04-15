#include <catch2/catch.hpp>
#include <vector>
#include <string>
#include <unordered_map>
#include <set>
#include <map>
#include <iostream>
#include "dawg.h"

#include "mafsa.h"

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
    // "ABA",
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

#if 0
void print_state(const Datrie2* dt)
{
    const std::size_t N = std::min<std::size_t>(11, dt->chck.size());
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

    printf("-----------------------------------------\n");
    printf("\n");
}
#endif

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

        const bool ok = insert2(&dt, word.c_str());
        REQUIRE(ok == true);

        const auto found = isword2(&dt, word.c_str());
        CHECK(found == Tristate::eWord);

        // verify potential movement of entries didn't cause us to not
        // find words already inserted
        for (std::size_t j = 0; j <= i; ++j) {
            const auto& word2  = DICT[j];
            INFO("After inserting " << word << ", checking " << word2);

            const auto  found2 = isword2(&dt, word2.c_str());
            CHECK(found2 == Tristate::eWord);

            const auto  ok2    = insert2(&dt, word2.c_str());
            REQUIRE(ok2 == true);
        }
    }

    for (std::size_t i = 0; i < MISSING.size(); ++i)
    {
        const auto& word = MISSING[i];
        INFO("Looking for missing word: " << word);
        auto found = isword2(&dt, word.c_str());
        CHECK(found != Tristate::eWord);
    }


    for (const auto& word : DICT) {
        bool ok = insert2(&dt, word.c_str());
        REQUIRE(ok == true);
    }
    for (const auto& word : DICT) {
        CHECK(isword2(&dt, word.c_str()) == Tristate::eWord);
    }
    for (const auto& word : MISSING) {
        CHECK(isword2(&dt, word.c_str()) != Tristate::eWord);
    }
}

TEST_CASE("Datrie2 get children")
{
    std::vector<std::string> dict = {
        "AA",
        "AAH",
        "AAHS",
        "AAB",
        "BAT",
        "BATS",
        "CRAB",
        "AAT",
    };

    Datrie2 dt;
    REQUIRE(init2(&dt) == true);

    for (const auto& word : dict) {
        REQUIRE(insert2(&dt, word.c_str()) == true);
    }

    SECTION("AA children")
    {
        Letters result = kids2(&dt, "AA");
        std::map<char, int> cnts;
        for (int i = 0; i < result.n_kids; ++i) {
            cnts[result.kids[i]]++;
        }
        CHECK(result.n_kids == 3);
        CHECK(cnts['B'] == 1);
        CHECK(cnts['H'] == 1);
        CHECK(cnts['T'] == 1);
    }

    SECTION("AAH children")
    {
        Letters result = kids2(&dt, "AAH");
        std::map<char, int> cnts;
        for (int i = 0; i < result.n_kids; ++i) {
            cnts[result.kids[i]]++;
        }
        CHECK(result.n_kids == 1);
        CHECK(cnts['S'] == 1);
    }

    SECTION("MISSING children")
    {
        Letters result = kids2(&dt, "MISSING");
        CHECK(result.n_kids == 0);
    }
}

TEST_CASE("MAFSA Insert")
{
    std::vector<std::string> dict{DICT};
    std::sort(dict.begin(), dict.end());
    REQUIRE(std::is_sorted(dict.begin(), dict.end()));

    Mafsa m;
    for (const auto& word : dict) {
        m.insert(word);
    }
    for (const auto& word : dict) {
        CHECK(m.isword(word));
    }
    for (const auto& word : MISSING) {
        CHECK(!m.isword(word));
    }
}

TEST_CASE("MAFSA reduce simple word list")
{
    const std::vector<std::string> words = {
        "BLIP",
        "CAT",
        "CATNIP",
    };
    REQUIRE(std::is_sorted(words.begin(), words.end()));

    const std::vector<std::string> missing = {
        "CATS",
        "CATNIPS",
        "BL",
        "BLIPS",
        "CATNI",
        "CA",
        "CATN",
        "CANIP",
        "CATNIPP",
    };

    Mafsa m;
    for (const auto& word : words) {
        m.insert(word);
    }

    SECTION("Verify find word correct after insertion")
    {
        for (const auto& word : words) {
            CHECK(m.isword(word));
        }
        for (const auto& word : MISSING) {
            CHECK(!m.isword(word));
        }
        for (const auto& word : missing) {
            CHECK(!m.isword(word));
        }
    }

    m.reduce();

#if 1
    SECTION("Verify find word still correct after reduce")
    {
        for (const auto& word : words) {
            CHECK(m.isword(word));
        }
        for (const auto& word : MISSING) {
            CHECK(!m.isword(word));
        }
        for (const auto& word : missing) {
            CHECK(!m.isword(word));
        }
    }
#endif
}
