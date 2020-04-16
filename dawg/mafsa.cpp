#include "mafsa.h"
#include "iconv.h"
#include <iostream>

Mafsa::Mafsa()
{
    ns.emplace_back();
    auto& newnode = ns.back();
    newnode.val  = -1;
    newnode.term = false;
}

void Mafsa::insert(const char* const word)
{
    int s = 0;
    for (const char* p = word; *p != '\0'; ++p) {
        assert(0 <= s && s < static_cast<int>(ns.size()));
        const char ch = *p;
        const int c = iconv(ch);
        auto& node = ns[static_cast<Node::KidIdx>(s)];
        auto it = node.kids.find(c);
        if (it == node.kids.end()) {
            const int t = static_cast<int>(ns.size());
            ns.emplace_back();
            auto& newnode = ns.back();
            newnode.val  = c;
            newnode.term = false;
            ns[static_cast<Node::KidIdx>(s)].kids[c] = t;
            s = t;
        } else {
            s = it->second;
        }
    }
    ns[static_cast<Node::KidIdx>(s)].term = true;
}

bool Mafsa::isword(const char* const word) const
{
    int s = 0;
    for (const char* p = word; *p != '\0'; ++p) {
        assert(0 <= s && s < static_cast<int>(ns.size()));
        const char ch = *p;
        const int c = iconv(ch);
        auto& node = ns[static_cast<Node::KidIdx>(s)];
        auto it = node.kids.find(c);
        if (it == node.kids.end()) {
            return false;
        }
        s = it->second;
        assert(c == ns[static_cast<Node::KidIdx>(s)].val);
    }
    return ns[static_cast<Node::KidIdx>(s)].term;
}

int Mafsa::numstates() const
{
    int count = 0;
    for (std::size_t i = 0; i < ns.size(); ++i) {
        if (validstate(i)) {
            ++count;
        }
    }
    // for (const auto& n : ns) {
    //     if (0 <= n.val && n.val <= 26) {
    //         ++count;
    //     } else {
    //         assert(n.val == -1);
    //     }
    // }
    return count;
}

bool Mafsa::validstate(std::size_t i) const
{
    const auto& n = ns[i];
    if (i == 0) {
        return true;
    }
    if (0 <= n.val && n.val <= 26) {
        return true;
    }
    assert(n.val == -1);
    return false;
}

bool Mafsa::nodecmp(const Node& a, const Node& b)
{
    // from https://www.aclweb.org/anthology/J00-1002.pdf pg. 7:
    //
    // (assuming doing post-order traversal of the trie)
    //
    // State `p` belongs to the same (equivalence) class as `q` if and only if:
    //
	// 1. they are either both final or both nonfinal; and
	// 2. they have the same number of outgoing transitions; and
	// 3. corresponding outgoing transitions have the same labels; and
    // 4'. corresponding transitions lead to the same states.

    if (a.val != b.val) {
        return false;
    }
    if (a.term != b.term) {
        return false;
    }
    if (a.kids.size() != b.kids.size()) {
        return false;
    }
    auto ait = a.kids.begin();
    auto bit = b.kids.begin();
	for (std::size_t i = 0; i < a.kids.size(); ++i, ++ait, ++bit) {
        assert(ait != a.kids.end());
        assert(bit != b.kids.end());
        if (ait->first != bit->first) {
            return false;
        }
        if (ait->second != bit->second) {
            return false;
        }
	}
    return true;
}

[[maybe_unused]] auto tochar  = [](int v)  { return static_cast<char>(v + 'A'); };
[[maybe_unused]] auto boolstr = [](bool v) { return v ? "T" : "F"; };

#if 1
std::ostream& operator<<(std::ostream& os, const Mafsa::Node& n)
{
    os << "Node(" << tochar(n.val) << ", " << boolstr(n.term) << ", [ ";
    for (auto [val, kid] : n.kids) {
        os << "(" << tochar(val) << ", " << kid << ") ";
    }
    os << "])";
    return os;
}
#endif

