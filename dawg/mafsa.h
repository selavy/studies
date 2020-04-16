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

    static constexpr u32 TERM_BIT     = 31;
    static constexpr u32 TERM_MASK    = 1u << TERM_BIT;
    static constexpr u32 INDEX_MASK   = ~TERM_MASK;
    static constexpr u32 NOTRANSITION = 0;
    static constexpr u32 NumLetters   = 26;

    // after reducing MA-FSA, should be able to have n_states * n_letters in memory
    // upper bit in t[x] indicates if x is a terminal state
    std::vector<u32> tt;

    bool isword(const char* const word) const;
    bool isword(const std::string& word) const { return isword(word.c_str()); }
    std::size_t size() const { return tt.size(); }

    static SDFA make(const Mafsa& m);
};

struct DATrie
{
    using u32 = uint32_t;

    // term state is first bit in base
    static constexpr int MIN_CHILD_OFFSET = 1;
    static constexpr int MAX_CHILD_OFFSET = 27;
    static constexpr int MAX_BASE    = (1u << 30) - MAX_CHILD_OFFSET; // exclusive
    static constexpr int NO_BASE     = static_cast<int>(MAX_BASE);
    static constexpr u32 UNSET_BASE  = 0;
    static constexpr int UNSET_CHECK = MAX_BASE;
    static constexpr u32 TERM_MASK   = 0x1u;

    std::vector<u32> base_;
    std::vector<int> check_;

    bool isword(const char* const word) const;
    bool isword(const std::string& word) const { return isword(word.c_str()); }
    int base(int s) const;
    int check(int s) const;
    int term(int s) const;
    // static DATrie make(const Mafsa& m);
};
