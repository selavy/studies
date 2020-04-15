#pragma once

#include <map>
#include <vector>
#include <string>


struct Mafsa
{
    struct Node
    {
        using Kids   = std::map<int, int>;
        using KidIdx = typename Kids::size_type;
        char val;
        bool term;
        Kids kids;
    };

    void insert(const char* const word);
    void insert(const std::string& word) { insert(word.c_str()); }
    bool isword(const char* const word) const;
    bool isword(const std::string& word) const { return isword(word.c_str()); }

    static bool nodecmp(const Node& a, const Node& b);

    std::vector<Node> ns;
};

