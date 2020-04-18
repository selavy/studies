#include "tarray.h"
#include <cassert>
#include <cstddef>
#include <fstream>
#include "iconv.h"
#include "tarray_generated.h"
#include "tarray_util.h"


bool Tarray::isword(const char* const word) const noexcept
{
    int s = 0;
    for (const char* p = word; *p != '\0'; ++p) {
        const char ch = *p;
        const int c = iconv(ch) + 1;
        const int t = base(s) + c;
        const auto [check, next] = xtn(t);
        if (check != s) {
            return false;
        }
        s = next;
    }
    return term(s);
}

int Tarray::base(int index) const noexcept
{
    auto s = static_cast<std::size_t>(index);
    return s < bases.size() ? static_cast<int>(bases[s]) >> 1 : NO_BASE;
}

Tarray::Xtn Tarray::xtn(int index) const noexcept
{
    auto s = static_cast<std::size_t>(index);
    return s < xtns.size() ? xtns[s] : Xtn{UNSET_CHECK, 0};
}

int Tarray::term(int index) const noexcept
{
    auto s = static_cast<std::size_t>(index);
    return s < bases.size() ? (bases[s] & 0x1u) != 0 : false;
}

void Tarray::setbase(std::size_t n, int val, bool term) noexcept
{
    u32 uval  = static_cast<u32>(val);
    u32 uterm = static_cast<u32>(term);
    bases[n] = (uval << 1) | (uterm & 0x1u);
}

std::optional<Tarray> Tarray::deserialize(const std::string& filename)
{
    auto buf = read_dict_file(filename);
    auto serial_tarray = GetSerialTarray(buf.data());
    flatbuffers::Verifier v(reinterpret_cast<const uint8_t*>(buf.data()), buf.size());
    assert(serial_tarray->Verify(v));
    auto* bases  = serial_tarray->bases();
    auto* checks = serial_tarray->checks();
    auto* nexts  = serial_tarray->nexts();
    return Tarray::make(bases->begin(), bases->end(), checks->begin(), checks->end(), nexts->begin(), nexts->end());
}

template <class Cont>
void vec_stats(std::ostream& os, Cont& vec, std::string name, std::size_t& items, std::size_t& bytes)
{
    os << name << " : items=" << vec.size() << ", bytes=" << (vec.size() * sizeof(vec[0])) << "\n";
    items += vec.size();
    bytes += vec.size() * sizeof(vec[0]);
}

void Tarray::dump_stats(std::ostream& os) const
{
    std::size_t total_items = 0;
    std::size_t total_bytes = 0;
    os << "Tarray Stats:\n";
    vec_stats(os, bases, "base ", total_items, total_bytes);
    vec_stats(os, xtns , "xtns", total_items, total_bytes);
    os << "total items=" << total_items << ", total bytes=" << total_bytes << "\n";
}
