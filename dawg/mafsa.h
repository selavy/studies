#pragma once

#include <map>
#include <vector>
#include <string>
#include <cassert>


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

