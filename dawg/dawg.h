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

bool init2(Datrie2* t);
bool insert2(Datrie2* t, const char* const word);
bool isword2(Datrie2* t, const char* const word);
Letters childs2(Datrie2* t, const char* const prefix);
