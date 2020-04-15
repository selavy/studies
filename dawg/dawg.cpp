#include "dawg.h"
#include <cassert>
#include <cstddef>
#include <climits>
#include <cstdio> // TEMP

#define AsIdx(x) static_cast<std::size_t>(x)

using u32 = uint32_t;

constexpr int MIN_CHILD_OFFSET = 1;
constexpr int MAX_CHILD_OFFSET = 27;
constexpr int TERM_BIT     = 31;
constexpr u32 TERM_MASK    = 1u << TERM_BIT;
constexpr u32 BASE_MASK    = ~TERM_MASK;
constexpr u32 MAX_BASE     = (1u << 30) - MAX_CHILD_OFFSET; // exclusive
constexpr int MISSING_BASE = static_cast<int>(MAX_BASE);
constexpr u32 UNSET_BASE   =  0;
constexpr int UNSET_CHCK   = MAX_BASE;
constexpr int UNSET_TERM   =  0;

static_assert((UNSET_BASE   & TERM_MASK) == 0, "unset base must not have terminal bit set");
static_assert((MISSING_BASE & TERM_MASK) == 0, "missing base must not have terminal bit set");
static_assert((MISSING_BASE + static_cast<u32>(MAX_CHILD_OFFSET)) < static_cast<u32>(INT_MAX),
        "adding max child offset would overflow missing base");
static_assert((UNSET_BASE + static_cast<u32>(MAX_CHILD_OFFSET)) < static_cast<u32>(INT_MAX),
        "adding max child offset would overflow missing base");


#define DEBUG(fmt, ...) fprintf(stderr, "DEBUG: " fmt "\n", ##__VA_ARGS__);


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

static int getbase2(const Datrie2* t, int index)
{
    assert(index >= 0);
    auto s = static_cast<std::size_t>(index);
    if (s < t->base.size()) {
        return static_cast<int>(t->base[s] & BASE_MASK);
    } else {
        return MISSING_BASE;
    }
}

static int getchck2(const Datrie2* t, int index)
{
    assert(index >= 0);
    auto s = static_cast<std::size_t>(index);
    return s < t->chck.size() ? t->chck[s] : UNSET_CHCK;
}

static bool getterm2(const Datrie2* t, int index)
{
    assert(index >= 0);
    auto s = static_cast<std::size_t>(index);
    return s < t->base.size() ? (t->base[s] & TERM_MASK) != 0 : false;
}

static void setbase2(Datrie2* t, int index, int base)
{
    assert(index >= 0);
    assert(base  >= 0);
    assert(static_cast<u32>(base) < MAX_BASE);
    auto s = static_cast<std::size_t>(index);
    assert(s < t->base.size());
    assert((UNSET_BASE & TERM_MASK) == 0);
    t->base[s] = (t->base[s] & TERM_MASK) | static_cast<u32>(base);
}

static void setchck2(Datrie2* t, int index, int base)
{
    assert(index >= 0);
    auto s = static_cast<std::size_t>(index);
    assert(s < t->chck.size());
    // DEBUG("SETCHECK[%d] = %d", index, base);
    t->chck[s] = base;
}

static void setterm2(Datrie2* t, int index, bool term)
{
    assert(index >= 0);
    auto s = static_cast<std::size_t>(index);
    assert(s < t->chck.size());
    const u32 bit = term ? 1u : 0u;
    t->base[s] |= (bit << TERM_BIT);
}

static void clrbase2(Datrie2* t, int index)
{
    assert(index >= 0);
    auto s = static_cast<std::size_t>(index);
    assert(s < t->base.size());
    // DEBUG("CLRBASE[%d]", index);
    t->base[s] = UNSET_BASE;
}

static void clrchck2(Datrie2* t, int index)
{
    assert(index >= 0);
    auto s = static_cast<std::size_t>(index);
    assert(s < t->chck.size());
    // DEBUG("CLRCHECK[%d]", index);
    t->chck[s] = UNSET_CHCK;
}

static void clrterm2(Datrie2* t, int index)
{
    assert(index >= 0);
    auto s = static_cast<std::size_t>(index);
    assert(s < t->chck.size());
    // DEBUG("CLRTERM[%d]", index);
    t->base[s] &= ~TERM_MASK;
}

// static bool validbase2(int base)
// {
//     return 0 <= base && base < INT_MAX; // MAX_BASE;
// }

bool init2(Datrie2* t)
{
    // const std::size_t N = 1; // TODO: fix me
    const std::size_t N = 1000; // TODO: fix me

    t->base = std::vector<u32>(N, UNSET_BASE);
    t->chck = std::vector<int>(N, 0);
    assert(t->base.size() == N);
    assert(t->chck.size() == N);
    std::fill(std::begin(t->base), std::end(t->base), UNSET_BASE);
    std::fill(std::begin(t->chck), std::end(t->chck), UNSET_CHCK);
    t->base[0] = 0;
    return true;
}

static int findbase(const int* const first, const int* const last, int c)
{
    for (const int* p = first; p != last; ++p) {
        if (p[c] == UNSET_CHCK) {
            return static_cast<int>(p - first);
        }
    }
    return -1;
}

static int baseworks(const int* const chck, const int* const cs, const int* const csend)
{
    for (const int* c = cs; c != csend; ++c) {
        assert(1 <= *c && *c <= 27);
        if (chck[*c] != UNSET_CHCK) {
            return false;
        }
    }
    return true;
}

