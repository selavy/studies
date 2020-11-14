#pragma once

#include <iterator>
#include <functional>

// TEMP TEMP
#include <iostream>
#include <iomanip>

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

// 32.1: The naive string-matching algorithm
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

// 32.2: The Rabin-Karp Algorithm
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

// 32.3 String matching with finite automata
template <class Iter>
struct FiniteAutomata
{
    static constexpr auto n = 256;

    constexpr static auto to_value(uint8_t c) noexcept
    {
        return static_cast<int>(c);
    }

    constexpr static auto to_letter(int x) noexcept
    {
        assert(0 <= x && x < n);
        return static_cast<char>(x);
    }

    FiniteAutomata(Iter first, Iter last) noexcept
        : s_first{first}, s_last{last}, m(std::distance(s_first, s_last))
    {
        // TODO(plesslie): probably better to just check in operator() for
        // tt.empty(), but this is just proof-of-concept for now
        if (s_first == s_last) {
            // empty pattern
            m = 1;
            tt.insert(tt.cend(), m*n, m);
            return;
        }

        std::string P_qA;
        P_qA.reserve(m);

        tt.insert(tt.cend(), m*n, 0);
        assert(tt.size() == m*n);

        for (int q = 0; q < m; ++q) {
            // q := length of pattern

            assert((s_first + q) <= s_last);
            // P_qA += *(s_first + q);
            P_qA = { s_first, s_first + q };

            for (int a = 0; a < n; ++a) {
                // a := next (potential) input character in the text
                P_qA += to_letter(a);

                // find longest `k` s.t. P_k is suffix of P_qA
                int k = std::min(m, q + 1);
                while (!is_suffix(s_first, s_first + k, P_qA)) {
                    assert(0 <= k && k <= m);
                    --k;
                }

                // std::cout << "q=" << q << ", a=" << a << ", PqA=" << P_qA << ", k=" << k << '\n';
                tt[ix(q, a)] = k;

                P_qA.pop_back();
            }
        }
    }

    void dump(std::ostream& os) const
    {
        os << "  |";
        for (int i = 0; i < n; ++i) {
            os << " " << std::setw(2) << i;
        }
        os << '\n';
        for (int i = 0; i < 3*n+3; ++i) {
            os << '-';
        }
        os << '\n';

        for (int i = 0; i < m; ++i) {
            os << std::setw(2) << i << '|';
            for (int j = 0; j < n; ++j) {
                os << ' ' << std::setw(2) << tt[ix(i, j)];
            }
            os << '\n';
        }

        os << "\nTable dump:";
        for (auto i = 0u; i < tt.size(); ++i) {
            os << ' ' << tt[i];
        }
        os << '\n';
    }

    /*static*/ bool is_suffix(Iter first, Iter last, const std::string& p) const noexcept
    {
        assert((int)std::distance(first, last) <= (int)p.size());
        auto it = p.cend();
        while (last > first) {
            --last;
            --it;
            if (*last != *it) {
                return false;
            }
        }
        return true;
    }

    /*static*/ constexpr size_t ix(size_t s, size_t a) const noexcept
    {
        // s := current state
        // a := next input character
        // r := next state
        assert(0 <= s && s < m);
        assert(0 <= a && a < n);
        auto r = s*n + a;
        assert(0 <= r && r <= tt.size());
        return r;
    }

    std::pair<Iter, Iter> operator()(Iter first, Iter last) const noexcept
    {
        int s = 0;
        int a;
        for (; first != last; ++first) {
            a = to_value(*first);
            s = tt[ix(s, a)];
            if (s == m) {
                return std::make_pair(first - m, first);
            }
        }
        return std::make_pair(last, last);
    }

    Iter s_first, s_last;
    int m; // max state
    std::vector<int> tt;
};

} // namespace pl
