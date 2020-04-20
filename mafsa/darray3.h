#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <optional>
#include <iosfwd>


struct Darray3
{
    using u32 = uint32_t;

    std::vector<u32> bases;
    std::vector<int> checks;

    Darray3();
    void trim();
    void insert(const char* const word);
    void insert(const std::string& word) { return insert(word.c_str()); }
    bool isword(const char* const word)  const;
    bool isword(const std::string& word) const { return isword(word.c_str()); }

    // TODO(peter): maybe move this to a "serializers.h"?
    // static std::optional<Darray3> deserialize(const std::string& filename);

    void dump_stats(std::ostream& os) const;

private:
    int  getbase(int index) const;
    int  getcheck(int index) const;
    bool getterm(int index) const;
    void setbase(int index, int val);
    void setcheck(int index, int val);
    void setterm(int index, bool val);
    void clrbase(int index);
    void clrcheck(int index);
    void clrterm(int index);

    void relocate(int s, int b, int* childs, int n_childs);
    int  countchildren(int s, int* childs) const;
    static int findbase(const int* const first, const int* const last, int c);
    static int findbaserange(const int* const first, const int* const last, const int* const cs, const int* const csend);

    static constexpr int MIN_CHILD_OFFSET = 1;
    static constexpr int MAX_CHILD_OFFSET = 27;
    static constexpr int TERM_BIT     = 31;
    static constexpr u32 TERM_MASK    = 1u << TERM_BIT;
    static constexpr u32 BASE_MASK    = ~TERM_MASK;
    static constexpr u32 MAX_BASE     = (1u << 30) - MAX_CHILD_OFFSET; // exclusive
    static constexpr int MISSING_BASE = static_cast<int>(MAX_BASE);
    static constexpr u32 UNSET_BASE   =  0;
    static constexpr int UNSET_CHECK  = MAX_BASE;
    static constexpr int UNSET_TERM   =  0;
};
