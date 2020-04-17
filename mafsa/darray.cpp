#include "darray.h"
#include <climits>
#include <cstddef>
#include <cassert>
#include "iconv.h"

Darray::Darray()
    : bases {1000, UNSET_BASE }
    , checks{1000, UNSET_CHECK} // should it be initialized to 0?
{
    bases[0] = 0;
    static_assert((UNSET_BASE   & TERM_MASK) == 0, "unset base must not have terminal bit set");
    static_assert((MISSING_BASE & TERM_MASK) == 0, "missing base must not have terminal bit set");
    static_assert((MISSING_BASE + static_cast<u32>(MAX_CHILD_OFFSET)) < static_cast<u32>(INT_MAX),
            "adding max child offset would overflow missing base");
    static_assert((UNSET_BASE + static_cast<u32>(MAX_CHILD_OFFSET)) < static_cast<u32>(INT_MAX),
            "adding max child offset would overflow missing base");

}

int Darray::getbase(int index) const
{
    assert(index >= 0);
    auto s = static_cast<std::size_t>(index);
    if (s < bases.size()) {
        return static_cast<int>(bases[s] & BASE_MASK);
    } else {
        return MISSING_BASE;
    }
}

int Darray::getcheck(int index) const
{
    assert(index >= 0);
    auto s = static_cast<std::size_t>(index);
    return s < checks.size() ? checks[s] : UNSET_CHECK;
}

bool Darray::getterm(int index) const
{
    assert(index >= 0);
    auto s = static_cast<std::size_t>(index);
    return s < bases.size() ? (bases[s] & TERM_MASK) != 0 : false;
}

void Darray::setbase(int index, int val)
{
    assert(index >= 0);
    assert(val  >= 0);
    assert(static_cast<u32>(val) < MAX_BASE);
    auto s = static_cast<std::size_t>(index);
    assert(s < bases.size());
    assert((UNSET_BASE & TERM_MASK) == 0);
    bases[s] = (bases[s] & TERM_MASK) | static_cast<u32>(val);
}

void Darray::setcheck(int index, int val)
{
    assert(index >= 0);
    auto s = static_cast<std::size_t>(index);
    assert(s < checks.size());
    checks[s] = val;
}

void Darray::setterm(int index, bool val)
{
    assert(index >= 0);
    auto s = static_cast<std::size_t>(index);
    assert(s < bases.size());
    const u32 bit = val ? 1u : 0u;
    bases[s] |= (bit << TERM_BIT);
}

void Darray::clrbase(int index)
{
    assert(index >= 0);
    auto s = static_cast<std::size_t>(index);
    assert(s < bases.size());
    bases[s] = UNSET_BASE;
}

void Darray::clrcheck(int index)
{
    assert(index >= 0);
    auto s = static_cast<std::size_t>(index);
    assert(s < checks.size());
    checks[s] = UNSET_CHECK;
}

void Darray::clrterm(int index)
{
    assert(index >= 0);
    auto s = static_cast<std::size_t>(index);
    assert(s < checks.size());
    bases[s] &= ~TERM_MASK;
}

void Darray::insert(const char* const word)
{
    // TODO: 
}

bool Darray::isword(const char* const word) const
{
    int s = 0;
    for (const char* p = word; *p != '\0'; ++p) {
        const char ch = *p;
        const int  c  = iconv(ch) + 1;
        const int  t  = getbase(s) + c;
        if (getcheck(t) != s) {
            return false;
        }
        s = t;
    }
    return getterm(s);
}
