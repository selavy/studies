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


// struct storage
// {
//     template <class... Args> auto construct(Args&&... args) -> void;
//     auto destroy() -> void;
//     auto lvalue() -> T&;
//     auto lvalue() -> T const&;
//     auto rvalue() -> T&&;
//     auto is_engaged() -> bool;
// }

// TODO(peter): test this as a customization point, e.g. for optional<T&>
template <class T>
struct storage
{
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

// TODO: implement storage<void>
#if 0
template <>
struct storage<void>
{
    using value_type = Void;

    storage() = default;

    ~storage() noexcept = default;

    template <class... Args>
    auto emplace(Args&&... args) -> void
    {
        engaged = true;
    }

    auto destroy() noexcept -> void
    {
        engaged = false;
    }

    auto ptr() noexcept -> void*
    {
        assert(engaged);
        return nullptr;
    }

    auto ptr() const noexcept -> void const*
    {
        assert(engaged);
        return nullptr;
    }

    auto is_engaged() const noexcept -> bool
    {
        return engaged;
    }

    bool engaged = false;
};
#endif

#if 0
template <class T>
struct storage<T&>
{
    using value_type = T&;
    using U = std::remove_reference_t<T>;

    storage() = default;

    storage(T t)
    {
        emplace(t);
    }

    ~storage() = default;

    void emplace(T t)
    {
        ptr_ = &t;
    }

    auto destroy() noexcept -> void
    {
        ptr_ = nullptr;
    }

    auto ptr() noexcept -> U*
    {
        assert(is_engaged());
        return ptr_;
    }

    auto ptr() const noexcept -> U const*
    {
        assert(is_engaged());
        return ptr_;
    }

    auto is_engaged() const noexcept -> bool
    {
        return ptr_ != nullptr;
    }

    U* ptr_;
};
#endif

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

    template <class... Args,
             REQUIRES(std::is_constructible_v<T, Args...>)>
    // explicit optional(Args&&... args) : impl{std::forward<Args>(args)...} {}
    explicit optional(Args&&... args) { impl.construct(std::forward<Args>(args)...); }

    explicit operator bool() const noexcept { return impl.is_engaged(); }

    template <class... Args,
             REQUIRES(std::is_constructible_v<T, Args...>)>
    auto emplace(Args&&... args) -> void
    {
        impl.destroy();
        impl.construct(std::forward<Args>(args)...);
    }

    auto is_engaged() const noexcept -> bool { return impl.is_engaged(); }

    auto operator*() noexcept -> T&
    {
        assert(is_engaged());
        return impl.lvalue();
    }

    auto operator*() const noexcept -> T const&
    {
        assert(is_engaged());
        return impl.lvalue();
    }

    auto operator->() noexcept -> T*
    {
        assert(is_engaged());
        return &impl.lvalue();
    }

    auto operator->() const noexcept -> T const*
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
