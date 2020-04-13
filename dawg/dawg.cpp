#include "dawg.h"
#include <cassert>
#include <cstddef>
#include <climits>

#define AsIdx(x) static_cast<std::size_t>(x)

constexpr int MISSING_BASE = 0;
constexpr int UNSET_BASE = -1;
constexpr int UNSET_CHCK = -1;
constexpr int UNSET_TERM = -1;


// TODO: make table based
int iconv(char c) {
    int result;
    if ('a' <= c && c <= 'z') {
        result = c - 'a';
    } else if ('A' <= c && c <= 'Z') {
        result = c - 'A';
    } else {
        assert(0 && "invalid character");
        result = -1; // should cause an error
    }
    assert(0 <= result && result < 26);
    return result;
}

[[maybe_unused]] static int getbase2(const Datrie2* t, int index)
{
    assert(index >= 0);
    auto s = static_cast<std::size_t>(index);
    return s < t->base.size() ? t->base[s] : MISSING_BASE;
}

[[maybe_unused]] static int getchck2(const Datrie2* t, int index)
{
    assert(index >= 0);
    auto s = static_cast<std::size_t>(index);
    return s < t->chck.size() ? t->chck[s] : UNSET_CHCK;
}

[[maybe_unused]] static bool getterm2(const Datrie2* t, int index)
{
    assert(index >= 0);
    auto s = static_cast<std::size_t>(index);
    return s < t->term.size() ? t->term[s] != 0 : false;
}

[[maybe_unused]] static void setbase2(Datrie2* t, int index, int base, bool term)
{
    assert(index >= 0);
    auto s = static_cast<std::size_t>(index);
    assert(s < t->base.size());
    assert(s < t->term.size());
    t->base[s] = base;
    t->term[s] = term;
}

[[maybe_unused]] static void setchck2(Datrie2* t, int index, int base)
{
    assert(index >= 0);
    auto s = static_cast<std::size_t>(index);
    assert(s < t->chck.size());
    t->chck[s] = base;
}

bool init2([[maybe_unused]] Datrie2* t)
{
    const std::size_t N = 10000; // TODO: fix me

    t->base = std::vector<int>(N, 0);
    t->chck = std::vector<int>(N, 0);
    t->term = std::vector<int>(N, 0);
    assert(t->base.size() == N);
    assert(t->chck.size() == N);
    assert(t->term.size() == N);

    std::fill(std::begin(t->base), std::end(t->base), UNSET_BASE);
    std::fill(std::begin(t->chck), std::end(t->chck), UNSET_CHCK);
    std::fill(std::begin(t->term), std::end(t->term), UNSET_TERM);

    t->base[0] = 0;

    return true;
}

int findbase(const int* const first, const int* const last, int c, int value)
{
    for (const int* p = first; p != last; ++p) {
        if (p[c] == value) {
            return static_cast<int>(p - first);
        }
    }
    return -1;
}

bool insert2([[maybe_unused]] Datrie2* dt, [[maybe_unused]] const char* const word)
{
#if 0
                     dt->chck[0] = 0;
    dt->base[0] = 0; dt->chck[1] = 0;
    dt->base[1] = 1; dt->chck[2] = 1;
    dt->base[2] = 2;
    dt->term[2] = true;
#endif


    auto& chck = dt->chck;
    int s = 0;
    for (const char* p = word; *p != '\0'; ++p) {
        const char ch  = *p;
        const int  c   = iconv(ch) + 1;
        const int  t   = getbase2(dt, s) + c;
        const int  chk = getchck2(dt, t);

        if (chk != s) {
            assert(AsIdx(t) < chck.size()); // TODO: handle resizing
            [[maybe_unused]] int installed[26];
            int n_installed = 0;
            for (int c2 = 1; c2 <= 27; ++c2) {
                if (getchck2(dt, getbase2(dt, s) + c2) == s) {
                    installed[n_installed++] = c2;
                }
            }
            if (n_installed > 0) {
                assert(getchck2(dt, t) == UNSET_CHCK); // TODO: implement relocate
                setchck2(dt, t, s);
                s = t;
            } else {
                const std::size_t chck_end = chck.size() - AsIdx(c);
                const int new_base = findbase(&chck[0], &chck[chck_end], c, UNSET_CHCK);
                assert(new_base != -1); // TODO: implement resizing
                setbase2(dt, s, new_base, false);
                setchck2(dt, new_base + c, s);
                s = new_base + c;
            }
        } else {
            s = t;
        }
    }
    return true;
}

bool isword2(Datrie2* dt, const char* const word)
{
    int s = 0;
    for (const char* p = word; *p != '\0'; ++p) {
        const char ch = *p;
        const int  c  = iconv(ch) + 1;
        const int  t  = getbase2(dt, s) + c;
        if (getchck2(dt, t) != s) {
            return false;
        }
        s = t;
    }
    return getterm2(dt, s);
}

Letters childs2(Datrie2* dt, const char* const prefix)
{
    Letters result;
    result.n_children = 0;
    int s = 0;
    for (const char* p = prefix; *p != '\0'; ++p) {
        const char ch = *p;
        const int  c  = iconv(ch) + 1;
        const int  t  = getbase2(dt, s) + c;
        if (getchck2(dt, t) != s) {
            return result;
        }
        s = t;
    }
    for (int ch = 0; ch < 26; ++ch) {
        const int c = ch + 1;
        const int t = getbase2(dt, s) + c;
        if (getchck2(dt, t) == s) {
            result.children[result.n_children++] = ch;
        }
    }
    return result;
}