void Mafsa::reduce()
{
    // algorithm sketched in https://www.aclweb.org/anthology/J00-1002.pdf on pg 7
    //
    // I'm not following the algorithm exactly. I try to match with a state in the register
    // for all nodes because if I skip the non-forward-branching states then the kid links
    // in forward-branching state won't match. I'm not sure how they are solving that? I guess
    // I could only compare on the forward-branching nodes then walk all the pointers to verify
    // that they are the same, but they claim to not do exactly that!

    std::map<int, int> rep;
    std::vector<int>   reg;
    visit_post(0, [&](int ss)
    {
        auto s = static_cast<Node::KidIdx>(ss);
        auto& n = ns[s];
        for (auto it = n.kids.begin(); it != n.kids.end(); ++it) {
            auto found = rep.find(it->second);
            if (found != rep.end()) {
                // std::cout << "MODIFING CHILD LINK FOR " << tochar(it->first) << " FROM " << it->second << " TO " << found->second << "\n";
                it->second = found->second;
            }
        }

        if (s == 0) {
            return;
        }

        // std::cout << "Visiting: " << n << std::endl;
        for (auto tt : reg) {
            assert(!rep.count(tt));
            auto t = static_cast<Node::KidIdx>(tt);
            if (nodecmp(ns[s], ns[t])) {
                // std::cout << "REPLACING (" << s << ")" << ns[s] << " WITH (" << t << ")" << ns[t] << "\n";
                rep[ss] = tt;
                return;
            }
        }
        reg.push_back(ss);
    });

    // delete nodes -- would want to actually reclaim memory here
    for (auto [to, from] : rep) {
        auto t = static_cast<Node::KidIdx>(to);
        ns[t].kids.clear();
        ns[t].val = -1;
        ns[t].term = false;
    }
}

bool SDFA::isword(const char* const word) const
{
    u32 N = static_cast<u32>(tt.size());
    u32 s = 0;
    for (const char* p = word; *p != '\0'; ++p) {
        const char ch = *p;
        const u32  c  = static_cast<u32>(iconv(ch));
        const u32  i  = s + c;
        const u32  t  = i < N ? (tt[i] & INDEX_MASK) : 0;
        if (t == 0) {
            return false;
        }
        s = t;
    }
    return (tt[s] & TERM_MASK) != 0 ? 1 : 0;
}

/*static*/ SDFA SDFA::make(const Mafsa& m)
{
    SDFA result;
    std::size_t n_states = static_cast<std::size_t>(m.numstates());
    result.tt = std::vector<u32>(n_states * NumLetters, NOTRANSITION);
    auto& tt = result.tt;
    assert(tt.size() == (n_states * NumLetters));

    std::map<std::size_t, u32> sconv; // <state number in MA-FSA> -> <state number in TT>

    // number mafsa states
    u32 sdfa_index = 0;
    for (std::size_t mafsa_index = 0; mafsa_index < m.ns.size(); ++mafsa_index) {
        if (!m.validstate(mafsa_index)) {
            continue;
        }
        sconv[mafsa_index] = sdfa_index++;
    }

    // assign into TT
    for (std::size_t mafsa_index = 0; mafsa_index < m.ns.size(); ++mafsa_index) {
        if (!m.validstate(mafsa_index)) {
            continue;
        }

        const auto& state = m.ns[mafsa_index];
        const auto midx = mafsa_index;
        const auto sidx = sconv[midx];
        assert(sconv.count(midx));
        assert(sidx != 0 || midx == 0);
        const std::size_t base = sidx * NumLetters;
        assert((base + NumLetters - 1) < tt.size());

        for (auto [val, next_state] : state.kids) {
            assert(0 <= val && static_cast<u32>(val) <= NumLetters);
            const auto off = static_cast<std::size_t>(val);
            const auto idx = base + off;
            const u32  next_base = sconv[static_cast<std::size_t>(next_state)] * NumLetters;
            assert(next_base != 0);
            assert(idx       < tt.size());
            assert(next_base < tt.size());
            tt[idx] = next_base;
        }

        if (state.term) {
            for (u32 i = 0; i < NumLetters; ++i) {
                tt[base + i] |= TERM_MASK;
            }
        }
    }

    return result;
}
