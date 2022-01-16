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
    char& operator[](std::size_t n) {
        return buf[n];
    }

    const char& operator[](std::size_t n) const {
        return buf[n];
    }

    char buf[sizeof(T)] alignas(N);
};

// TODO(peter): test this as a customization point, e.g. for optional<T&>
template <class T>
struct storage
{
    storage() = default;

    storage(T t)
    {
        emplace(std::move(t));
    }

    template <class... Args>
    explicit storage(Args&&... args)
    {
        emplace(std::forward<Args>(args)...);
    }

    ~storage() noexcept
    {
        destroy_if_engaged();
    }

    template <class... Args>
    auto emplace(Args&&... args) -> void
    {
        construct_at<T>(&buf[0], std::forward<Args>(args)...);
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

template <class T>
class optional
{
public:
    optional() noexcept : impl{} {}

    optional(T t) : impl{std::move(t)} {}

    template <class... Args>
    explicit optional(Args&&... args) : impl{std::forward<Args>(args)...} {}

    explicit operator bool() const noexcept { return impl.is_engaged(); }

    auto is_engaged() const noexcept -> bool { return impl.is_engaged(); }

    auto operator*() noexcept -> T&
    {
        assert(is_engaged());
        return *impl.ptr();
    }

    auto operator*() const noexcept -> T const&
    {
        assert(is_engaged());
        return *impl.ptr();
    }

    auto operator->() noexcept -> T*
    {
        assert(is_engaged());
        return impl.ptr();
    }

    auto operator->() const noexcept -> T const*
    {
        assert(is_engaged());
        return impl.ptr();
    }

private:
    detail::storage<T> impl;
};

template <class T, class... Args>
auto make_optional(Args&&... args) -> optional<T>
{
    return optional<T>(std::forward<Args>(args)...);
}

}  // namespace pl
