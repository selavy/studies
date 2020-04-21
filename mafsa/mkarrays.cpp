#include <iostream>
#include <fstream>
#include <optional>
#include <string>
#include <climits>
#include "mafsa.h"
#include "mafsa_generated.h"
#include "darray.h"
#include "darray_generated.h"
#include "tarraysep.h"
#include "tarray_generated.h"


template <class T>
std::optional<T> load_dictionary(std::string path, int max_words)
{
    T dict;
    std::string word;
    std::ifstream ifs{path};
    int n_words = 0;
    if (!ifs) {
        std::cerr << "error: unable to open input file\n";
        return std::nullopt;
    }
    while (ifs >> word) {
        if (word.empty()) {
            continue;
        }
        if (word.size() < 2 || word.size() > 15) {
            std::cerr << "warning: skipping invalid word: \"" << word << "\"\n";
        }
        bool valid_word = true;
        for (std::size_t i = 0; i < word.size(); ++i) {
            char c = word[i];
            if ('a' <= c && c <= 'z') {
                word[i] = (c - 'a') + 'A';
            } else if ('A' <= c && c <= 'Z') {
                word[i] = c;
            } else {
                std::cerr << "warning: invalid character '" << c << "' in word \"" << word << "\"\n";
                valid_word = false;
                break;
            }
        }
        if (valid_word) {
            dict.insert(word);
            if (++n_words >= max_words) {
                break;
            }
        }
    }
    return dict;
}

template <class T>
bool test_dictionary(const T& dict, std::string path, int max_words)
{
    std::string word;
    std::ifstream ifs{path};
    int n_words = 0;
    if (!ifs) {
        std::cerr << "error: unable to open input file\n";
        return false;
    }
    while (ifs >> word) {
        if (word.empty()) {
            continue;
        }
        if (word.size() < 2 || word.size() > 15) {
            std::cerr << "warning: skipping invalid word: \"" << word << "\"\n";
        }
        bool valid_word = true;
        for (std::size_t i = 0; i < word.size(); ++i) {
            char c = word[i];
            if ('a' <= c && c <= 'z') {
                word[i] = (c - 'a') + 'A';
            } else if ('A' <= c && c <= 'Z') {
                word[i] = c;
            } else {
                std::cerr << "warning: invalid character '" << c << "' in word \"" << word << "\"\n";
                valid_word = false;
                break;
            }
        }
        if (valid_word) {
            if (!dict.isword(word)) {
                std::cerr << "Failed on word: " << word << "\n";
                return false;
            }
            if (++n_words >= max_words) {
                break;
            }
        }
    }
    return true;
}

std::string make_out_filename(std::string inname, std::string newext)
{
    const std::string ext = ".txt";
    auto pos = inname.rfind(ext);
    if (pos == std::string::npos) {
        return inname + newext;
    } else {
        assert(pos < inname.size());
        return inname.replace(pos, ext.size(), newext);
    }
}

bool write_data(const std::string& filename, const uint8_t* buf, std::size_t length)
{
    std::ofstream ofs;
    ofs.open(filename, std::ios::binary);
    ofs.write(reinterpret_cast<const char*>(buf), length);
    ofs.close();
    return true;
}

bool write_darray(const Darray& darray, const std::string& filename)
{
    flatbuffers::FlatBufferBuilder builder;
    auto serial_darray = CreateSerialDarrayDirect(builder, &darray.bases, &darray.checks);
    builder.Finish(serial_darray);
    auto* buf = builder.GetBufferPointer();
    auto  len = builder.GetSize();
    return write_data(filename, buf, len);
}

bool write_tarray(const Tarraysep& tarray, const std::string& filename)
{
    flatbuffers::FlatBufferBuilder builder;
    auto serial_tarray = CreateSerialTarrayDirect(builder, &tarray.bases, &tarray.checks, &tarray.nexts);
    builder.Finish(serial_tarray);
    auto* buf = builder.GetBufferPointer();
    auto  len = builder.GetSize();
    return write_data(filename, buf, len);
}

