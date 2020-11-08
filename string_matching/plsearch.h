#pragma once

#include <iterator>
#include <functional>

namespace pl {

#if 0
template <class ForwardIter>
ForwardIter search(ForwardIter first, ForwardIter last,
        ForwardIter s_first, ForwardIter s_last, std::forward_iterator_tag)
{
    auto is_match = [](ForwardIter p1, ForwardIter e1, ForwardIter p2,
                       ForwardIter e2)
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
#endif

template <class ForwardIter>
ForwardIter search(ForwardIter first, ForwardIter last,
        ForwardIter s_first, ForwardIter s_last, std::forward_iterator_tag)
{
    if (s_first == s_last) {
        return first;
    }
    if (first == last) {
        return last;
    }
    // special case for 1 character search
    auto p = s_first;
    if (++p == s_last) {
        return std::find(first, last, *s_first);
    }
    // general case:
    for (;;) {
        first = std::find(first, last, *s_first);
        if (first == last) {
            return last;
        }
        auto p1 = first;
        if (++p1 == last) {
            return last;
        }
        auto p2 = p;
        while (*p1 == *p2) {
            if (++p2 == s_last) {
                return first;
            }
            if (++p1 == last) {
                return last;
            }
        }
        ++first;
    }
    __builtin_unreachable();
    return last;
}

template <class RandomAccessIter>
RandomAccessIter search(RandomAccessIter first, RandomAccessIter last,
        RandomAccessIter s_first, RandomAccessIter s_last,
        std::random_access_iterator_tag)
{
    auto is_match = [](RandomAccessIter p1, RandomAccessIter p2,
                       RandomAccessIter e2)
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

template <class Iter, class Searcher>
Iter search(Iter first, Iter last, Searcher& searcher)
{
    return searcher(first, last).first;
}

template <class ForwardIter, class BinaryPredicate = std::equal_to<>>
struct NaiveSearch : BinaryPredicate
{
    NaiveSearch(ForwardIter pat_first, ForwardIter pat_last,
            BinaryPredicate pred = BinaryPredicate{}) noexcept
        : BinaryPredicate{pred}
        , s_first{pat_first}
        , s_last{pat_last}
    {}

    std::pair<ForwardIter, ForwardIter> operator()(
            ForwardIter first, ForwardIter last) noexcept
    {
        auto iter = search(first, last, s_first, s_last);
        auto result = std::make_pair(iter, iter);
        if (result.first != last) {
            std::advance(result.second, std::distance(s_first, s_last));
        }
        return result;
    }

    ForwardIter s_first;
    ForwardIter s_last;
};

// template <class BiDirIter>

} // namespace pl