// TODO: replace with free list
static int findbaserange(const int* const first, const int* const last, const int* const cs, const int* const csend)
{
    for (const int* chck = first; chck != last; ++chck) {
        if (baseworks(chck, cs, csend)) {
            return static_cast<int>(chck - first);
        }
    }
    return -1;
}

static int cntchilds(Datrie2* dt, int s, int* childs)
{
    int n_childs = 0;
    for (int c = 1; c <= 27; ++c) {
        if (getchck2(dt, getbase2(dt, s) + c) == s) {
            childs[n_childs++] = c;
        }
    }
    return n_childs;
}

static void extendarrays(Datrie2* dt, std::size_t need)
{
    dt->chck.insert(dt->chck.end(), need, UNSET_CHCK);
    dt->base.insert(dt->base.end(), need, UNSET_CHCK);
}

void relocate2(Datrie2* dt, int s, int b, int* childs, int n_childs)
{
    // DEBUG("!!! relocating the base for state s=%d => base[s] from %d -> %d !!!", s, getbase2(dt, s), b);

    // TODO(peter): revisit -- warnings don't like parameter being named `s` saying it shadows...
    auto base  = [dt](int x) { return getbase2(dt, x); };
    auto term  = [dt](int x) { return getterm2(dt, x); };
    auto check = [dt](int x) { return getchck2(dt, x); };
    for (int i = 0; i < n_childs; ++i) {
        assert(1 <= childs[i] && childs[i] <= 27);
        const int c = childs[i];
        const int t_old = base(s) + c;
        const int t_new = b + c;
        assert(check(t_old) == s);
        setchck2(dt, t_new, s);
        setbase2(dt, t_new, base(t_old));
        setterm2(dt, t_new, term(t_old));
        // update grand children
        for (int d = 1; d <= 27; ++d) {
            if (check(base(t_old) + d) == t_old) {
                setchck2(dt, base(t_old) + d, t_new);
            }
        }
        clrchck2(dt, t_old);
        clrbase2(dt, t_old);  // TODO(peter): remove -- just for debugging
        clrterm2(dt, t_old);  // TODO(peter): remove -- just for debugging
    }
    setbase2(dt, s, b);

    // DEBUG("Finished relocate state %d to base=%d", s, b);
}

// TODO(peter): revisit: could have insert2 return false (or switch to error code) if it
//              is unable to locate a base and the end user has to do the resize, then
//              add a "insertex2" that will do the resizing and re-call insert2 if needed.
bool insert2(Datrie2* dt, const char* const word)
{
    // TODO(peter): revisit -- warnings don't like parameter being named `s` saying it shadows...
    auto check = [dt](int x) { return getchck2(dt, x); };

    int childs[26];
    auto& chck = dt->chck;
    int s = 0;
    for (const char* p = word; *p != '\0'; ++p) {
        const char ch  = *p;
        const int  c   = iconv(ch) + 1;
        const int  t   = getbase2(dt, s) + c;
        if (check(t) == s) {
            s = t;
            continue;
        }

        // TODO: Can I mark the current branch as a child? Not sure if the logic will work
        //       trying to move an uninstall node.
        int n_childs = cntchilds(dt, s, &childs[0]);
        if (n_childs > 0) {
            if (AsIdx(t) < dt->chck.size() && check(t) == UNSET_CHCK) { // slot is available
                setchck2(dt, t, s);
                s = t;
            } else {
                childs[n_childs++] = c;
                std::size_t start = 0;
                int b_new;
                for (;;) {
                    const std::size_t maxc = AsIdx(childs[n_childs - 1]);
                    const std::size_t last = chck.size() - maxc;
                    assert(chck.size() > maxc);
                    b_new = findbaserange(&chck[start], &chck[last], &childs[0], &childs[n_childs]);
                    if (b_new >= 0) {
                        b_new = b_new + static_cast<int>(start);
                        break;
                    }
                    const std::size_t lookback = 26;
                    start = dt->chck.size() > lookback ? dt->chck.size() - lookback : 0;
                    extendarrays(dt, 50);
                }
                assert(0 <= b_new && AsIdx(b_new) < dt->chck.size());

                --n_childs;
                relocate2(dt, s, b_new, &childs[0], n_childs);
                setchck2(dt, b_new + c, s);
                s = b_new + c;
            }
        } else {
            std::size_t start = 0;
            int b_new;
            for (;;) {
                const std::size_t chck_end = chck.size() - AsIdx(c);
                b_new = findbase(&chck[start], &chck[chck_end], c);
                if (b_new >= 0) {
                    b_new = b_new + static_cast<int>(start);
                    break;
                }
                start = dt->chck.size();
                extendarrays(dt, 50);
            }
            assert(0 <= b_new && AsIdx(b_new) < dt->chck.size()); // TODO: implement resizing
            // DEBUG("\tSET ch=%c s=%d b_new=%d c=%d check[%d]=%d", ch, s, b_new, c, b_new + c, s);
            setbase2(dt, s, b_new);
            setchck2(dt, b_new + c, s);
            s = b_new + c;
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

void trim2( Datrie2* dt)
{
    assert(dt->chck.size() == dt->base.size());
    int i = static_cast<int>(dt->chck.size());
    while (i-- > 10) {
        std::size_t ii = AsIdx(i);
        if (dt->chck[ii] != UNSET_CHCK) {
            break;
        }
    }
    ++i;
    dt->chck.erase(dt->chck.cbegin() + i, dt->chck.cend());
    dt->base.erase(dt->base.cbegin() + i, dt->base.cend());
}
