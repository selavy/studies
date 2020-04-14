#pragma once

#include <vector>

struct Letters
{
    int children[26];
    int n_children;
};

struct Datrie2
{
    std::vector<int> base;
    std::vector<int> chck;
    std::vector<int> term; // TEMP TEMP
};

enum class Tristate
{
    eWord    = 0,
    eNoLink  = 1,
    eNotTerm = 2,
};

inline const char* tristate_to_str(Tristate t)
{
    switch (t) {
        case Tristate::eWord:    return "Word";
        case Tristate::eNoLink:  return "NoLink";
        case Tristate::eNotTerm: return "NotTerminal";
    }
    return "Unknown";
}

bool init2(Datrie2* t);
bool insert2(Datrie2* t, const char* const word);
Tristate isword2(Datrie2* t, const char* const word);
Letters childs2(Datrie2* t, const char* const prefix);
