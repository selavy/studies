#include "darray2.h"
#include <climits>
#include <cstddef>
#include <cassert>
#include <fstream>
#include <iostream>
#include <algorithm>
#include "iconv.h"
// #include "darray_generated.h"
#include "tarray_util.h"


// TODO: remove
#define AsIdx(x) static_cast<std::size_t>(x)

Darray2::Darray2()
    : bases (1000, UNSET_BASE )
    , checks(1000, UNSET_CHECK)
{}

int Darray2::base(int index) const
{
    auto n = static_cast<std::size_t>(index);
    return n < bases.size() ? static_cast<int>(bases[n]) >> 1 : MAX_BASE;
}

int Darray2::check(int index) const
{
    auto n = static_cast<std::size_t>(index);
    return n < checks.size() ? checks[n] : UNSET_CHECK;
}

bool Darray2::term(int index) const
{
    auto n = static_cast<std::size_t>(index);
    return n < bases.size() ? (bases[n] & 0x1u) != 0 : false;
}

void Darray2::setbase(int index, int val, bool term)
{
    assert(0 <= index);
    auto n = static_cast<std::size_t>(index);
    u32 uval  = static_cast<u32>(val);
    u32 uterm = static_cast<u32>(term);
    assert(n < bases.size());
    bases[n] = (uval << 1) | (uterm & 0x1u);
}

void Darray2::setbase(int index, int val)
{
    assert(0 <= index);
    auto n = static_cast<std::size_t>(index);
    u32 uval  = static_cast<u32>(val);
    assert(n < bases.size());
    bases[n] = (uval << 1) | (bases[n] & 0x1u);
}

void Darray2::setcheck(int index, int val)
{
    assert(0 <= index);
    auto n = static_cast<std::size_t>(index);
    assert(n < checks.size());
    checks[n] = val;
}

void Darray2::setterm(int index, bool term)
{
    assert(0 <= index);
    auto n = static_cast<std::size_t>(index);
    u32 uterm = static_cast<u32>(term);
    assert(n < bases.size());
    bases[n] = (bases[n] & ~0x1u) | (uterm & 0x1u);
}

void Darray2::clrbase(int index)
{
    assert(0 <= index);
    auto n = static_cast<std::size_t>(index);
    assert(n < bases.size());
    bases[n] = UNSET_BASE;
}

void Darray2::clrcheck(int index)
{
    assert(0 <= index);
    auto n = static_cast<std::size_t>(index);
    assert(n < checks.size());
    checks[n] = UNSET_CHECK;
}

int Darray2::countchildren(int s, int* children) const
{
    int n_children = 0;
    for (int c = 1; c <= 27; ++c) {
        if (check(base(s) + c) == s) {
            children[n_children++] = c;
        }
    }
    return n_children;
}

int Darray2::findbaserange(const int* const first, const int* const last, const int* const cs, const int* const csend)
{
    auto baseworks = [](const int* const check, const int* const cs, const int* const csend)
    {
        for (const int* c = cs; c != csend; ++c) {
            assert(1 <= *c && *c <= 27);
            if (check[*c] != UNSET_CHECK) {
                return false;
            }
        }
        return true;
    };

    for (const int* chck = first; chck != last; ++chck) {
        if (baseworks(chck, cs, csend)) {
            return static_cast<int>(chck - first);
        }
    }
    return -1;
}

void Darray2::relocate(int s, int b, int* childs, int n_childs)
{
    for (int i = 0; i < n_childs; ++i) {
        assert(1 <= childs[i] && childs[i] <= 27);
        const int c = childs[i];
        const int t_old = base(s) + c;
        const int t_new = b + c;
        assert(check(t_old) == s);
        setcheck(t_new, s);
        setbase(t_new, base(t_old), term(t_old));
        // update grand children
        for (int d = 1; d <= 27; ++d) {
            if (check(base(t_old) + d) == t_old) {
                setcheck(base(t_old) + d, t_new);
            }
        }
        clrcheck(t_old);
        clrbase(t_old);  // TODO(peter): remove -- just for debugging
    }
    setbase(s, b/*, term(s)*/);
}

