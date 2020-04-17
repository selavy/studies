#include "mafsa.h"
#include <cstring>
#include <cstddef>
#include <cassert>
#include <iostream>
#include <algorithm>
#include "iconv.h"


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
                it->second = found->second;
            }
        }

        if (s == 0) {
            return;
        }

        for (auto tt : reg) {
            assert(!rep.count(tt));
            auto t = static_cast<Node::KidIdx>(tt);
            if (nodecmp(ns[s], ns[t])) {
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

    std::size_t new_size;
    std::map<std::size_t, std::size_t> conv;  // old -> new
    std::map<std::size_t, std::size_t> rconv; // new -> old
    { // re-number states
        std::size_t next_state = 0;
        for (std::size_t i = 0; i < ns.size(); ++i) {
            if (!validstate(i)) {
                continue;
            }
            conv[i]           = next_state;
            rconv[next_state] = i;
            ++next_state;
        }
        new_size = next_state;
    }

    std::vector<Node> newnodes(new_size);
    assert(newnodes.size() == new_size);
    for (std::size_t i = 0; i < new_size; ++i) {
        const auto newidx = i;
        const auto oldidx = rconv[newidx];
        assert(rconv.count(newidx));
        assert(conv.count(oldidx));
        assert(ns[oldidx].val != -1 || oldidx == 0);

        newnodes[newidx].term = ns[oldidx].term;
        newnodes[newidx].val  = ns[oldidx].val;
        for (const auto [val, kid] : ns[oldidx].kids) {
            const auto oldkididx = static_cast<std::size_t>(kid);
            const auto newkididx = conv[oldkididx];
            assert(conv.count(oldkididx));
            newnodes[newidx].kids.emplace(val, newkididx);
        }
    }

    ns = std::move(newnodes);
}

// ----------------------------------------------------------------------------
// Make Tarraysep
// ----------------------------------------------------------------------------

// TODO: move into `Mafsa::make_tarraysep`
template <class Cont>
auto getkeys_plus_one(const Cont& c)
{
    std::vector<typename Cont::key_type> keys;
    for (auto&& [key, val] : c) {
        keys.emplace_back(key + 1);
    }
    return keys;
}

const int* findbase(const int* const ckbegin, const int* const ckend, const int* const vbegin, const int* const vend)
{
    // precondition: values in [vbegin, vend) are sorted
    // assert(std::is_sorted(vbegin, vend));

    auto baseworks = [=](const int* const check)
    {
        for (const int* valp = vbegin; valp != vend; ++valp) {
            const int val = *valp - *vbegin;
            assert((ckbegin + val) < ckend);
            if (check[val] != Tarraysep::UNSET_CHECK) {
                return false;
            }
        }
        return true;
    };

    for (const int* ckp = ckbegin; ckp != ckend; ++ckp) {
        if (baseworks(ckp)) {
            return ckp;
        }
    }
    return nullptr;
}

Tarraysep Mafsa::make_tarray() const
{
    const auto n_states = std::max<std::size_t>(static_cast<std::size_t>(numstates()), 50);
    Tarraysep result(n_states);
    auto& bases  = result.bases;
    auto& checks = result.checks;
    auto& nexts  = result.nexts;

    bases[0] = 0;
    checks[0] = 0;

    auto extendarrays = [&](const std::size_t need)
    {
        nexts.insert(nexts.end()  , need, Tarraysep::UNSET_NEXT);
        checks.insert(checks.end(), need, Tarraysep::UNSET_CHECK);
    };

    auto find_base_or_extend = [&](const std::vector<int>& vals) -> int
    {
        assert(std::is_sorted(vals.begin(), vals.end()));
        std::size_t start = 0;
        std::size_t maxv  = !vals.empty() ? static_cast<std::size_t>(vals.back()) : 0;
        for (;;) {
            assert(maxv <= checks.size());
            const std::size_t chkend = std::max(start, checks.size() - maxv);
            const int* p = findbase(&checks[start], &checks[chkend], &vals[0], &vals[vals.size()]);
            if (p != nullptr) {
                const int first_value = !vals.empty() ? vals[0] : 0;
                const int index = static_cast<int>(p - &checks[start]) - first_value + static_cast<int>(start);
                assert(0 <= (index + first_value));
                assert(static_cast<std::size_t>(index + first_value) < checks.size());
                assert(checks[static_cast<std::size_t>(index + first_value)] == Tarraysep::UNSET_CHECK);
                return index;
            }
            const std::size_t lookback = !vals.empty() ? static_cast<std::size_t>(vals.back()) : 1;
            start = checks.size() > lookback ? checks.size() - lookback : 0;
            extendarrays(10);
        }
    };

    visit_pre(0,
        [&](int ss)
        {
            auto s = static_cast<std::size_t>(ss);
            assert(0 <= ss && s < ns.size());
            auto& node = ns[s];
            if (node.kids.empty()) {
                result.setbase(s, Tarraysep::UNSET_BASE, node.term);
                return;
            }

            auto vals  = getkeys_plus_one(node.kids);
            const int  base_ = find_base_or_extend(vals);
            result.setbase(s, base_, node.term);
            assert(0 <= ss && s < bases.size());
            for (auto&& [val, next_] : node.kids) {
                std::size_t c = static_cast<std::size_t>(val + 1);
                assert(base_ + static_cast<int>(c) >= 0);
                std::size_t i = static_cast<std::size_t>(base_) + c;
                assert(i < checks.size());
                assert(i < nexts.size());
                assert(nexts[i]  == Tarraysep::UNSET_NEXT);
                assert(checks[i] == Tarraysep::UNSET_CHECK);
                nexts [i] = next_;
                checks[i] = ss;
            }
        });

    return result;
}
