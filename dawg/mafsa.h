#pragma once

#include <map>
#include <vector>
#include <string>
#include <cassert>
#include <cstdint>


struct Mafsa
{
    struct Node
    {
        using Kids   = std::map<int, int>;
        using KidIdx = typename Kids::size_type;
        int  val;
        bool term;
        Kids kids;
    };

    Mafsa();
    void insert(const char* const word);
    void insert(const std::string& word) { insert(word.c_str()); }
    bool isword(const char* const word) const;
    bool isword(const std::string& word) const { return isword(word.c_str()); }

    int numstates() const;

    bool validstate(std::size_t i) const;

    void reduce();

    template <class F>
    void visit_post(int s, F&& f)
    {
        assert(s < static_cast<int>(ns.size()));
        auto& node = ns[static_cast<Node::KidIdx>(s)];
        for (auto [val, kid] : node.kids) {
            visit_post(kid, f);
        }
        f(s);
    }

    static bool nodecmp(const Node& a, const Node& b);

    std::vector<Node> ns;
};


struct SDFA
{
    using u32 = uint32_t;

    constexpr static u32 TERM_BIT     = 31;
    constexpr static u32 TERM_MASK    = 1u << TERM_BIT;
    constexpr static u32 INDEX_MASK   = ~TERM_MASK;
    constexpr static u32 NOTRANSITION = 0;
    constexpr static u32 NumLetters   = 26;

    // after reducing MA-FSA, should be able to have n_states * n_letters in memory
    // upper bit in t[x] indicates if x is a terminal state
    std::vector<u32> tt;

    bool isword(const char* const word) const;
    bool isword(const std::string& word) const { return isword(word.c_str()); }
    std::size_t size() const { return tt.size(); }

    static SDFA make(const Mafsa& m);
};
