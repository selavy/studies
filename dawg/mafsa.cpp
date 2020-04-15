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
    std::cout << "REDUCE\n";
    // algorithm sketched in https://www.aclweb.org/anthology/J00-1002.pdf on pg 7

    std::map<int, int> rep;
    std::vector<int>   reg;
    visit_post(0, [&](int ss)
    {
        if (ss == 0) {
            return;
        }
        auto s = static_cast<Node::KidIdx>(ss);
        auto& n = ns[s];

        for (auto it = n.kids.begin(); it != n.kids.end(); ++it) {
            auto found = rep.find(it->second);
            if (found != rep.end()) {
                std::cout << "MODIFING CHILD LINK FOR " << tochar(it->first) << " FROM " << it->second << " TO " << found->second << "\n";
                it->second = found->second;
            }
        }

        std::cout << "Visiting: " << n << std::endl;
        for (auto tt : reg) {
            assert(!rep.count(tt));
            auto t = static_cast<Node::KidIdx>(tt);
            if (nodecmp(ns[s], ns[t])) {
                std::cout << "REPLACING (" << s << ")" << ns[s] << " WITH (" << t << ")" << ns[t] << "\n";
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
