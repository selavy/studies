#pragma once

#include <iosfwd>
#include <map>
#include <vector>
#include <string>
#include <cassert>
#include <cstdint>
#include <optional>
#include "tarraysep.h"


struct Mafsa
{
    using u32 = uint32_t;

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

    void dump_stats(std::ostream& os) const;

    static std::optional<Mafsa> deserialize(const std::string& filename);

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

    template <class F>
    void visit_pre(int s, F&& f) const
    {
        assert(0 <= s && s < static_cast<int>(ns.size()));
        f(s);
        auto& node = ns[static_cast<Node::KidIdx>(s)];
        for (auto [val, kid] : node.kids) {
            visit_pre(kid, f);
        }
    }

    static bool nodecmp(const Node& a, const Node& b);

    Tarraysep make_tarray() const;

    std::vector<Node> ns;
};
