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

    auto dotest = [&](const std::string& word)
    {
        bool actual = dict.isword(word);
        bool expect = isword(word);
        if (expect != actual) {
            printf("FAILURE: word=%s expect=%s actual=%s", word.c_str(), bstr(expect), bstr(actual));
            ++nfail;
        } else {
            ++npass;
        }
        ++ntest;
    };

    std::string w;
    for (int i = 0; i < 100000; ++i) {
        w = "";
        for (int jj = 0; jj < 5; ++jj) {
            w += nextch();
        }
        dotest(w);
    }

    for (auto word : words) {
        for (char c = 'A'; c <= 'Z'; ++c) {
            word += c;
            dotest(word);
            word.pop_back();
        }

        auto ww = word;
        for (int ii = 0; ii < 10; ++ii) {
            ww += nextch();
            if (ww.size() > 15) {
                break;
            }
            for (char c = 'A'; c <= 'Z'; ++c) {
                ww += c;
                dotest(word);
                ww.pop_back();
            }
        }
    }

    printf("# pass = %d; # fail = %d; # test = %d\n", npass, nfail, ntest);
    if (nfail == 0) {
        printf("PASSED!\n");
    } else {
        printf("FAILED!\n");
    }

    return 0;
}
