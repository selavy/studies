#pragma once

#include <cstdint>
#include <string>
#include <vector>

struct Tarraysep
{
    using u32 = uint32_t;

    std::vector<u32> bases;
    std::vector<int> checks;
    std::vector<int> nexts;

    bool isword(const char* const word)  const;
    bool isword(const std::string& word) const { return isword(word.c_str()); }

private:
    static constexpr int MIN_CHILD_OFFSET = 1;
    static constexpr int MAX_CHILD_OFFSET = 27;
    static constexpr int MAX_BASE    = (1u << 30) - MAX_CHILD_OFFSET; // exclusive
    static constexpr int NO_BASE     = static_cast<int>(MAX_BASE);
    static constexpr u32 UNSET_BASE  = 0;
    static constexpr int UNSET_CHECK = MAX_BASE;
    static constexpr int UNSET_NEXT  = 0;
    static constexpr u32 TERM_MASK   = 0x1u;

    int base(int s)  const;
    int check(int s) const;
    int term(int s)  const;
    int next(int s)  const;
    void setbase(std::size_t s, int base, bool term);
};
