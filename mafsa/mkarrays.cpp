#include <iostream>
#include <fstream>
#include <optional>
#include <string>
#include <climits>
#include "darray.h"
#include "darray_generated.h"

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

std::string make_out_filename(std::string inname)
{
    const std::string ext    = ".txt";
    const std::string newext = ".bin";
    auto pos = inname.rfind(ext);
    if (pos == std::string::npos) {
        return inname + newext;
    } else {
        assert(pos < inname.size());
        return inname.replace(pos, ext.size(), newext);
    }
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
    const std::string outname   = argc >= 4 ? argv[3]       : make_out_filename(inname);

    std::cout << "INPUT:     " << inname    << "\n"
              << "OUTPUT   : " << outname   << "\n"
              << "MAX WORDS: " << max_words << "\n"
              ;

    if (inname.empty() || max_words <= 0) {
        std::cerr << "error: invalid arguments";
        return 1;
    }


    auto maybe_darray = load_dictionary<Darray>(inname, max_words);
    if (!maybe_darray) {
        return 1;
    }
    const auto& darray = *maybe_darray;

    return 0;
}
