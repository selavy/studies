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
Iter search(Iter first, Iter last, const Searcher& searcher)
{
    return searcher(first, last).first;
}

// TODO: use BinaryPredicate
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
            ForwardIter first, ForwardIter last) const noexcept
    {
        auto iter = search(first, last, s_first, s_last);
        auto result = std::make_pair(iter, iter);
        if (result.first != last) {
            std::advance(result.second, std::distance(s_first, s_last));
        }
        return result;
    }

    ForwardIter s_first, s_last;
};

template <class Iter>
struct RabinKarp
{
    constexpr static auto q = 18446744073709551557ull;
    // constexpr static auto q = 2147483647ull;
    constexpr static auto d = 256ull;

    RabinKarp(Iter first, Iter last) noexcept
        : s_first{first}, s_last{last} {}

    std::pair<Iter, Iter> operator()(Iter first, Iter last) const noexcept
    {
        // preprocessing: calculate hashes
        uint64_t h = 1;
        uint64_t p = 0;
        uint64_t t = 0;
        auto m = 0u;
        auto p1 = first;
        auto p2 = s_first;
        if (s_first == s_last) {
            return std::make_pair(first, first);
        }
        for (; p2 != s_last;) {
            if (p1 == last) { // pattern is longer than text
                return std::make_pair(last, last);
            }
            auto c1 = static_cast<uint8_t>(*p1++);
            auto c2 = static_cast<uint8_t>(*p2++);
            h = (d*h     ) % q;
            t = (d*t + c1) % q;
            p = (d*p + c2) % q;
            ++m;
        }
        h = (h / d) % q;

        // matching
        while (p1 != last) {
            if (p == t) { // hash hit!
                auto it = std::search(first, p1, s_first, s_last);
                if (it == first) {
                    return std::make_pair(first, p1);
                }
            }
            auto c1 = static_cast<uint8_t>(*first);
            auto c2 = static_cast<uint8_t>(*p1);
            t = (d*(t - c1*h) + c2) % q;
            ++first;
            ++p1;
        }

        if (p == t) { // hash hit!
            auto it = std::search(first, p1, s_first, s_last);
            if (it == first) {
                return std::make_pair(first, p1);
            }
        }

        return std::make_pair(last, last);
    }

    Iter s_first, s_last;
};

} // namespace pl
