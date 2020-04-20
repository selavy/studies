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
    : bases (1000, UNSET_BASE )
    , checks(1000, UNSET_CHECK) // should it be initialized to 0?
{
    bases[0] = 0;
    static_assert((UNSET_BASE   & TERM_MASK) == 0, "unset base must not have terminal bit set");
    static_assert((MISSING_BASE & TERM_MASK) == 0, "missing base must not have terminal bit set");
    static_assert((MISSING_BASE + static_cast<u32>(MAX_CHILD_OFFSET)) < static_cast<u32>(INT_MAX),
            "adding max child offset would overflow missing base");
    static_assert((UNSET_BASE + static_cast<u32>(MAX_CHILD_OFFSET)) < static_cast<u32>(INT_MAX),
            "adding max child offset would overflow missing base");
}

void Darray3::trim()
{
    while (checks.size() >= 27 && checks.back() == UNSET_CHECK) {
        bases .pop_back();
        checks.pop_back();
    }
    bases .shrink_to_fit();
    checks.shrink_to_fit();
}

int Darray3::base(int index) const
{
    assert(index >= 0);
    auto s = static_cast<std::size_t>(index);
    if (s < bases.size()) {
        return static_cast<int>(bases[s] & BASE_MASK);
    } else {
        return MISSING_BASE;
    }
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

void Darray3::setbase(int index, int val)
{
    assert(index >= 0);
    assert(val  >= 0);
    assert(static_cast<u32>(val) < MAX_BASE);
    auto s = static_cast<std::size_t>(index);
    assert(s < bases.size());
    assert((UNSET_BASE & TERM_MASK) == 0);
    bases[s] = (bases[s] & TERM_MASK) | static_cast<u32>(val);
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
    bases[s] |= (bit << TERM_BIT);
}

void Darray3::clrbase(int index)
{
    assert(index >= 0);
    auto s = static_cast<std::size_t>(index);
    assert(s < bases.size());
    bases[s] = UNSET_BASE;
}

void Darray3::clrcheck(int index)
{
    assert(index >= 0);
    auto s = static_cast<std::size_t>(index);
    assert(s < checks.size());
    checks[s] = UNSET_CHECK;
}

void Darray3::clrterm(int index)
{
    assert(index >= 0);
    auto s = static_cast<std::size_t>(index);
    assert(s < checks.size());
    bases[s] &= ~TERM_MASK;
}

int Darray3::countchildren(int s, int* children) const
{
    int n_children = 0;
    for (int c = 1; c <= 27; ++c) {
        if (check(base(s) + c) == s) {
            children[n_children++] = c;
        }
    }
    return n_children;
}

void Darray3::relocate(int s, int b, int* childs, int n_childs)
{
    // TODO: remove
    for (int i = 0; i < n_childs; ++i) {
        assert(1 <= childs[i] && childs[i] <= 27);
        const int c = childs[i];
        const int t_old = base(s) + c;
        const int t_new = b + c;
        assert(check(t_old) == s);
        setcheck(t_new, s);
        setbase(t_new, base(t_old));
        setterm(t_new, term(t_old));
        // update grand children
        for (int d = 1; d <= 27; ++d) {
            if (check(base(t_old) + d) == t_old) {
                setcheck(base(t_old) + d, t_new);
            }
        }
        clrcheck(t_old);
        clrbase(t_old);  // TODO(peter): remove -- just for debugging
        clrterm(t_old);  // TODO(peter): remove -- just for debugging
    }
    setbase(s, b);
}

void Darray3::insert(const char* const word)
{
    auto extendarrays = [this](std::size_t need)
    {
        this->bases.insert (this->bases.end() , need, UNSET_BASE );
        this->checks.insert(this->checks.end(), need, UNSET_CHECK);
    };

    auto findbase = [&](const int* const first, const int* const last, int c)
    {
        for (;;) {
            for (const int* p = first; p != last; ++p) {
                if (p[c] == UNSET_CHECK) {
                    return static_cast<int>(p - first);
                }
            }
            extendarrays(50);
        }
    };

    auto findnewbase = [&](const int* children, int n_children)
    {
        auto baseok = [](const int* const checks, const int* children, int n_children)
        {
            for (int i = 0; i < n_children; ++i) {
                assert(1 <= children[i] && children[i] <= 27);
                if (checks[children[i]] != UNSET_CHECK) {
                    return false;
                }
            }
            return true;
        };

        for (;;) {
            for (auto it = checks.begin(), endit = checks.end(); it != endit; ++it) {
                if (baseok(&*it, &children[0], n_children)) {
                    return static_cast<int>(std::distance(checks.begin(), it));
                }
            }
            extendarrays(50);
        }
    };

    int children[26];
    int s = 0;
    for (const char* p = word; *p != '\0'; ++p) {
        const int c = sconv(*p);
        const int t = base(s) + c;
        if (check(t) == s) {
            s = t;
            continue;
        }

        // TODO: Can I mark the current branch as a child? Not sure if the logic will work
        //       trying to move an uninstall node.
        int n_children = countchildren(s, &children[0]);
        if (n_children > 0) {
            if (AsIdx(t) < checks.size() && check(t) == UNSET_CHECK) { // slot is available
                setcheck(t, s);
                s = t;
            } else {
                children[n_children++] = c;
                std::size_t start = 0;
                const int b_new = findnewbase(&children[0], n_children);
                assert(0 <= (b_new + c) && AsIdx(b_new + c) < checks.size());
                assert(checks[AsIdx(b_new + c)] == UNSET_CHECK);
                relocate(s, b_new, &children[0], n_children - 1);
                setcheck(b_new + c, s);
                s = b_new + c;
            }
        } else {
            std::size_t start = 0;
            const int b_new = findbase(&*checks.begin(), &*checks.end(), c);
            assert(0 <= (b_new + c) && AsIdx(b_new + c) < checks.size());
            assert(checks[AsIdx(b_new + c)] == UNSET_CHECK);
            setbase(s, b_new);
            setcheck(b_new + c, s);
            s = b_new + c;
        }
    }

    setterm(s, true);
}

bool Darray3::isword(const char* const word) const
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
