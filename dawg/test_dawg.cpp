#include <cstdlib>
#include <cstring>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <memory>
#include <vector>
#include <algorithm>
// #include <map>


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
        int  value;
        int  nodenum;
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
    node->nodenum = 0;
    node->value   = 0;
    node->term    = false;
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
            new_node->nodenum = 0; // TODO: can I set this now?
            new_node->value   = c;
            new_node->term    = false;
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

    auto all_next_entries_fit = [](int index, const int* first, const int* last, const int* next, int next_size)
    {
        for (auto* c = first; c != last; ++c) {
            if (index + *c >= next_size) {
                return false;
            }
            if (next[index + *c] != -1) {
                return false;
            }
        }
        return true;
    };

    int next_base_idx = -1;
    for (int i = 0; i < next.size(); ++i) {
        if (all_next_entries_fit(i, &cs[0], &cs[n_cs], &next[0], next.size())) {
            next_base_idx = i;
            break;
        }
    }
    if (next_base_idx == -1) {
        assert(0 && "unable to find location for next entries");
    }

    assert(0 <= node->nodenum && node->nodenum < base.size());
    setbase3(t3, node->nodenum, next_base_idx, node->term);
    for (int i = 0; i < n_cs; ++i) {
        const int c = cs[i];
        const int t = next_base_idx + c;
        assert(0 <= t && t < next.size());
        assert(0 <= t && t < chck.size());
        assert(next[t] == -1);
        assert(chck[t] == -1);
        next[t] = nodes[node->links[c-1]].nodenum;
        chck[t] = node->nodenum;
    }
}

void _numbernodes(Trie* trie, int n, int& nodenum)
{
    trie->nodes[n].nodenum = nodenum++;
    auto* links = &trie->nodes[n].links[0];
    for (int i = 0; i < 26; ++i) {
        if (links[i] != 0) {
            _numbernodes(trie, links[i], nodenum);
        }
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

void build3(Datrie3* t3, Trie* trie, int n_symbols, int n_states)
{
    t3->base = Datrie3::Array(n_states            , -1);
    t3->next = Datrie3::Array(n_symbols * n_states, -1);
    t3->chck = Datrie3::Array(n_symbols * n_states, -1);
    int nn = 0;
    _numbernodes(trie, 0, nn);
    _build3(t3, trie, 0);
}

int main(int argc, char** argv)
{
    // clang-format off
    const std::vector<std::string> words = {
        "HE",
        "HEAT",
        "HEAL",
        "HEAP",
        "HEM",
        "HA",
        "HAT",
        "HATE",
        "HAPPY",
        "HI",
        "HIM",
        "HIP",
        "HIPPY",
        "HIT",
    };
    // clang-format on

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

    Datrie3 t3;
    build3(&t3, &t, n_symbols, n_states);

    // ------------------------------------------------------------------------
    // verification
    // ------------------------------------------------------------------------

    // clang-format off
    const std::vector<std::string> missing = {
        "HELLO",
        "HEE",
        "HITE",
        "MISSING",
        "ANOTHER",
        "Z",
        "ZZZ",
    };
    // clang-format on

#if 1
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

        int failures = 0;
        for (const auto& word : words) {
            if (!walk3(&t3, word.c_str())) {
                failures++;
                printf("Failure: failed to find '%s'\n", word.c_str());
            }
        }
        for (const auto& word : missing) {
            if (walk3(&t3, word.c_str())) {
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

    return 0;
}
