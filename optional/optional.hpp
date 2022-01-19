#pragma once

#include <cstddef>
#include <memory>

// TODO(peter): use a different name
#define REQUIRES(...) typename std::enable_if_t<(__VA_ARGS__), bool> = true

namespace pl {

namespace detail {

struct Void
{
    Void() noexcept = default;
    Void(const Void&) noexcept = default;
    Void(Void&&) noexcept = default;
    Void& operator=(const Void&) noexcept = default;
    Void& operator=(Void&&) noexcept = default;
};

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

    auto ptr()       noexcept -> T*       { return reinterpret_cast<T*>(&buf[0]); }
    auto ptr() const noexcept -> T const* { return reinterpret_cast<T*>(&buf[0]); }

    char buf[sizeof(T)] alignas(N);
};

// -----------------------------------------------------------------------------
// Storage Requirements:
// -----------------------------------------------------------------------------
//
// struct storage
// {
//     template <class... Args>
//     auto construct(Args&&... args) -> void;
//     auto destroy() -> void;
//     auto lvalue() -> T&;
//     auto lvalue() -> T const&;
//     auto rvalue() -> T&&;
//     auto is_engaged() -> bool;
// }

template <class T>
struct storage
{
    storage() noexcept = default;

    ~storage() noexcept
    {
        destroy();
    }

    auto destroy() noexcept -> void
    {
        if (engaged) {
            std::destroy_at(buf.ptr());
            engaged = false;
        }
    }

    template <class... Args>
    void construct(Args&&... args)
    {
        construct_at<T>(&buf[0], std::forward<Args>(args)...);
        engaged = true;
    }

    auto lvalue()       noexcept -> T&        { return *buf.ptr(); }
    auto lvalue() const noexcept -> T const&  { return *buf.ptr(); }
    auto rvalue()       noexcept -> T&&       { return *buf.ptr(); }
    auto is_engaged() const noexcept -> bool  { return engaged; }

    bool engaged = false;
    storage_for<T> buf = {};
};

template <>
struct storage<void> : Void
{
    storage() = default;

    ~storage() noexcept = default;

    auto destroy() noexcept -> void { engaged = false; }

    template <class... Args>
    void construct(Args&&... args) { engaged = true; }

    auto lvalue()       noexcept -> Void&       { return *this; }
    auto lvalue() const noexcept -> Void const& { return *this; }
    auto rvalue()       noexcept -> Void&       { return *this; }
    auto is_engaged() const noexcept -> bool    { return engaged; }

    bool engaged = false;
};

template <class T>
struct storage<T&>
{
    storage() = default;
    ~storage() noexcept = default;
    auto destroy() noexcept -> void { ptr_ = nullptr; }
    void construct(T& t) { ptr_ = &t; }
    auto lvalue()       noexcept -> T&       { return *ptr_; }
    auto lvalue() const noexcept -> T const& { return *ptr_; }
    auto rvalue()       noexcept -> T&       { return *ptr_; }
    auto is_engaged() const noexcept -> bool { return ptr_ != nullptr; }

    T* ptr_;
};

} // namespace detail

template <class T>
class optional
{
    using storage = detail::storage<T>;

public:
    optional() noexcept : impl{} {}

    template <class U,
             REQUIRES(std::is_constructible_v<U, T> && !std::is_void_v<T>)>
    // optional(U t) : impl(std::move(t)) {}
    optional(U t)
    {
        impl.construct(t);
    }

    optional& operator=(std::conditional_t<std::is_void_v<T>, pl::detail::Void, T> t)
    {
        impl.destroy();
        impl.construct(std::move(t));
        return *this;
    }

    optional& operator=(const optional<T>& other)
    {
        impl = other.impl;
        return *this;
    }

    template <class... Args,
             REQUIRES(std::is_constructible_v<T, Args...>)>
    // explicit optional(Args&&... args) : impl{std::forward<Args>(args)...} {}
    explicit optional(Args&&... args) { impl.construct(std::forward<Args>(args)...); }

    explicit operator bool() const noexcept { return impl.is_engaged(); }

    template <class... Args>
    auto emplace(Args&&... args) -> void
    {
        impl.destroy();
        impl.construct(std::forward<Args>(args)...);
    }

    auto is_engaged() const noexcept -> bool { return impl.is_engaged(); }

    auto discard() -> void { impl.destroy(); }

    decltype(auto) operator*() noexcept
    {
        assert(is_engaged());
        return impl.lvalue();
    }

    decltype(auto) operator*() const noexcept
    {
        assert(is_engaged());
        return impl.lvalue();
    }

    auto operator->() noexcept
    {
        assert(is_engaged());
        return &impl.lvalue();
    }

    auto operator->() const noexcept
    {
        assert(is_engaged());
        return &impl.lvalue();
    }

    // auto value_or(U val) const noexcept -> U
    // {
    //     return is_engaged() ? *impl.ptr() : val;
    // }

private:
    storage impl;
};

#if 0
// TODO(plesslie): this is kind of a cop out implementing it this way
template <>
class optional<void>
{
public:
    optional() noexcept = default;
    explicit operator bool() const noexcept { return engaged_; }
    auto is_engaged() const noexcept -> bool { return engaged_; }
    auto emplace() noexcept -> void { engaged_ = true; }

    // TODO(peter): implement copy + move ctor/assignment

private:
    bool engaged_ = false;
};
#endif

template <class T, class... Args>
auto make_optional(Args&&... args) -> optional<T>
{
    return optional<T>(std::forward<Args>(args)...);
}

}  // namespace pl

#undef REQUIRES
