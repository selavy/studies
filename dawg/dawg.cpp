#include "dawg.h"
#include <cassert>
#include <cstddef>
#include <climits>
#include <cstdio> // TEMP

#define AsIdx(x) static_cast<std::size_t>(x)

constexpr int MISSING_BASE = -1;
constexpr int UNSET_BASE   =  0;
constexpr int UNSET_CHCK   = -1;
constexpr int UNSET_TERM   =  0;


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

[[maybe_unused]] static void setbase2(Datrie2* t, int index, int base /*, bool term*/)
{
    assert(index >= 0);
    auto s = static_cast<std::size_t>(index);
    assert(s < t->base.size());
    assert(s < t->term.size());
    // DEBUG("SETBASE[%d] = %d", index, base);
    t->base[s] = base;
    // t->term[s] = term;
}

[[maybe_unused]] static void clrbase2(Datrie2* t, int index)
{
    assert(index >= 0);
    auto s = static_cast<std::size_t>(index);
    assert(s < t->base.size());
    assert(s < t->term.size());
    // DEBUG("CLRBASE[%d]", index);
    t->base[s] = UNSET_BASE;
}

[[maybe_unused]] static void setchck2(Datrie2* t, int index, int base)
{
    assert(index >= 0);
    auto s = static_cast<std::size_t>(index);
    assert(s < t->chck.size());
    // DEBUG("SETCHECK[%d] = %d", index, base);
    t->chck[s] = base;
}

[[maybe_unused]] static void clrchck2(Datrie2* t, int index)
{
    assert(index >= 0);
    auto s = static_cast<std::size_t>(index);
    assert(s < t->chck.size());
    // DEBUG("CLRCHECK[%d]", index);
    t->chck[s] = UNSET_CHCK;
}

[[maybe_unused]] static void setterm2(Datrie2* t, int index, bool term)
{
    assert(index >= 0);
    auto s = static_cast<std::size_t>(index);
    assert(s < t->chck.size());
    // DEBUG("SETTERM[%d] = %s", index, term?"T":"F");
    t->term[s] = term;
}

[[maybe_unused]] static void clrterm2(Datrie2* t, int index)
{
    assert(index >= 0);
    auto s = static_cast<std::size_t>(index);
    assert(s < t->chck.size());
    // DEBUG("CLRTERM[%d]", index);
    t->term[s] = false;
}

bool init2([[maybe_unused]] Datrie2* t)
{
    const std::size_t N = 1000; // TODO: fix me

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
    for (int c = 1; c <= 27; ++c) {
        if (getchck2(dt, getbase2(dt, s) + c) == s) {
            childs[n_childs++] = c;
        }
    }
    return n_childs;
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
        [[maybe_unused]] const char ch = static_cast<char>((childs[i] - 1) + 'A'); // TEMP TEMP
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
        clrbase2(dt, t_old); // TEMP TEMP: for debugging
        clrterm2(dt, t_old); // TEMP TEMP: for debugging
    }
    setbase2(dt, s, b);
    // clrterm2(dt, s); // TODO: needed?

    // DEBUG("Finished relocate state %d to base=%d", s, b);
}

bool insert2([[maybe_unused]] Datrie2* dt, [[maybe_unused]] const char* const word)
{
    // TODO(peter): revisit -- warnings don't like parameter being named `s` saying it shadows...
    [[maybe_unused]] auto base  = [dt](int x) { return getbase2(dt, x); };
    [[maybe_unused]] auto check = [dt](int x) { return getchck2(dt, x); };

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

        assert(AsIdx(t) < chck.size()); // TODO: handle resizing

        // TODO: Can I mark the current branch as a child? Not sure if the logic will work
        //       trying to move an uninstall node.
        int n_childs = cntchilds(dt, s, &childs[0]);
        if (n_childs > 0) {
            if (check(t) == UNSET_CHCK) { // slot is available
                setchck2(dt, t, s);
                s = t;
            } else {
                childs[n_childs++] = c;
                const std::size_t maxc = AsIdx(childs[n_childs - 1]);
                const std::size_t last = chck.size() - maxc;
                assert(chck.size() > maxc);
                int b_new = findbaserange(&chck[0], &chck[last], &childs[0], &childs[n_childs], UNSET_CHCK);
                assert(b_new >= 0);
                --n_childs;
                relocate2(dt, s, b_new, &childs[0], n_childs);
                setchck2(dt, b_new + c, s);
                s = b_new + c;
            }
        } else {
            const std::size_t chck_end = chck.size() - AsIdx(c);
            const int b_new = findbase(&chck[0], &chck[chck_end], c, UNSET_CHCK);
            assert(b_new != -1); // TODO: implement resizing
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
    // DEBUG("ENTER isword2: %s", word);
    int s = 0;
    for (const char* p = word; *p != '\0'; ++p) {
        const char ch = *p;
        const int  c  = iconv(ch) + 1;
        const int  t  = getbase2(dt, s) + c;
        // DEBUG("\tch=%c c=%d s=%d base[s]=%d t=%d check[t]=check[%d]=%d =?= %d", ch, c, s,
        //         getbase2(dt, s), t, t, getchck2(dt, t), s);
        if (getchck2(dt, t) != s) {
            // return false;
            return Tristate::eNoLink;
        }
        s = t;
    }
    // DEBUG("\tgetterm2(dt, %d) = %d", s, getterm2(dt, s));
    // DEBUG("EXIT  isword2: %s", word);
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
