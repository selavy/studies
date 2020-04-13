#include "dawg.h"
#include <cassert>
#include <cstddef>
#include <climits>


[[maybe_unused]] static int getbase2(const Datrie2* t, int index)
{
    assert(index >= 0);
    auto s = static_cast<std::size_t>(index);
    return s < t->base.size() ? t->base[s] : INT_MIN;
}

[[maybe_unused]] static int getchck2(const Datrie2* t, int index)
{
    assert(index >= 0);
    auto s = static_cast<std::size_t>(index);
    return s < t->chck.size() ? t->chck[s] : INT_MIN;
}

[[maybe_unused]] static bool getterm2(const Datrie2* t, int index)
{
    assert(index >= 0);
    auto s = static_cast<std::size_t>(index);
    return s < t->term.size() ? t->term[s] != 0 : false;
}

[[maybe_unused]] static void setbase2(Datrie2* t, int index, int base, bool term)
{
    assert(index >= 0);
    auto s = static_cast<std::size_t>(index);
    assert(s < t->base.size());
    assert(s < t->term.size());
    t->base[s] = base;
    t->term[s] = term;
}

bool init2([[maybe_unused]] Datrie2* t)
{
    // TODO(peter): implement
    return false;
}

bool insert2([[maybe_unused]] Datrie2* t, [[maybe_unused]] const char* const word)
{
    // TODO(peter): implement
    return false;
}

bool isword2([[maybe_unused]] Datrie2* t, [[maybe_unused]] const char* const word)
{
    // TODO(peter): implement
    return false;
}

Letters childs2([[maybe_unused]] Datrie2* t, [[maybe_unused]] const char* const prefix)
{
    // TODO(peter): implement
    return {};
}
