#include <iostream>
#include <fstream>
#include <string>
#include <climits>
#include <memory>
#include "dawg.h"

template <class F>
bool foreach_in_dictionary(std::string path, int max_words, F&& f)
{
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
            const bool ok = f(word);
            // const bool ok = insert2(trie, word.c_str());
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


bool load_dictionary(Datrie2* trie, std::string path, int max_words=INT_MAX)
{
    return foreach_in_dictionary(path, max_words,
        [trie](const std::string& word)
        {
            return insert2(trie, word.c_str());
        });
}

bool test_trie(Datrie2* trie, std::string path, int max_words=INT_MAX) {
    return foreach_in_dictionary(path, max_words,
        [trie](const std::string& word)
        {
            return isword2(trie, word.c_str()) == Tristate::eWord;
        });
}

int main(int argc, char** argv)
{
    if (argc <= 1) {
        std::cerr << "Usage: [" << argv[0] << "] [DICT]" << std::endl;
        return 1;
    }
    const std::string filename = argv[1];
    int max_words = 1000;
    if (argc >= 3) {
        max_words = std::max(atoi(argv[2]), 10);
    }

    Datrie2 trie;
    if (!init2(&trie)) {
        std::cerr << "error: failed to initialize trie" << std::endl;
        return 1;
    }

    if (!load_dictionary(&trie, filename)) {
        std::cerr << "error: failed to load dictionary" << std::endl;
        return 1;
    }

    if (argc > 2) {
        printf("Checking %d words\n", max_words);
        if (test_trie(&trie, filename, max_words)) {
            printf("Passed!\n");
        } else {
            printf("Failed!\n");
        }
    }

    return 0;
}
