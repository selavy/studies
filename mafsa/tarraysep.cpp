#include "tarraysep.h"
#include <cassert>
#include <cstddef>
#include <fstream>
#include "iconv.h"
#include "tarray_generated.h"


bool Tarraysep::isword(const char* const word) const
{
    int s = 0;
    for (const char* p = word; *p != '\0'; ++p) {
        const char ch = *p;
        const int  c  = iconv(ch) + 1;
        const int  t  = base(s) + c;
        if (t < 0 || check(t) != s) {
            return false;
        }
        s = nexts[static_cast<std::size_t>(t)];
    }
    return term(s);
}

int Tarraysep::base(int index) const
{
    assert(index >= 0);
    auto s = static_cast<std::size_t>(index);
    return s < bases.size() ? static_cast<int>(bases[s]) >> 1 : NO_BASE;
}

int Tarraysep::check(int index) const
{
    assert(index >= 0);
    auto s = static_cast<std::size_t>(index);
    return s < checks.size() ? checks[s] : UNSET_CHECK;
}

int Tarraysep::term(int index) const
{
    assert(index >= 0);
    auto s = static_cast<std::size_t>(index);
    return s < bases.size() ? (bases[s] & 0x1u) != 0 : false;
}

int Tarraysep::next(int index) const
{
    assert(index >= 0);
    auto s = static_cast<std::size_t>(index);
    return s < nexts.size() ? nexts[s] : 0;
}

void Tarraysep::setbase(std::size_t n, int val, bool term)
{
    u32 uval  = static_cast<u32>(val);
    u32 uterm = static_cast<u32>(term);
    bases[n] = (uval << 1) | (uterm & 0x1u);
}

std::optional<Tarraysep> Tarraysep::deserialize(const std::string& filename)
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

    Tarraysep tarray;
    auto* bases  = serial_tarray->bases();
    auto* checks = serial_tarray->checks();
    auto* nexts  = serial_tarray->nexts();
    tarray.bases .assign(bases ->begin(), bases ->end());
    tarray.checks.assign(checks->begin(), checks->end());
    tarray.nexts .assign(nexts ->begin(), nexts ->end());
    return tarray;
}
