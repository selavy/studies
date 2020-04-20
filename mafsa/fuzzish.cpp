#include <iostream>
#include <fstream>
#include <string>
#include <unordered_set>
#include <climits>
#include <random>
#include "darray.h"


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

int main(int argc, char** argv)
{
    std::random_device rd;

    const std::string inname    = argc > 1 ?      argv[1]  : "csw19.txt";
    const int         max_words = argc > 2 ? atoi(argv[2]) : INT_MAX;
    const int         seed      = argc > 3 ? atoi(argv[3]) : rd();

    using STLDict = std::unordered_set<std::string>;
    const auto maybe_words = load_dictionary<STLDict>(inname, max_words);
    const auto maybe_dict  = load_dictionary<Darray> (inname, max_words);
    if (!maybe_words || !maybe_dict) {
        std::cerr << "error: unable to load dictionary" << std::endl;
        return 1;
    }
    const auto& words = *maybe_words;
    const auto& dict  = *maybe_dict;

    auto isword = [&](const std::string& word) { return words.count(word) != 0; };
    auto bstr   = [](bool b) { return b ? "true" : "false"; };

    std::mt19937 gen(seed);
    std::uniform_int_distribution<> dis(static_cast<int>('A'), static_cast<int>('Z'));
    auto nextch = [&]() { return static_cast<char>(dis(gen)); };

    int nfail = 0;
    int ntest = 0;
    int npass = 0;

    std::string w;
    for (int i = 0; i < 100000; ++i) {
        w = "";
        for (int jj = 0; jj < 5; ++jj) {
            w += nextch();
        }

        if (dict.isword(w) != isword(w)) {
            printf("FAILURE: word=%s expect=%s actual=%s", w.c_str(), bstr(dict.isword(w)), bstr(isword(w)));
            ++nfail;
        } else {
            ++npass;
        }
        ++ntest;
    }

    printf("# pass = %d; # fail = %d; # test = %d\n", npass, nfail, ntest);
    if (nfail == 0) {
        printf("PASSED!\n");
    } else {
        printf("FAILED!\n");
    }

    return 0;
}
