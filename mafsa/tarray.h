#pragma once

#include <cstdint>
#include <cstddef>
#include <string>
#include <vector>
#include <optional>
#include <iosfwd>


struct Tarray
{
    using u32 = uint32_t;
    static constexpr int MIN_CHILD_OFFSET = 1;
    static constexpr int MAX_CHILD_OFFSET = 27;
    static constexpr int MAX_BASE    = (1u << 30) - MAX_CHILD_OFFSET; // exclusive
    static constexpr int NO_BASE     = static_cast<int>(MAX_BASE);
    static constexpr u32 UNSET_BASE  = 0;
    static constexpr int UNSET_CHECK = MAX_BASE;
    static constexpr int UNSET_NEXT  = 0;
    static constexpr u32 TERM_MASK   = 0x1u;

    struct Xtn
    {
        int check;
        int next;

        constexpr explicit Xtn(int c=UNSET_CHECK, int n=UNSET_NEXT) noexcept : check(c), next(n) {}
    };

    std::vector<u32> bases;
    std::vector<Xtn> xtns;

    bool isword(const char* const word)  const noexcept;
    bool isword(const std::string& word) const noexcept { return isword(word.c_str()); }

    // TODO(peter): maybe move this to a "serializers.h"?
    static std::optional<Tarray> deserialize(const std::string& filename);

    template <class BItr, class CItr, class NItr>
    static Tarray make(
               BItr bases_begin,  BItr bases_end,
               CItr checks_begin, CItr checks_end,
               NItr nexts_begin, NItr nexts_end
           )
    {
        Tarray tarray;
        tarray.bases.assign(bases_begin, bases_end);
        std::size_t n = std::distance(checks_begin, checks_end);
        tarray.xtns.insert(tarray.xtns.end(), n, Xtn{});
        auto citr = checks_begin;
        auto nitr = nexts_begin;
        for (std::size_t i = 0; i < n; ++i) {
            tarray.xtns[i].check = *citr++;
            tarray.xtns[i].next  = *nitr++;
        }
        return tarray;
    }

    void dump_stats(std::ostream& os) const;

private:
    friend struct Mafsa;
    int base(int s) const noexcept;
    Xtn xtn(int s)  const noexcept;
    int term(int s) const noexcept;
    void setbase(std::size_t s, int base, bool term) noexcept;
};
