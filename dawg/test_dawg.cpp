#include <cstdlib>
#include <cstring>
#include <cassert>
#include <cstdint>
#include <climits>
#include <iostream>
#include <memory>
#include <vector>
#include <algorithm>
#include <optional>
// #include <map>
#include <fstream>

#if 0
#define Datrie Datrie3
#define build  build3
#define walk   walk3
#else
#define Datrie Datrie2
#define build  build2
#define walk   walk2
#endif


// TODO: make table based
int iconv(char c) {
    int result;
    if ('a' <= c && c <= 'z') {
        result = c - 'a';
    } else if ('A' <= c && c <= 'Z') {
        result = c - 'A';
    } else {
        assert(0 && "invalid character");
        result = -1; // should cause an error
    }
    assert(0 <= result && result < 26);
    return result;
}

struct Trie
{
    struct Node {
        int  parent = 0;
        int  base = 0;
        char  value;
        int  links[26];
        bool term;
    };

    // TODO: remove std::vector<> usage
    using Nodes = std::vector<Node>;
    Nodes nodes;
    int   symcnts[26];
};

void trie_init(Trie* t)
{
    memset(&t->symcnts[0], 0, sizeof(t->symcnts));
    t->nodes.emplace_back();
    auto* node = &t->nodes.back();
    node->value   = ' ';
    node->term    = false;
    node->parent  = 0;
    node->base    = 0;
    std::fill(std::begin(node->links), std::end(node->links), 0);
}

void trie_insert(Trie* t, const char* const word)
{
    auto& nodes = t->nodes;
    int i = 0;
    const char* ch = word;
    for (const char* ch = word; *ch != '\0'; ++ch) {
        const int c = iconv(*ch);
        t->symcnts[c]++;
        assert(i < nodes.size());
        auto* node = &nodes[i];
        if (node->links[c] == 0) {
            int next = static_cast<int>(nodes.size());
            nodes.emplace_back();
            auto* new_node = &nodes.back();
            new_node->base    = -1; // sentinel
            new_node->value   = *ch;
            new_node->term    = false;
            new_node->parent  = i;
            std::fill(std::begin(new_node->links), std::end(new_node->links), 0);
            node = &nodes[i]; // emplace could move the node
            node->links[c]  = next;
        }
        i = node->links[c];
        assert(i != 0);
    }
    nodes[i].term = 1;
}

void trie_insert(Trie* t, const std::string& word) { trie_insert(t, word.c_str()); }

bool trie_isword(const Trie* t, const char* const word)
{
    auto& nodes = t->nodes;
    int i = 0;
    for (const char* ch = word; *ch != '\0'; ++ch) {
        const int c = iconv(*ch);
        assert(i < nodes.size());
        const auto* node = &nodes[i];
        i = node->links[c];
        if (i == 0) {
            return false;
        }
    }
    return nodes[i].term;
}

bool trie_isword(const Trie* t, const std::string& word) { return trie_isword(t, word.c_str()); }

struct Datrie2
{
    using Array = std::vector<int>;
    Array base;
    Array chck;
    Array term;              // TEMP
    std::vector<char> vals;  // TEMP
};

int getbase2(const Datrie2* t, int s)
{
    assert(s >= 0);
    // return s < t->base.size() ? t->base[s] & ~(1u << 31) : 0;
    return s < t->base.size() ? t->base[s] : INT_MIN;
}

int getchck2(const Datrie2* t, int s)
{
    assert(s >= 0);
    return s < t->chck.size() ? t->chck[s] : INT_MIN;
}

bool getterm2(const Datrie2* t, int s)
{
    assert(s >= 0);
    // return s < t->base.size() ? (t->base[s] >> 31) != 0 : false;
    return s < t->term.size() ? t->term[s] : false;
}

void setbase2(Datrie2* t, int s, int base, bool term)
{
    assert(0 <= s && s < t->base.size());
    // t->base[s] = base | ((term ? 1 : 0) << 31);
    t->base[s] = base;
    t->term[s] = term;
}

