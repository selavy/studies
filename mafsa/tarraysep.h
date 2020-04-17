#pragma once

#include <cstdint>
#include <cstddef>
#include <string>
#include <vector>
#include <optional>
#include <iosfwd>

struct Tarraysep
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

    std::vector<u32> bases;
    std::vector<int> checks;
    std::vector<int> nexts;

    explicit Tarraysep(std::size_t n_states=50) noexcept;
    bool isword(const char* const word)  const noexcept;
    bool isword(const std::string& word) const noexcept { return isword(word.c_str()); }

    // TODO(peter): maybe move this to a "serializers.h"?
    static std::optional<Tarraysep> deserialize(const std::string& filename);

    void dump_stats(std::ostream& os) const;

private:
    friend struct Mafsa;
    int base(int s)  const noexcept;
    int check(int s) const noexcept;
    int term(int s)  const noexcept;
    int next(int s)  const noexcept;
    void setbase(std::size_t s, int base, bool term) noexcept;
};
