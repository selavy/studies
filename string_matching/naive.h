#pragma once

#include <iterator>

namespace naive {

template <class ForwardIter>
ForwardIter search(ForwardIter first, ForwardIter last,
        ForwardIter s_first, ForwardIter s_last, std::forward_iterator_tag)
{
    auto is_match = [](auto p1, auto e1, auto p2, auto e2)
    {
        for (; p1 != e1 && p2 != e2; ++p1, ++p2) {
            if (*p1 != *p2) {
                return false;
            }
        }
        return p2 == e2;
    };

    for (; first != last; ++first) {
        if (is_match(first, last, s_first, s_last)) {
            return first;
        }
    }
    return last;
}

template <class RandomAccessIter>
RandomAccessIter search(RandomAccessIter first, RandomAccessIter last,
        RandomAccessIter s_first, RandomAccessIter s_last,
        std::random_access_iterator_tag)
{
    auto is_match = [](auto p1, auto p2, auto e2)
    {
        for (; p2 != e2; ++p1, ++p2) {
            if (*p1 != *p2) {
                return false;
            }
        }
        return true;
    };

    auto n = std::distance(first, last);
    auto m = std::distance(s_first, s_last);
    if (m > n) {
        return last;
    }
    auto t_last = std::prev(last, m - 1);
    for (; first != t_last; ++first) {
        if (is_match(first, s_first, s_last)) {
            return first;
        }
    }
    return last;
}

template <class Iter>
Iter search(Iter first, Iter last, Iter s_first, Iter s_last)
{
    return search(first, last, s_first, s_last,
            typename std::iterator_traits<Iter>::iterator_category());
}

} // namespace naive