bool walk2(const Datrie2* trie, const char* const word)
{
    printf("walk2: %s\n", word);
    auto& base = trie->base;
    auto& chck = trie->chck;
    int s = 0;
    for (const char* ch = word; *ch != '\0'; ++ch) {
        const int c = iconv(*ch) + 1;
        assert(1 <= c && c < 27);
        const int t   = getbase2(trie, s) + c;
        const int chk = getchck2(trie, t);
        printf("\t'%c' s=%d base[s]=%d c=%d t=%d chck[t]=%d\n", *ch, s, getbase2(trie, s), c, t, getchck2(trie, t));
        // if (getchck2(trie, t) != s) {
        if (chk != s) {
            return false;
        }
        assert(0 <= t && t < next.size());
        s = t;
    }
    return getterm2(trie, s);
}

bool walk2(const Datrie2* t, const std::string& word) { return walk2(t, word.c_str()); }

struct Datrie3
{
    using Array = std::vector<int>;
    Array base;
    Array next;
    Array chck;
};

int getbase3(const Datrie3* t, int s)
{
    assert(s >= 0);
    return s < t->base.size() ? t->base[s] & ~(1u << 31) : 0;
}

bool getterm3(const Datrie3* t, int s)
{
    assert(s >= 0);
    return s < t->base.size() ? (t->base[s] >> 31) != 0 : false;
}

void setbase3(Datrie3* t, int s, int base, bool term)
{
    assert(0 <= s && s < t->base.size());
    t->base[s] = base | ((term ? 1 : 0) << 31);
}

bool walk3(const Datrie3* trie, const char* const word)
{
    auto& base = trie->base;
    auto& next = trie->next;
    auto& chck = trie->chck;
    int s = 0;
    for (const char* ch = word; *ch != '\0'; ++ch) {
        const int c = iconv(*ch) + 1;
        assert(1 <= c && c < 27);
        int t = getbase3(trie, s) + c;
        if (t > chck.size() || chck[t] != s) {
            return false;
        }
        assert(0 <= t && t < next.size());
        s = next[t];
    }
    return getterm3(trie, s);
}

bool walk3(const Datrie3* t, const std::string& word) { return walk3(t, word.c_str()); }

int mksymcodes(const int freqs[26], int out[26])
{
    int syms[26];
    for (int i = 0; i < 26; ++i) {
        syms[i] = i;
    }
    std::sort(std::begin(syms), std::end(syms),
            [&freqs](int s1, int s2)
            {
                return freqs[s1] < freqs[s2];
            });
    for (int i = 0; i < 26; ++i) {
        out[syms[i]] = i + 1;
    }
    int n_symbols = 0;
    for (int i = 0; i < 26; ++i) {
        if (freqs[i] > 0) {
            n_symbols++;
        }
    }
    return n_symbols;
}

bool all_entries_fit(const int* first, const int* last, const int* vs)
{
    for (auto* c = first; c != last; ++c) {
        if (vs[*c] != -1) {
            return false;
        }
    }
    return true;
}

void _assign3(Datrie3* t3, int n, const Trie::Node* nodes)
{
    auto& base = t3->base;
    auto& next = t3->next;
    auto& chck = t3->chck;
    auto* node = &nodes[n];

    int cs[26];
    int n_cs = 0;
    for (int i = 0; i < 26; ++i) {
        if (node->links[i] == 0) {
            continue;
        }
        cs[n_cs++] = i + 1;
    }

    if (n_cs == 0) {
        return;
    }

    int next_base_idx = -1;
    int maxc = cs[n_cs - 1];
    for (int i = 0, N = next.size() - maxc; i < N; ++i) {
        if (all_entries_fit(&cs[0], &cs[n_cs], &next[i])) {
            next_base_idx = i;
            break;
        }
    }
    assert(next_base_idx != -1);

    const int s = n;
    assert(0 <= s && s < base.size());
    setbase3(t3, s, next_base_idx, node->term);
    for (int i = 0; i < n_cs; ++i) {
        const int c = cs[i];
        const int t = next_base_idx + c;
        const int link = node->links[c-1];
        assert(0 <= t && t < next.size());
        assert(0 <= t && t < chck.size());
        assert(next[t] == -1);
        assert(chck[t] == -1);
        next[t] = link;
        chck[t] = s;
    }
}

void _build3(Datrie3* t3, const Trie* trie, int n)
{
    auto* nodes = &trie->nodes[0];
    auto* node  = &nodes[n];
    auto* links = &node->links[0];
    _assign3(t3, n, nodes);
    for (int i = 0; i < 26; ++i) {
        if (links[i] == 0) {
            continue;
        }
        _build3(t3, trie, links[i]);
    }
}