void Darray2::insert(const char* const word)
{
    auto extendarrays = [this](std::size_t need)
    {
        this->bases.insert (this->bases.end() , need, UNSET_BASE );
        this->checks.insert(this->checks.end(), need, UNSET_CHECK);
    };

    auto findbase = [&](int c) -> int
    {
        for (;;) {
            // TODO: be smarter about way to start search from on second try
            for (std::size_t i = 0, n = checks.size() - static_cast<std::size_t>(c); i < n; ++i) {
                if (checks[i+c] == UNSET_CHECK) {
                    return static_cast<int>(i); //  - c;
                }
            }
            extendarrays(50);
        }
    };

    int childs[26];
    int s = 0;
    for (const char* p = word; *p != '\0'; ++p) {
        const char ch = *p;
        const int  c  = sconv(ch);
        const int  t  = base(s) + c;
        if (check(t) == s) {
            s = t;
            continue;
        }

        // TODO: Can I mark the current branch as a child? Not sure if the logic will work
        //       trying to move an uninstall node.
        int n_childs = countchildren(s, &childs[0]);
        if (n_childs > 0) {
            if (AsIdx(t) < checks.size() && check(t) == UNSET_CHECK) { // slot is available
                setcheck(t, s);
                s = t;
            } else {
                childs[n_childs++] = c;
                std::size_t start = 0;
                int b_new;
                for (;;) {
                    const std::size_t maxc = AsIdx(childs[n_childs - 1]);
                    const std::size_t last = checks.size() - maxc;
                    assert(checks.size() > maxc);
                    b_new = findbaserange(&checks[start], &checks[last], &childs[0], &childs[n_childs]);
                    if (b_new >= 0) {
                        b_new = b_new + static_cast<int>(start);
                        break;
                    }
                    const std::size_t lookback = 26;
                    start = checks.size() > lookback ? checks.size() - lookback : 0;
                    extendarrays(50);
                }
                assert(0 <= b_new && AsIdx(b_new) < checks.size());
                --n_childs;
                relocate(s, b_new, &childs[0], n_childs);
                setcheck(b_new + c, s);
                s = b_new + c;
            }
        } else {
            std::size_t start = 0;
            int b_new = findbase(c);
            assert(0 <= (b_new + c) && (b_new + c) < checks.size());
            setbase(s, b_new/*, term(s)*/);
            setcheck(b_new + c, s);
            s = b_new + c;
        }
    }

    setterm(s, true);
}

bool Darray2::isword(const char* const word) const
{
    int s = 0;
    for (const char* p = word; *p != '\0'; ++p) {
        const int c = sconv(*p);
        const int t = base(s) + c;
        if (check(t) != s) {
            return false;
        }
        s = t;
    }
    return term(s);
}

#if 0
std::optional<Darray2> Darray2::deserialize(const std::string& filename)
{
    auto buf = read_dict_file(filename);
    auto serial_darray = GetSerialDarray2(buf.data());
    flatbuffers::Verifier v(reinterpret_cast<const uint8_t*>(buf.data()), buf.size());
    assert(serial_darray->Verify(v));
    Darray2 darray;
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

void Darray2::dump_stats(std::ostream& os) const
{
    std::size_t total_items = 0;
    std::size_t total_bytes = 0;
    os << "Darray2 Stats:\n";
    vec_stats(os, bases , "base ", total_items, total_bytes);
    vec_stats(os, checks, "check", total_items, total_bytes);
    os << "total items=" << total_items << ", total bytes=" << total_bytes << "\n";
}

void Darray2::dumpstate() const
{
    const int N = 20;
    const char* FMT = " %2d";

    printf("INDEX:  |");
    for (int i = 0; i < N; ++i) {
        printf(FMT, i);
    }
    printf("\n");

    printf("BASE :  |");
    for (int i = 0; i < N; ++i) {
        printf(FMT, base(i));
    }
    printf("\n");

    printf("CHECK:  |");
    for (int i = 0; i < N; ++i) {
        printf(FMT, check(i));
    }
    printf("\n");

    printf("TERM :  |");
    for (int i = 0; i < N; ++i) {
        printf(FMT, term(i)?1:0);
    }
    printf("\n");
}
