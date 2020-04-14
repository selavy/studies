#include <iostream>
#include <fstream>
#include <string>
#include <climits>
#include <memory>
#include "dawg.h"


bool load_dictionary(Datrie2* trie, std::string path, int max_words=INT_MAX) {
    int n_words = 0;
    std::string word;
    std::ifstream ifs{path};
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
                word[i] = static_cast<char>((c - 'a') + 'A');
            } else if ('A' <= c && c <= 'Z') {
                word[i] = c;
            } else {
                std::cerr << "warning: invalid character '" << c << "' in word \"" << word << "\"\n";
                valid_word = false;
                break;
            }
        }
        if (valid_word) {
            const bool ok = insert2(trie, word.c_str());
            if (!ok) {
                std::cerr << "Failed to insert the word \"" << word << "\"" << std::endl;
                return false;
            }
            if (++n_words >= max_words) {
                break;
            }
        }
    }
    return true;
}

int main(int argc, char** argv)
{
    if (argc <= 1) {
        std::cerr << "Usage: [" << argv[0] << "] [DICT]" << std::endl;
        return 1;
    }

    Datrie2 trie;
    if (!init2(&trie)) {
        std::cerr << "error: failed to initialize trie" << std::endl;
        return 1;
    }

    if (!load_dictionary(&trie, argv[1])) {
        std::cerr << "error: failed to load dictionary" << std::endl;
        return 1;
    }

    return 0;
}
