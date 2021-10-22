#pragma once

#include <cstddef>
#include <cassert>
#include <memory>
#include <malloc.h>

template <class T>
struct SmallVector
{
    using value_type = T;
    using iterator = T*; // TODO: fix me
    using const_iterator = const T*;
    using pointer = T*;
    using reference = T&;
    using const_reference = const T&;
    using size_type = std::size_t;

    static_assert(std::is_nothrow_move_constructible_v<T>,
            "SmallVector only supports nothrow move constructible types");

    SmallVector() noexcept = default;

    ~SmallVector() noexcept
    {
        clear();
    }

    reference       operator[](std::size_t index)       noexcept { return m_begin[index]; }
    const_reference operator[](std::size_t index) const noexcept { return m_begin[index]; }

    void clear() noexcept
    {
        assert(m_begin <= m_end && m_end <= m_capacity);
        std::destroy(m_begin, m_end);
        free(m_begin);
        m_begin = m_end = m_capacity = nullptr;
    }

    bool      empty()    const noexcept { return m_begin == m_end; }
    bool      is_empty() const noexcept { return m_begin == m_end; }
    size_type size()     const noexcept { return m_end      - m_begin; }
    size_type capacity() const noexcept { return m_capacity - m_begin; }

    void reserve(std::size_t n_elems) noexcept
    {
        grow(n_elems);
    }

    void resize(std::size_t n_elems, const T& value = T()) noexcept
    {
        if (n_elems >= size()) {
            grow(n_elems);
            std::uninitialized_default_construct(m_end, m_capacity);
            assert(m_begin <= m_end && m_end == m_capacity);
        } else {
            auto n_end = m_begin + n_elems;
            std::destroy(n_end, m_end);
            m_end = n_end;
        }
    }

    template <class... Args>
    reference emplace_back(Args&&... args) noexcept
    {
        assert(m_begin <= m_end && m_end <= m_capacity);
        if (m_end == m_capacity) {
            grow();
        }
        new (m_end) T{std::forward<Args>(args)...};
        return *m_end++;
    }

    reference push_back(const value_type& v) noexcept
    {
        return emplace_back(v);
    }

    void pop_back() noexcept
    {
        assert(m_end != m_begin);
        std::destroy_at(m_end);
        --m_end;
    }

    reference       front()       noexcept { return *m_begin; }
    const_reference front() const noexcept { return *m_begin; }

    reference       back()       noexcept { return *(m_end - 1); }
    const_reference back() const noexcept { return *(m_end - 1); }

private:
    void grow(std::size_t n_elems = 16u) noexcept
    {
        assert(m_begin <= m_end && m_end <= m_capacity);
        auto have  = capacity();
        auto want  = std::max<std::size_t>(have * 1.5, n_elems);
        auto avail = malloc_usable_size(m_begin) / sizeof(T);
        if (avail >= want) {
            return;
        }
        auto size_ = size();
        auto* p = static_cast<T*>(calloc(want, sizeof(T)));
        std::uninitialized_move_n(m_begin, size_, p);
        m_end      = p + size_;
        m_capacity = p + want;
        m_begin    = p;
        assert(size()     == size_);
        assert(capacity() == want);
        assert(m_begin <= m_end && m_end < m_capacity);
    }

private:
    T* m_begin    = nullptr;
    T* m_end      = nullptr;
    T* m_capacity = nullptr;
};
