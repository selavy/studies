#include "dawg.h"
#include <cassert>
#include <cstddef>
#include <climits>
#include <cstdio> // TEMP

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

[[maybe_unused]] static void clrbase2(Datrie2* t, int index)
{
    assert(index >= 0);
    auto s = static_cast<std::size_t>(index);
    assert(s < t->base.size());
    assert(s < t->term.size());
    t->base[s] = UNSET_BASE;
    t->term[s] = UNSET_TERM;
}

[[maybe_unused]] static void setchck2(Datrie2* t, int index, int base)
{
    assert(index >= 0);
    auto s = static_cast<std::size_t>(index);
    assert(s < t->chck.size());
    t->chck[s] = base;
}

[[maybe_unused]] static void clrchck2(Datrie2* t, int index)
{
    assert(index >= 0);
    auto s = static_cast<std::size_t>(index);
    assert(s < t->chck.size());
    t->chck[s] = UNSET_CHCK;
}

[[maybe_unused]] static void setterm2(Datrie2* t, int index, bool term)
{
    assert(index >= 0);
    auto s = static_cast<std::size_t>(index);
    assert(s < t->chck.size());
    t->term[s] = term;
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

int baseworks(const int* const base, const int* const cs, const int* const csend, int value)
{
    for (const int* c = cs; c != csend; ++c) {
        assert(1 <= *c && *c <= 27);
        if (base[*c] != value) {
            return false;
        }
    }
    return true;
}

// TODO: replace with free list
int findbaserange(
        const int* const first,
        const int* const last,
        const int* const cs,
        const int* const csend,
        int value)
{
    for (const int* chck = first; chck != last; ++chck) {
        if (baseworks(chck, cs, csend, value)) {
            return static_cast<int>(chck - first);
        }
    }
    return -1;
}

int cntchilds(Datrie2* dt, int s, int* childs)
{
    int n_childs = 0;
    if (getbase2(dt, s) != UNSET_BASE) {
        for (int c = 1; c <= 27; ++c) {
            if (getchck2(dt, getbase2(dt, s) + c) == s) {
                childs[n_childs++] = c;
            }
        }
    }
    return n_childs;
}

bool insert2([[maybe_unused]] Datrie2* dt, [[maybe_unused]] const char* const word)
{
    int childs[26];
    auto& chck = dt->chck;
    int s = 0;
    for (const char* p = word; *p != '\0'; ++p) {
        const char ch  = *p;
        const int  c   = iconv(ch) + 1;
        const int  t   = getbase2(dt, s) + c;

        if (getchck2(dt, t) != s) {
            assert(AsIdx(t) < chck.size()); // TODO: handle resizing

            // TODO: Can I mark the current branch as a child? Not sure if the logic will work
            //       trying to move an uninstall node.
            const int n_childs = cntchilds(dt, s, &childs[0]);
            if (n_childs > 0) {
                if (getchck2(dt, t) == UNSET_CHCK) { // slot is available
                    setchck2(dt, t, s);
                    s = t;
                } else {
                    printf("Inserting new link for '%s' on character %c (%zu) move %d children: ", word, ch, p - word, n_childs);
                    printf("[ ");
                    for (int i = 0; i < n_childs; ++i) {
                        printf("%c ", static_cast<char>(childs[i] - 1 + 'A'));
                    }
                    printf("]\n");

                    const std::size_t maxc = AsIdx(childs[n_childs - 1]);
                    assert(chck.size() > maxc);
                    const std::size_t last = chck.size() - maxc;
                    int new_base = findbaserange(&chck[0], &chck[last], &childs[0], &childs[n_childs], UNSET_CHCK);
                    assert(new_base >= 0);
                    for (int i = 0; i < n_childs; ++i) {
                        const int c2 = childs[i];
                        const int old_t = getbase2(dt, s) + c2;
                        const int new_t = new_base + c2;
                        setbase2(dt, new_t, getbase2(dt, old_t), getterm2(dt, old_t));
                        setchck2(dt, new_t, s);
                        clrbase2(dt, old_t);
                        clrchck2(dt, old_t);
                    }
                    setbase2(dt, s, new_base, false);
                    setchck2(dt, new_base + c, s);
                    s = new_base + c;
                }
            } else {
                const std::size_t chck_end = chck.size() - AsIdx(c);
                const int new_base = findbase(&chck[0], &chck[chck_end], c, UNSET_CHCK);
                assert(new_base != -1); // TODO: implement resizing
                // assert(getterm2(dt, s) == false);
                setbase2(dt, s, new_base, false);
                setchck2(dt, new_base + c, s);
                s = new_base + c;
            }
        } else {
            s = t;
        }
    }
    setterm2(dt, s, true);
    return true;
}

Tristate isword2(Datrie2* dt, const char* const word)
{
    int s = 0;
    for (const char* p = word; *p != '\0'; ++p) {
        const char ch = *p;
        const int  c  = iconv(ch) + 1;
        const int  t  = getbase2(dt, s) + c;
        if (getchck2(dt, t) != s) {
            // return false;
            return Tristate::eNoLink;
        }
        s = t;
    }
    return getterm2(dt, s) ? Tristate::eWord : Tristate::eNotTerm;
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
