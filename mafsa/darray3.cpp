#include "darray3.h"
#include <climits>
#include <cstddef>
#include <cassert>
#include <fstream>
#include <iostream>
#include "iconv.h"
#include "darray_generated.h"
#include "tarray_util.h"


// TODO: remove
#define AsIdx(x) static_cast<std::size_t>(x)

Darray3::Darray3()
    : bases (1, UNSET_BASE )
    , checks(1, UNSET_CHECK) // should it be initialized to 0?
{
    static_assert((UNSET_BASE   & TERM_MASK) == 0, "unset base must not have terminal bit set");
    static_assert((MISSING_BASE & TERM_MASK) == 0, "missing base must not have terminal bit set");
    static_assert((UNSET_BASE   & TAIL_MASK) == 0, "unset base must not have tail bit set");
    static_assert((MISSING_BASE & TAIL_MASK) == 0, "missing base must not have tail bit set");
    static_assert((MISSING_BASE + static_cast<u32>(MAX_CHILD_OFFSET)) < static_cast<u32>(INT_MAX),
            "adding max child offset would overflow missing base");
    static_assert((UNSET_BASE + static_cast<u32>(MAX_CHILD_OFFSET)) < static_cast<u32>(INT_MAX),
            "adding max child offset would overflow missing base");
}

int Darray3::base(int index) const
{
    assert(index >= 0);
    auto s = static_cast<std::size_t>(index);
    return s < bases.size() ? static_cast<int>(bases[s] & BASE_MASK) : MISSING_BASE;
}

int Darray3::check(int index) const
{
    assert(index >= 0);
    auto s = static_cast<std::size_t>(index);
    return s < checks.size() ? checks[s] : UNSET_CHECK;
}

bool Darray3::term(int index) const
{
    assert(index >= 0);
    auto s = static_cast<std::size_t>(index);
    return s < bases.size() ? (bases[s] & TERM_MASK) != 0 : false;
}

bool Darray3::intail(int index) const
{
    assert(index >= 0);
    auto s = static_cast<std::size_t>(index);
    return s < bases.size() ? (bases[s] & TAIL_MASK) != 0 : false;
}

void Darray3::setbase(int index, int val)
{
    assert(index >= 0);
    assert(val  >= 0);
    assert(static_cast<u32>(val) < MAX_BASE);
    auto s = static_cast<std::size_t>(index);
    assert(s < bases.size());
    bases[s] = (bases[s] & (TAIL_MASK | TERM_MASK)) | static_cast<u32>(val);
}

void Darray3::setcheck(int index, int val)
{
    assert(index >= 0);
    auto s = static_cast<std::size_t>(index);
    assert(s < checks.size());
    checks[s] = val;
}

void Darray3::setterm(int index, bool val)
{
    assert(index >= 0);
    auto s = static_cast<std::size_t>(index);
    assert(s < bases.size());
    const u32 bit = val ? 1u : 0u;
    bases[s] |= bit << TERM_BIT;
}

void Darray3::setintail(int index, bool val)
{
    assert(index >= 0);
    auto s = static_cast<std::size_t>(index);
    assert(s < bases.size());
    const u32 bit = val ? 1u : 0u;
    bases[s] |= bit << TAIL_BIT;
}

bool Darray3::istailsuffix(int s, const char* const word) const
{
#if 0
    assert(istail(s));
    std::size_t i = static_cast<std::size_t>(s);
    for (auto* p = reinterpret_cast<const u8*>(word); *p != '\0'; ++p) {
        if ((*p & 0x7Fu) != (tail[i++] & 0x7Fu)) {
            return false;
        }
    }
    return tail[i] & 0x80u;
#endif
    return true;
}

bool Darray3::isword(const char* const word) const
{
#if 0
    int s = 0;
    for (const char* p = word; *p != '\0'; ++p) {
        const int c = sconv(*p);
        const int t = base(s) + c;
        if (check(t) != s) {
            return false;
        }
        if (istail(t)) {
            return istailsuffix(s, p + 1);
        }
        s = t;
    }
    return term(s);
#endif
    return true;
}

#if 0
std::optional<Darray3> Darray3::deserialize(const std::string& filename)
{
    auto buf = read_dict_file(filename);
    auto serial_darray = GetSerialDarray(buf.data());
    flatbuffers::Verifier v(reinterpret_cast<const uint8_t*>(buf.data()), buf.size());
    assert(serial_darray->Verify(v));
    Darray3 darray;
    auto* bases  = serial_darray->bases();
    auto* checks = serial_darray->checks();
    darray.bases .assign(bases ->begin(), bases ->end());
    darray.checks.assign(checks->begin(), checks->end());
    return darray;
}
#endif

template <class Cont>
void vec_stats(std::ostream& os, Cont& vec, std::string name, std::size_t& items, std::size_t& bytes)
{
    os << name << " : items=" << vec.size() << ", bytes=" << (vec.size() * sizeof(vec[0])) << "\n";
    items += vec.size();
    bytes += vec.size() * sizeof(vec[0]);
}

void Darray3::dump_stats(std::ostream& os) const
{
    std::size_t total_items = 0;
    std::size_t total_bytes = 0;
    os << "Darray3 Stats:\n";
    vec_stats(os, bases , "base ", total_items, total_bytes);
    vec_stats(os, checks, "check", total_items, total_bytes);
    os << "total items=" << total_items << ", total bytes=" << total_bytes << "\n";
}