bool write_mafsa(const Mafsa& mafsa, const std::string& filename)
{
    auto make_serial_links = [](const std::map<int, int>& children)
    {
        std::vector<SerialLink> result;
        for (auto [value, next] : children) {
            result.emplace_back(value, next);
        }
        return result;
    };

    flatbuffers::FlatBufferBuilder builder;
    std::vector<flatbuffers::Offset<SerialNode>> nodes;
    for (const auto& node : mafsa.ns) {
        auto children = make_serial_links(node.kids);
        auto serial_node = CreateSerialNodeDirect(builder, node.val, node.term, &children);
        nodes.emplace_back(serial_node);
    }
    auto serial_mafsa = CreateSerialMafsaDirect(builder, &nodes);
    builder.Finish(serial_mafsa);
    auto* buf = builder.GetBufferPointer();
    auto  len = builder.GetSize();
    return write_data(filename, buf, len);
}

std::ostream& operator<<(std::ostream& os, const Mafsa::Node& n)
{
    os << "value=" << n.val << ", term=" << (n.term ? "TRUE":"FALSE") << ", kids=[ ";
    for (auto [value, next] : n.kids) {
        os << "(" << value << ", " << next << ") ";
    }
    os << "]";
    return os;
}

int main(int argc, char** argv)
{
    // TODO: real command line parser
    if (argc == 0) {
        std::cerr << "Usage: " << argv[0] << " [FILE]" << std::endl;
        return 0;
    }

    const std::string inname    = argc >= 2 ? argv[1]       : "";
    const int         max_words = argc >= 3 ? atoi(argv[2]) : INT_MAX;
    const std::string doutname  = argc >= 4 ? argv[3]       : make_out_filename(inname, ".ddic");
    const std::string toutname  = argc >= 5 ? argv[4]       : make_out_filename(inname, ".tdic");
    const std::string moutname  = argc >= 6 ? argv[5]       : make_out_filename(inname, ".mfsa");

    std::cout << "INPUT:     " << inname    << "\n"
              << "OUTPUT   : " << doutname  << "\n"
              << "OUTPUT   : " << toutname  << "\n"
              << "OUTPUT   : " << moutname  << "\n"
              << "MAX WORDS: " << max_words << "\n"
              ;

    if (inname.empty() || max_words <= 0) {
        std::cerr << "error: invalid arguments";
        return 1;
    }


    if (0) {
        auto maybe_darray = load_dictionary<Darray>(inname, max_words);
        if (!maybe_darray) {
            return 1;
        }
        const auto& darray = *maybe_darray;

        if (!test_dictionary<Darray>(darray, inname, max_words)) {
            std::cerr << "dictionary test failed!" << std::endl;
            return 1;
        }

        write_darray(darray, doutname);
    }

    if (1) {
        auto maybe_mafsa = load_dictionary<Mafsa>(inname, max_words);
        if (!maybe_mafsa) {
            return 1;
        }
        auto& mafsa = *maybe_mafsa;
        mafsa.reduce();
        if (!test_dictionary<Mafsa>(mafsa, inname, max_words)) {
            std::cerr << "Mafsa test failed!" << std::endl;
            return 1;
        }

        write_mafsa(mafsa, moutname);

        {
            auto maybe_m2 = Mafsa::deserialize(moutname);
            if (!maybe_m2) {
                std::cerr << "error: unable to deserialize mafsa!" << std::endl;
                return 1;
            }
            auto& m2 = *maybe_m2;

#if 0
            std::cout << "mafsa nodes: " << mafsa.ns.size() << "\n";
            std::cout << "m2    nodes: " << m2   .ns.size() << "\n";
            if (mafsa.ns.size() == m2.ns.size()) {
                for (std::size_t i = 0; i < m2.ns.size(); ++i) {
                    std::cout << "Mafsa Node: " << mafsa.ns[i] << "\n";
                    std::cout << "M2    Node: " << m2   .ns[i] << "\n";
                }
            }
#endif

            if (!test_dictionary<Mafsa>(m2, inname, max_words)) {
                std::cerr << "Mafsa deserialize test failed!" << std::endl;
                return 1;
            }
        }

        if (0) {
            const auto& tarray = mafsa.make_tarray();
            if (!test_dictionary<Tarraysep>(tarray, inname, max_words)) {
                std::cerr << "dictionary test failed!" << std::endl;
                return 1;
            }
            write_tarray(tarray, toutname);
        }
    }

    return 0;
}