void build3(Datrie3* t3, const Trie* trie, int n_symbols, int n_states)
{
    t3->base = Datrie3::Array(n_states            , -1);
    t3->next = Datrie3::Array(n_symbols * n_states, -1);
    t3->chck = Datrie3::Array(n_symbols * n_states, -1);
    _build3(t3, trie, 0);
}

void printcs(const int* cs, const int* cs_end)
{
    printf("[ ");
    for (; cs != cs_end; ++cs) {
        printf("%c ", (char)(*cs - 1 + 'A'));
    }
    printf("]");
}

void _assign2(Datrie2* t2, int n, Trie::Node* nodes)
{
    auto& base = t2->base;
    auto& chck = t2->chck;
    auto* node = &nodes[n];
    auto* links = &node->links[0];

    int cs[26];
    int n_cs = 0;
    for (int i = 0; i < 26; ++i) {
        if (links[i] == 0) {
            continue;
        }
        cs[n_cs++] = i + 1;
    }
    if (n_cs == 0) {
        return;
    }

    int next_base_idx = -1;
    int maxc = cs[n_cs - 1];
    for (int i = 0, N = base.size() - maxc; i < N; ++i) {
        if (all_entries_fit(&cs[0], &cs[n_cs], &base[i])) {
            next_base_idx = i;
            break;
        }
    }
    assert(next_base_idx != -1);

    const int my_c = iconv(node->value) + 1;
    assert(nodes[node->parent].base >= 0);
    const int s = nodes[node->parent].base + my_c;
    assert(0 <= s && s < base.size());
    setbase2(t2, s, next_base_idx, node->term);
    t2->vals[s] = node->value;
    node->base = next_base_idx;
    printf("assign2: c=%c parent=%c my_c=%d s=%d next_base=%d children=",
            node->value, nodes[node->parent].value, my_c, s, next_base_idx);
    printcs(&cs[0], &cs[n_cs]);
    printf("\n");
    for (int i = 0; i < n_cs; ++i) {
        const int c = cs[i];
        const int t = next_base_idx + c;
        assert(0 <= t && t < chck.size());
        assert(chck[t] == -1);
        chck[t] = s;
    }
}

void _build2(Datrie2* t2, Trie* trie, int n)
{
    auto* nodes = &trie->nodes[0];
    auto* node  = &nodes[n];
    auto* links = &node->links[0];
    _assign2(t2, n, nodes);
    for (int i = 0; i < 26; ++i) {
        if (links[i] == 0) {
            continue;
        }
        _build2(t2, trie, links[i]);
    }
}

void build2(Datrie2* t2, Trie* trie, int n_symbols, int n_states)
{
    t2->base = Datrie3::Array(n_symbols * n_states, -1);
    t2->chck = Datrie3::Array(n_symbols * n_states, -1);
    t2->term = Datrie3::Array(n_symbols * n_states, -1);  // TEMP TEMP
    t2->vals = std::vector<char>(n_symbols * n_states, ' '); // TEMP TEMP
    t2->base[0] = 0;
    _build2(t2, trie, 0);
}

std::optional<Trie> load_dictionary(std::string path, int max_entries=INT_MAX) {
    Trie trie;
    trie_init(&trie);
    std::string word;
    std::ifstream ifs{path};
    int nwords = 0;
    if (!ifs) {
        std::cerr << "error: unable to open input file\n";
        return std::nullopt;
    }
    while (ifs >> word) {
        if (word.empty()) {
            continue;
        }
        if (word.size() < 2 || word.size() > 15) {
            std::cerr << "warning: skipping invalid word: \"" << word << "\"\n";
        }
        bool valid_word = true;
        for (std::size_t i = 0; i < word.size(); ++i) {
            char c = word[i];
            if ('a' <= c && c <= 'z') {
                word[i] = (c - 'a') + 'A';
            } else if ('A' <= c && c <= 'Z') {
                word[i] = c;
            } else {
                std::cerr << "warning: invalid character '" << c << "' in word \"" << word << "\"\n";
                valid_word = false;
                break;
            }
        }
        if (valid_word) {
            trie_insert(&trie, word);
            if (++nwords == max_entries) {
                break;
            }
        }
    }
    return trie;
}

