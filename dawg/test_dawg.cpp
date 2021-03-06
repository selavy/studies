#include <catch2/catch.hpp>
#include <vector>
#include <string>
#include <unordered_map>
#include <set>
#include <map>
#include <iostream>
#include <random>
#include <cstdlib>
#include <cstring>
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

Tatrie make_tatrie(const std::vector<std::string>& words)
{
    Mafsa m;
    for (const auto& word : words) {
        m.insert(word);
    }
    m.reduce();
    return Tatrie::make(m);
}

TEST_CASE("Tatrie")
{
    Tatrie tt = make_tatrie(DICT);
    for (const auto& word : DICT) {
        CHECK(tt.isword(word));
    }
    for (const auto& word : MISSING) {
        CHECK(!tt.isword(word));
    }
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
        "CLIP",
        "LIP",
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

    // std::cout << "# of states before: " << m.numstates() << "\n";
    m.reduce();
    // std::cout << "# of states after : " << m.numstates() << "\n";

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

TEST_CASE("SDFA")
{
    const std::vector<std::string> words = {
        "BLIP",
        "CAT",
        "CATNIP",
        "CLIP",
        "LIP",
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

    SDFA tt = SDFA::make(m);

    for (const auto& word : words) {
        CHECK(tt.isword(word));
    }
    for (const auto& word : MISSING) {
        CHECK(!tt.isword(word));
    }
    for (const auto& word : missing) {
        CHECK(!tt.isword(word));
    }
}

inline std::ostream& operator<<(std::ostream& os, const Mafsa::Node& n)
{
    [[maybe_unused]] auto tochar  = [](int v)  { return static_cast<char>(v + 'A'); };
    [[maybe_unused]] auto boolstr = [](bool v) { return v ? "T" : "F"; };
    os << "Node(" << tochar(n.val) << ", " << boolstr(n.term) << ", [ ";
    for (auto [val, kid] : n.kids) {
        os << "(" << tochar(val) << ", " << kid << ") ";
    }
    os << "])";
    return os;
}


using u32 = uint32_t;

struct Base1
{
    bool term : 1;
    int  base : 31;
};
static_assert(sizeof(Base1) == 4);

using Base2 = u32;

int getbase1(const Base1* bases, std::size_t n) noexcept
{
    return bases[n].base;
}

int getbase2(const Base2* bases, std::size_t n) noexcept
{
    return static_cast<int>(bases[n]) >> 1;
}

bool getterm1(const Base1* bases, std::size_t n) noexcept
{
    return bases[n].term;
}

bool getterm2(const Base2* bases, std::size_t n) noexcept
{
    return (bases[n] & 0x1u) != 0;
}

void setbase1(Base1* bases, std::size_t n, int val) noexcept
{
    bases[n].base = val;
}

void setterm1(Base1* bases, std::size_t n, bool val) noexcept
{
    bases[n].term = val;
}

void setbase2(Base2* bases, std::size_t n, int val) noexcept
{
    uint32_t uval = static_cast<uint32_t>(val);
    bases[n] = (uval << 1) | (bases[n] & 0x1u);
}

void setterm2(Base2* bases, std::size_t n, bool val) noexcept
{
    uint32_t uval = val & 0x1u;
    bases[n] = (bases[n] & ~0x1u) | uval;
}

void setbase_mimic_clang(Base2* rdi, std::size_t rsi, int edx) noexcept
{
    uint64_t rdx;
    uint32_t eax;

    rdx = static_cast<uint64_t>(edx);
    eax = rdi[rsi];                           // mov eax, dword ptr [rdi + 4*rsi]
    eax &= 1;                                 // and eax, 1
    eax = static_cast<uint64_t>(eax) + 2*rdx; // lea eax, [rax + 2*rdx]
    rdi[rsi] = eax;                           // move dword ptr [rdi + 4*rsi], eax
}

void setterm_mimic_clang(Base2* rdi, std::size_t rsi, bool edx) noexcept
{
    uint32_t eax;

    eax = rdi[rsi]; // move eax, dword ptr [rdi + 4*rsi]
    eax &= ~0x1u;   // and eax, -2
    eax |= edx;     // or eax, edx
    rdi[rsi] = eax; // move dword ptr [rdi + 4*rsi], eax
}

// implementation comparisons: https://gcc.godbolt.org/z/pKzwLS
TEST_CASE("DATRIE from MA-FSA")
{
    const std::vector<std::string> words = {
        "BAG",
        "BAT",
        "BLIP",
        "CAT",
        "CLIP",
        "LIP",
    };

    Mafsa m;
    for (const auto& word : words) {
        m.insert(word);
    }
    m.reduce();

    // std::cout << "Nodes:\n";
    // for (std::size_t i = 0; i < m.ns.size(); ++i) {
    //     std::cout << i << ": " << m.ns[i] << "\n";
    // }

    const Tatrie t = Tatrie::make(m);

    for (const auto& word : words) {
        CHECK(t.isword(word));
    }

#if 1
    for (const auto& word : MISSING) {
        CHECK(!t.isword(word));
    }

    SECTION("Unsigned magic")
    {
        constexpr int N = 16;
        // 32 bit base => 30 bits of magnitude, 1 sign bit, 1 term bit
        constexpr int MaxValue = 1 << 30;
        Base1 b1s[N];
        Base2 b2s[N];
        Base2 mimic_clangs[N];
        memset(b1s, 0, sizeof(b1s));
        memset(b2s, 0, sizeof(b2s));
        memset(mimic_clangs, 0, sizeof(mimic_clangs));
        std::vector<int>  correct_bases;
        std::vector<bool> correct_terms;

        srand(42);
        for (int iter = 0; iter < 10; ++iter) {
            correct_bases.clear();
            correct_terms.clear();

            for (int i = 0; i < N; ++i) {
                const int sign = rand() % 2 == 0 ? 1 : -1;
                const int uval = (rand() % MaxValue) * sign;
                const bool term = rand() % 2 == 0;
                correct_bases.push_back(uval);
                correct_terms.push_back(term);

                setbase1(b1s, i, uval);
                setbase2(b2s, i, uval);
                setbase_mimic_clang(mimic_clangs, i, uval);

                setterm1(b1s, i, term);
                setterm2(b2s, i, term);
                setterm_mimic_clang(mimic_clangs, i, term);
            }

            for (int i = 0; i < N; ++i) {
                CHECK(getbase1(b1s, i) == correct_bases[i]);
                CHECK(getbase2(b2s, i) == correct_bases[i]);
                CHECK(getbase2(mimic_clangs, i) == correct_bases[i]);

                CHECK(getterm1(b1s, i) == correct_terms[i]);
                CHECK(getterm2(b2s, i) == correct_terms[i]);
                CHECK(getterm2(mimic_clangs, i) == correct_terms[i]);
            }
        }
    }
#endif

}
