#pragma once

#include <cstddef>
#include <memory>

namespace pl {

namespace detail {

// TODO: use std::construct_at if available
template <class T, class... Args>
T* construct_at(void* p, Args&&... args)
{
    return new (p) T{std::forward<Args>(args)...};
}

template <class T, std::size_t N = alignof(T)>
struct storage_for
{
    char* operator[](std::size_t n) {
        return &buf[n];
    }

    const char* operator[](std::size_t n) const {
        return &buf[n];
    }

    char buf[sizeof(T)] alignas(N);
};

// TODO(peter): test this as a customization point, e.g. for optional<T&>
template <class T>
struct storage
{
    storage() = default;

    ~storage() noexcept
    {
        destroy_if_engaged();
    }

    template <class... Args>
    auto emplace(Args&&... args) -> void
    {
        construct_at(&buf, std::forward<Args>(args)...);
        engaged = true;
    }

    auto destroy_if_engaged() noexcept -> void
    {
        if (engaged) {
            destroy();
        }
    }

    auto destroy() noexcept -> void
    {
        std::destroy_at(ptr());
        engaged = false;
    }

    auto ptr() noexcept -> T*
    {
        assert(engaged);
        return reinterpret_cast<T*>(&buf[0]);
    }

    auto ptr() const noexcept -> T const*
    {
        assert(engaged);
        return reinterpret_cast<T const*>(&buf[0]);
    }

    auto is_engaged() const noexcept -> bool
    {
        return engaged;
    }

    bool engaged = false;
    storage_for<T> buf = {};
};

} // namespace detail

#if 0
template <class T>
struct optional
{
    optional();

    explicit optional(T t);

    template <class... Args>
    optional(Args&&... args);


};
#endif

}  // namespace pl