void test_dict(const char* const path)
{
    auto maybe_trie = load_dictionary(path, 1000);
    if (!maybe_trie) {
        fprintf(stderr, "unable to load dictionary from %s\n", path);
        return;
    }
    auto& trie = *maybe_trie;
    printf("Loading dictionary\n");

    int codes[26];
    const int n_symbols = mksymcodes(trie.symcnts, codes);
    const int n_states  = static_cast<int>(trie.nodes.size());
    printf("# symbols = %d\n", n_symbols);
    printf("# states  = %d\n", n_states);

    printf("Building Tripple array...\n");

    Datrie dt;
    build(&dt, &trie, n_symbols, n_states);
    printf("Finished!\n");

    // clang-format off
    std::vector<std::string> words = {
        "AA",
        "AAHS",
        "AAL",
        "AALII",
        "AALIIS",
        "AALS",
        "AARDVARK",
        "ABASIA",
    };
    // clang-format on

    int fails = 0;
    for (const auto& word : words) {
        bool found = walk(&dt, word);
        if (!found) {
            ++fails;
            printf("Failed to find: '%s'\n", word.c_str());
        } else {
            printf("Success! found %s\n", word.c_str());
        }
    }

    // clang-format off
    std::vector<std::string> missing = {
        "MISSINX",
        "AAX",
        "ABAA",
        "ZZZZZZ",
        "ZAZA",
    };
    // clang-format on

    for (const auto& word : missing) {
        bool found = walk(&dt, word);
        if (found) {
            ++fails;
            printf("Failed to NOT find: '%s'\n", word.c_str());
        }
    }

    if (fails == 0) {
        printf("Passed.\n");
    } else{
        printf("Failed: %d\n", fails);
    }

}

int main(int argc, char** argv)
{
    if (argc > 1) {
        test_dict(argv[1]);
        return 0;
    }

    // clang-format off
    const std::vector<std::string> words = {
        "HE",
        "HEAT",
        "HEAL",
        "HEAP",
        // "HEM",
        // "HA",
        // "HAT",
        // "HATE",
        // "HAPPY",
        // "HI",
        // "HIM",
        // "HIP",
        // "HIPPY",
        // "HIT",
        // "APPLE",
    };
    // clang-format on

    printf("--- WORDS ---\n");
    for (const auto& word : words) {
        printf("\t%s\n", word.c_str());
    }
    printf("--- END WORDS ---\n\n");

    Trie t;
    trie_init(&t);
    for (const auto& word : words) {
        trie_insert(&t, word);
    }

    int codes[26];
    const int n_symbols = mksymcodes(t.symcnts, codes);
    const int n_states  = static_cast<int>(t.nodes.size());
    printf("# symbols = %d\n", n_symbols);
    printf("# states  = %d\n", n_states);


    Datrie dt;
    build(&dt, &t, n_symbols, n_states);

    // ------------------------------------------------------------------------
    // verification
    // ------------------------------------------------------------------------

    // clang-format off
    const std::vector<std::string> missing = {
        "AH",
        "AAH",
        "HELLO",
        "HEE",
        "HITE",
        "MISSING",
        "ANOTHER",
        "Z",
        "ZZZ",
    };
    // clang-format on

#if 0
    { // Trie verification
        printf("trie verfication starting...\n");

        int failures = 0;
        for (const auto& word : words) {
            if (!trie_isword(&t, word)) {
                failures++;
                printf("Failure: failed to find '%s'\n", word.c_str());
            }
        }
        for (const auto& word : missing) {
            if (trie_isword(&t, word)) {
                failures++;
                printf("Failure: incorrectly found '%s'\n", word.c_str());
            }
        }
        if (failures == 0) {
            printf("trie verfication: passed.\n");
        } else {
            printf("trie verfication: failed. %d failures\n", failures);
        }
    }
#endif

#if 1
    { // Datrie3 verification
        printf("datrie3 verfication starting...\n");

        int tests = 0;
        int fails = 0;
        for (const auto& word : words) {
            tests++;
            if (!walk(&dt, word.c_str())) {
                fails++;
                printf("Failure: failed to find '%s'\n", word.c_str());
            }
        }
        for (const auto& word : missing) {
            tests++;
            if (walk(&dt, word.c_str())) {
                fails++;
                printf("Failure: incorrectly found '%s'\n", word.c_str());
            }
        }

        if (fails == 0) {
            printf("trie verfication: passed %d tests.\n", tests);
        } else {
            printf("trie verfication: failed. %d fails out of %d tests\n", fails, tests);
        }
    }
#endif

    return 0;
}
