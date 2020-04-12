#include <cstdlib>
#include <cstring>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <memory>
#include <vector>
// #include <map>


// TODO: make table based
int iconv(char c) {
    int result;
    if ('a' <= c && c <= 'z') {
        result = c - 'a' + 1;
    } else if ('A' <= c && c <= 'Z') {
        result = c - 'A' + 1;
    } else {
        assert(0 && "invalid character");
        result = -1; // should cause an error
    }
    assert(1 <= result && result < 27);
    return result;
}

struct Trie
{
    struct Node {
        int  value;
        int  nodenum;
        int  links[27];
        bool term;
    };

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

int getbase3(const Datrie3& trie, int s)
{
    assert(s >= 0);
    return s < trie.base.size() ? trie.base[s] & ~(1u << 31) : 0;
}

bool getterm3(const Datrie3& trie, int s)
{
    assert(s >= 0);
    return s < trie.base.size() ? (trie.base[s] >> 31) != 0 : false;
}

void setbase3(Datrie3& trie, int s, int base, bool term)
{
    assert(0 <= s && s < trie.base.size());
    trie.base[s] = base | ((term ? 1 : 0) << 31);
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

    // ------------------------------------------------------------------------
    // Trie verification
    // ------------------------------------------------------------------------

    printf("trie verfication starting...\n");
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
    // clang-format ont

    int trie_failures = 0;
    for (const auto& word : words) {
        if (!trie_isword(&t, word)) {
            trie_failures++;
            printf("Failure: failed to find '%s'\n", word.c_str());
        }
    }
    for (const auto& word : missing) {
        if (trie_isword(&t, word)) {
            trie_failures++;
            printf("Failure: incorrectly found '%s'\n", word.c_str());
        }
    }

    if (trie_failures == 0) {
        printf("trie verfication: passed.\n");
    } else {
        printf("trie verfication: failed. %d failures\n", trie_failures);
    }

    return 0;
}
