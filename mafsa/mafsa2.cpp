#include "mafsa2.h"
#include "iconv.h"
#include <iostream>
#include <cassert>
#include "tarray_util.h"
#include "mafsa_generated.h"

bool Mafsa2::isword(const char* const word) const noexcept
{
    int s = 0;
    for (const char* p = word; *p != '\0'; ++p) {
        const int c = iconv(*p);
        assert(0 <= s && static_cast<std::size_t>(s) < nodes.size());
        assert(0 <= c && c <= 26);
        auto& node = nodes[static_cast<std::size_t>(s)];
        auto t = node.children[c];
        if (t == 0) {
            return false;
        }
        s = t;
    }
    return terms[static_cast<std::size_t>(s)];
}

template <class Cont>
void vec_stats(std::ostream& os, Cont& vec, std::string name, std::size_t& items, std::size_t& bytes)
{
    using T = typename Cont::value_type;
    os << name << " : items=" << vec.size() << ", bytes=" << (vec.size() * sizeof(T)) << "\n";
    items += vec.size();
    bytes += vec.size() * sizeof(T);
}

void Mafsa2::dump_stats(std::ostream& os) const
{
    std::size_t total_items = 0;
    std::size_t total_bytes = 0;
    os << "Mafsa2 Stats:\n";
    vec_stats(os, nodes, "nodes", total_items, total_bytes);
    vec_stats(os, terms, "terms", total_items, total_bytes);
    os << "total items=" << total_items << ", total bytes=" << total_bytes << "\n";
}

std::optional<Mafsa2> Mafsa2::deserialize(const std::string& filename)
{
    auto buf = read_dict_file(filename);
    auto serial_mafsa = GetSerialMafsa(buf.data());
    flatbuffers::Verifier v(reinterpret_cast<const uint8_t*>(buf.data()), buf.size());
    assert(serial_mafsa->Verify(v));
    Mafsa2 mafsa;
    mafsa.nodes = std::vector<Mafsa2::Node>(serial_mafsa->nodes()->size());
    mafsa.terms = std::vector<bool        >(serial_mafsa->nodes()->size(), false);
    std::size_t i = 0;
    for (const auto* node : *serial_mafsa->nodes()) {
        // mafsa.nodes[i].value = node->value();
        mafsa.terms[i] = node->term();
        std::fill(std::begin(mafsa.nodes[i].children), std::end(mafsa.nodes[i].children), 0);
        for (const auto* link : *node->children()) {
            assert(0 <= link->value() && link->value() <= 26);
            mafsa.nodes[i].children[link->value()] = link->next();
        }
        i++;
    }
    return mafsa;
}
