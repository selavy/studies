#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <optional>
#include <iosfwd>


struct Darray3
{
    using u32 = uint32_t;
    using u8  = uint8_t;

    std::vector<u32> bases;
    std::vector<int> checks;
    std::vector<u8 > suffixs;

    Darray3();
    bool isword(const char* const word)  const;
    bool isword(const std::string& word) const { return isword(word.c_str()); }

    // TODO(peter): maybe move this to a "serializers.h"?
    // static std::optional<Darray3> deserialize(const std::string& filename);

    void dump_stats(std::ostream& os) const;

private:
    int  base(int index) const;
    int  check(int index) const;
    bool term(int index) const;
    bool intail(int index) const;
    void setbase(int index, int val);
    void setcheck(int index, int val);
    void setterm(int index, bool val);
    void setintail(int index, bool val);
    bool istailsuffix(int index, const char* const word) const;

    static constexpr int MAX_CHILD_OFFSET = 27;
    static constexpr int TAIL_BIT     = 30;
    static constexpr u32 TAIL_MASK    = 1u << TAIL_BIT;
    static constexpr int TERM_BIT     = 31;
    static constexpr u32 TERM_MASK    = 1u << TERM_BIT;
    static constexpr u32 BASE_MASK    = ~(TERM_MASK | TAIL_MASK);
    static constexpr u32 MAX_BASE     = (1u << 30) - MAX_CHILD_OFFSET; // exclusive
    static constexpr int MISSING_BASE = static_cast<int>(MAX_BASE);
    static constexpr u32 UNSET_BASE   =  0;
    static constexpr int UNSET_CHECK  = -1;
    static constexpr int UNSET_TERM   =  0;
};
