#include <iostream>
#include <fstream>
#include <string>
#include <climits>
#include <memory>
#include "dawg.h"
#include "mafsa.h"


template <class F>
bool foreach_in_dictionary(std::string path, int max_words, std::string action, F&& f)
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
                std::cerr << "Failed to " << action << " the word \"" << word << "\"" << std::endl;
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
    return foreach_in_dictionary(path, max_words, /*action*/"insert",
        [trie](const std::string& word)
        {
            return insert2(trie, word.c_str());
        });
}

bool test_trie(Datrie2* trie, std::string path, int max_words=INT_MAX) {
    return foreach_in_dictionary(path, max_words, /*action*/"find",
        [trie](const std::string& word)
        {
            auto res = isword2(trie, word.c_str());
            if (res != Tristate::eWord) {
                std::cerr << "Word: \"" << word << "\": " << tristate_to_str(res) << "\n";
            }
            return res == Tristate::eWord;
        });
}

bool load_dict_mafsa(Mafsa& m, std::string path, int max_words=INT_MAX) {
    return foreach_in_dictionary(path, max_words, /*action*/"insert",
        [&m](const std::string& word)
        {
            m.insert(word);
            return true;
        });

}

template <class T>
bool test_dfa(const T& m, std::string path, int max_words=INT_MAX) {
    return foreach_in_dictionary(path, max_words, /*action*/"find",
        [&m](const std::string& word) { return m.isword(word); });

}

int main(int argc, char** argv)
{
    if (argc <= 1) {
        std::cerr << "Usage: [" << argv[0] << "] [DICT]" << std::endl;
        return 1;
    }
    const std::string filename = argv[1];
    int max_dict_words = INT_MAX;
    int max_check_words = 1000;
    if (argc >= 3) {
        max_check_words = std::max(atoi(argv[2]), 10);
    }
    if (argc >= 4) {
        max_dict_words = atoi(argv[3]);
        if (max_dict_words == 0) {
            max_dict_words = INT_MAX;
        }
    }

    if (1) {
        // DATrie
        std::cout << "Using DATrie implementation...\n";
        Datrie2 trie;
        if (!init2(&trie)) {
            std::cerr << "error: failed to initialize trie" << std::endl;
            return 1;
        }
        if (!load_dictionary(&trie, filename, max_dict_words)) {
            std::cerr << "error: failed to load dictionary" << std::endl;
            return 1;
        }
        std::cout << "SIZE BEFORE: " << trie.chck.size() << "\n";
        trim2(&trie);
        std::cout << "SIZE AFTER : " << trie.chck.size() << "\n";
        if (argc > 2) {
            printf("Checking %d words\n", max_check_words);
            if (test_trie(&trie, filename, max_check_words)) {
                printf("Passed!\n");
            } else {
                printf("Failed!\n");
            }
        }

        auto& base = trie.base;
        auto& chck = trie.chck;
        std::cout << "DATRIE2 memory usage:\n"
            << "base size = " << base.size() << " x 4 = " << (base.size() * 4) << "\n"
            << "chck size = " << chck.size() << " x 4 = " << (chck.size() * 4) << "\n"
            << "total size = " << (base.size() + chck.size()) << " x 4 = " << ((base.size() + chck.size()) * 4) << "\n";
    } else {
        // MAFSA
        std::cout << "Using MAFSA implementation...\n";
        Mafsa m;
        if (!load_dict_mafsa(m, filename, max_dict_words)) {
            std::cerr << "error: failed to load dictionary" << std::endl;
            return 1;
        }

        std::cout << "Checking words...\n";
        if (!test_dfa(m, filename, max_check_words)) {
            std::cerr << "error: failed to find all inserted words" << std::endl;
            return 1;
        }

        std::cout << "Reducing MAFSA...\n";
        std::cout << "# state before = " << m.numstates() << "\n";
        m.reduce();
        std::cout << "# state after  = " << m.numstates() << "\n";

        std::cout << "Checking words...\n";
        if (!test_dfa(m, filename, max_check_words)) {
            std::cerr << "error: failed to find all inserted words after reduce" << std::endl;
            return 1;
        }

        std::cout << "MAFSA Passed!" << std::endl;


        SDFA tt = SDFA::make(m);
        std::cout << "Checking words...\n";
        if (!test_dfa(m, filename, max_check_words)) {
            std::cerr << "error: failed to find all inserted words after reduce" << std::endl;
            return 1;
        }
        std::cout << "SDFA memory usage: " << tt.size() << " x 4 = " << (tt.size() * 4) << std::endl;
    }

    return 0;
}
