#include "tarraysep.h"
#include <cassert>
#include <cstddef>
#include <fstream>
#include "iconv.h"
#include "tarray_generated.h"
#include "tarray_util.h"


Tarraysep::Tarraysep(std::size_t n_states) noexcept
    : bases (n_states, UNSET_BASE)
    , checks(n_states, UNSET_CHECK)
    , nexts (n_states, UNSET_NEXT)
{
}

bool Tarraysep::isword(const char* const word) const noexcept
{
    int s = 0;
    for (const char* p = word; *p != '\0'; ++p) {
        const char ch = *p;
        const int c = sconv(ch);
        const int t = base(s) + c;
        if (check(t) != s) {
            return false;
        }
        s = nexts[static_cast<std::size_t>(t)];
    }
    return term(s);
}

int Tarraysep::base(int index) const noexcept
{
    auto s = static_cast<std::size_t>(index);
    return s < bases.size() ? static_cast<int>(bases[s]) >> 1 : NO_BASE;
}

int Tarraysep::check(int index) const noexcept
{
    auto s = static_cast<std::size_t>(index);
    return s < checks.size() ? checks[s] : UNSET_CHECK;
}

int Tarraysep::term(int index) const noexcept
{
    auto s = static_cast<std::size_t>(index);
    return s < bases.size() ? (bases[s] & 0x1u) != 0 : false;
}

int Tarraysep::next(int index) const noexcept
{
    auto s = static_cast<std::size_t>(index);
    return s < nexts.size() ? nexts[s] : 0;
}

void Tarraysep::setbase(std::size_t n, int val, bool term) noexcept
{
    u32 uval  = static_cast<u32>(val);
    u32 uterm = static_cast<u32>(term);
    bases[n] = (uval << 1) | (uterm & 0x1u);
}

std::optional<Tarraysep> Tarraysep::deserialize(const std::string& filename)
{
    auto buf = read_dict_file(filename);
    auto serial_tarray = GetSerialTarray(buf.data());
    flatbuffers::Verifier v(reinterpret_cast<const uint8_t*>(buf.data()), buf.size());
    assert(serial_tarray->Verify(v));
    Tarraysep tarray;
    auto* bases  = serial_tarray->bases();
    auto* checks = serial_tarray->checks();
    auto* nexts  = serial_tarray->nexts();
    tarray.bases .assign(bases ->begin(), bases ->end());
    tarray.checks.assign(checks->begin(), checks->end());
    tarray.nexts .assign(nexts ->begin(), nexts ->end());
    return tarray;
}

template <class Cont>
void vec_stats(std::ostream& os, Cont& vec, std::string name, std::size_t& items, std::size_t& bytes)
{
    os << name << " : items=" << vec.size() << ", bytes=" << (vec.size() * sizeof(vec[0])) << "\n";
    items += vec.size();
    bytes += vec.size() * sizeof(vec[0]);
}

void Tarraysep::dump_stats(std::ostream& os) const
{
    std::size_t total_items = 0;
    std::size_t total_bytes = 0;
    os << "TarraySep Stats:\n";
    vec_stats(os, bases , "base ", total_items, total_bytes);
    vec_stats(os, checks, "check", total_items, total_bytes);
    vec_stats(os, nexts , "next ", total_items, total_bytes);
    os << "total items=" << total_items << ", total bytes=" << total_bytes << "\n";
}
