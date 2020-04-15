#pragma once

#include <vector>
#include <cstdint>

struct Letters
{
    char kids[26];
    int  n_kids;
};

struct Datrie2
{
    std::vector<uint32_t> base;
    std::vector<int>      chck;
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

bool     init2(Datrie2* t);
bool     insert2(Datrie2* t, const char* const word);
Tristate isword2(Datrie2* t, const char* const word);
Letters  kids2(Datrie2* t, const char* const prefix);
void     trim2(Datrie2* t);
// void     destroy2(Datrie2* t);
