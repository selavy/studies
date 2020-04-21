#pragma once

#include <vector>
#include <string>
#include <cstdint>
#include <optional>


struct Mafsa2
{
    using u32 = uint32_t;
    struct Node
    {
        int  children[26];
    };
    std::vector<Node> nodes;
    std::vector<bool> terms; // TODO: better to store in the node?

    bool isword(const char* const word) const noexcept;
    bool isword(const std::string& word) const noexcept { return isword(word.c_str()); }
    void dump_stats(std::ostream& os) const;
    static std::optional<Mafsa2> deserialize(const std::string& filename);
};
