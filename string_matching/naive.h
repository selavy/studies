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

template <class Iter>
Iter search(Iter first, Iter last, Iter s_first, Iter s_last)
{
    return search(first, last, s_first, s_last, std::forward_iterator_tag{});
}

// namespace detail {
// 
// bool is_match(const char* p1, const char* p2, const char* e2)
// {
//     for (; p2 != e2; ++p1, ++p2) {
//         if (*p1 != *p2) {
//             return false;
//         }
//     }
//     return true;
// }
// 
// } // namespace detail
// 
// const char* search(const char* const first, const char* const last,
//         const char* const s_first, const char* const s_last) noexcept
// {
//     auto n = last - first;
//     auto m = s_last - s_first;
//     if (m > n) {
//         return last;
//     }
//     auto end = first + (n - m);
// 
//     for (auto p = first; p != end; ++p) {
//         if (detail::is_match(p, s_first, s_last)) {
//             return p;
//         }
//     }
// 
//     return last;
// }

} // namespace naive
