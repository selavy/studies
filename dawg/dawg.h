#pragma once

#include <vector>

struct Letters
{
    bool ischild[26];
};

struct Datrie2
{
    std::vector<int> base;
    std::vector<int> chck;
};

bool init2(Datrie2* t);
bool insert2(Datrie2* t, const char* const word);
bool isword2(Datrie2* t, const char* const word);
Letters childs2(Datrie2* t, const char* const prefix);
