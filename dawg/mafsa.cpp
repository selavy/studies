#include "mafsa.h"
#include "iconv.h"

void Mafsa::insert([[maybe_unused]] const char* const word)
{
    // TODO(peter): implement
}

bool Mafsa::isword(const char* const word) const
{
    int s = 0;
    for (const char* p = word; *p != '\0'; ++p) {
        const char ch = *p;
        const int c = iconv(ch);
        auto& node = ns[static_cast<Node::KidIdx>(s)];
        auto it = node.kids.find(c);
        if (it == node.kids.end()) {
            return false;
        }
        s = it->second;
    }
    return ns[static_cast<Node::KidIdx>(s)].term;
}

bool Mafsa::nodecmp(const Node& a, const Node& b)
{
    // To be a duplicate:
    //
    // 1) edges are the same
    // 2) term state is the same

    if (a.term != b.term) {
        return false;
    }
    if (a.kids.size() != b.kids.size()) {
        return false;
    }
    for (auto [val, link] : a.kids) {
        auto f = b.kids.find(val);
        if (f == b.kids.end()) {
            return false;
        }
        if (f->second != link) {
            return false;
        }
    }
    return true;
}
