#pragma once



template <class Id, class Value>
class IdMap
{
    using key_type    = Id;
    using mapped_type = Value;
    using index_type  = std::size_t;
    using size_type   = std::size_t;

    static_assert(std::is_nothrow_move_constructible_v<Value>,
            "IdMap only supports nothrow move constructible type");
    static_assert(std::is_convertible_v<Id, index_type>,
            "Must be able to convert Id type to index_type");
    static_assert(std::is_default_constructible_v<Value>);

    IdMap() noexcept = default;
    // IdMap(std::size_t n_slots) noexcept

    template <class F>
    Value& get_or_assign(Id id, F&& or_else)
    {
        const auto index = static_cast<index_type>(id);
        if (!(index < size())) {
            grow(index + 1);
            new (&m_begin[index]) Value{or_else()};
        }
        return get(id);
    }

    Value& get(Id id) const noexcept
    {
        const auto index = static_cast<index_type>(id);
        assert(index < size());
        return m_begin[index];
    }

    template <class... Args>
    Value& insert(Id id, Args&&... args)
    {
        const auto index = static_cast<index_type>(id);
        if (!(index < size())) {
            grow(index + 1);
        }
        new (&m_begin[index]) Value{std::forward<Args>(args)...};
        return get(index);
    }

    // jmp::Optional<Value> get(Id id) const
    // {
    // }

    size_type size() const noexcept
    {
        return m_end - m_begin;
    }

    size_type capacity() const noexcept
    {
        return m_capacity - m_begin;
    }

private:
    void grow(std::size_t minsize) noexcept
    {
        const auto cursize = size();
        const auto newcapacity = std::max(minsize, (capacity() + 1) * 1.5);
        assert(newcapacity > capacity());
        // TODO: check malloc_usable_size
        // TODO: use reallocarrary if Value is trivial
        auto* begin = static_cast<Value*>(calloc(cursize, sizeof(Value)));
        assert(begin != nullptr); // TODO: decide if going to throw std::bad_alloc{}
        std::uninitialized_move_n(m_begin, cursize, begin);
        m_begin    = begin;
        m_end      = begin + cursize;
        m_capacity = begin + newcapacity;
    }

    // TODO: add small vector optimization
    Value* m_begin = nullptr;
    Value* m_end = nullptr;
    Value* m_capacity = nullptr;
};
