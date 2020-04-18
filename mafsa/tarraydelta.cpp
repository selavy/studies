#include "tarraydelta.h"
#include <cassert>
#include <cstddef>
#include <fstream>
#include "iconv.h"
#include "tarray_generated.h"

TarrayDelta::TarrayDelta(std::size_t n_states) noexcept
    : bases (n_states, UNSET_BASE)
    , checks(n_states, UNSET_CHECK)
    , nexts (n_states, UNSET_NEXT)
{
}

bool TarrayDelta::isword(const char* const word) const noexcept
{
    int s = 0;
    for (const char* p = word; *p != '\0'; ++p) {
        const char ch = *p;
        const int c = iconv(ch) + 1;
        const int t = base(s) + c;
        if (t < 0 || check(t) != s) {
            return false;
        }
        s += nexts[static_cast<std::size_t>(t)];
    }
    return term(s);
}

int TarrayDelta::base(int index) const noexcept
{
    assert(index >= 0);
    auto s = static_cast<std::size_t>(index);
    return s < bases.size() ? static_cast<int>(bases[s]) >> 1 : NO_BASE;
}

int TarrayDelta::check(int index) const noexcept
{
    assert(index >= 0);
    auto s = static_cast<std::size_t>(index);
    return s < checks.size() ? checks[s] : UNSET_CHECK;
}

int TarrayDelta::term(int index) const noexcept
{
    assert(index >= 0);
    auto s = static_cast<std::size_t>(index);
    return s < bases.size() ? (bases[s] & 0x1u) != 0 : false;
}

void TarrayDelta::setbase(std::size_t n, int val, bool term) noexcept
{
    u32 uval  = static_cast<u32>(val);
    u32 uterm = static_cast<u32>(term);
    bases[n] = (uval << 1) | (uterm & 0x1u);
}

std::optional<TarrayDelta> TarrayDelta::deserialize(const std::string& filename)
{
    std::ifstream infile;
    infile.open(filename, std::ios::binary);
    infile.seekg(0, std::ios::end);
    int length = infile.tellg();
    infile.seekg(0, std::ios::beg);
    std::vector<char> data(length);
    infile.read(data.data(), length);
    infile.close();

    auto serial_tarray = GetSerialTarray(data.data());
    flatbuffers::Verifier v((const uint8_t*)data.data(), data.size());
    assert(serial_tarray->Verify(v));

    TarrayDelta tarray;
    auto* bases  = serial_tarray->bases();
    auto* checks = serial_tarray->checks();
    auto* nexts  = serial_tarray->nexts();
    tarray.bases .assign(bases ->begin(), bases ->end());
    tarray.checks.assign(checks->begin(), checks->end());
    tarray.nexts.reserve(nexts->size());
    for (std::size_t i = 0; i < nexts->size(); ++i) {
        const int delta = (*nexts)[i] - (*checks)[i];
        if (!(INT16_MIN <= delta && delta <= INT16_MAX)) {
            printf("warning: delta=%u next=%d check=%d\n", delta, (*nexts)[i], (*checks)[i]);
            throw std::runtime_error("Unable to delta encode tarray");
        }
        assert(INT16_MIN <= delta && delta <= INT16_MAX);
        tarray.nexts[i] = static_cast<s16>(delta);
    }

    return tarray;
}

template <class T, class Alloc>
void vec_stats(std::ostream& os, const std::vector<T, Alloc>& vec, std::string name, std::size_t& items, std::size_t& bytes)
{
    os << name << " : items=" << vec.size() << ", bytes=" << (vec.size() * sizeof(vec[0])) << "\n";
    items += vec.size();
    bytes += vec.size() * sizeof(vec[0]);
}

void TarrayDelta::dump_stats(std::ostream& os) const
{
    std::size_t total_items = 0;
    std::size_t total_bytes = 0;
    os << "TarrayDelta Stats:\n";
    vec_stats(os, bases , "base ", total_items, total_bytes);
    vec_stats(os, checks, "check", total_items, total_bytes);
    vec_stats(os, nexts , "next ", total_items, total_bytes);
    os << "total items=" << total_items << ", total bytes=" << total_bytes << "\n";
}
