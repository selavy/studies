#pragma once
#include "util.h"

namespace v2
{

namespace impl {

struct VTable
{
    void* (*create)(const void*);
    void  (*destroy)(void*);
    void* (*clone)(const void*);
    const std::type_info& (*type_info)(const void*);
};

struct Impl {
    template <class T>
    Impl(const T& t) : tbl_(vtable<T>()), val_(tbl_->create(&t)) {}

    ~Impl() noexcept { tbl_->destroy(val_); }

    Impl(const Impl& other) {
        tbl_->destroy(val_);
        tbl_ = other.tbl_;
        val_ = tbl_->clone(other.val_);
    }

    template <class T>
    const VTable& vtable() const {
        static VTable t{
            .create = +[](void* v) { return new T{static_cast<T*>(v)}; },
            .destroy =
                +[](void* v) {
                    auto* x = static_cast<T*>(v);
                    delete x;
                },
            .clone =
                +[](void* v) {
                    auto* x = static_cast<T*>(v);
                    return new T{*x};
                },
            .type_info =
                +[](void* v) {
                    auto* x = static_cast<T*>(v);
                    return x->type_info();
                },
        };
        return t;
    }

    const std::type_info& type_info() const {
        return tbl_->type_info(val_);
    }

    const VTable* tbl_ = nullptr;
    void* val_ = nullptr;
};

} // ~impl

class Any
{
public:
    Any() noexcept = default;

    template <class T>
    explicit Any(const T& t) : val_{std::make_unique<impl::Impl<T>>(t)} {}

    Any(const Any& other) : val_{std::move(other.clone())} {}

    bool has_value() const noexcept { return val_ != nullptr; }

    void reset() noexcept { val_.reset(); }

    template <class T> friend       T  any_cast(Any& operand);
    template <class T> friend       T  any_cast(const Any& operand);
    template <class T> friend       T* any_cast(Any* operand);
    template <class T> friend const T* any_cast(const Any* operand);

private:
    std::unique_ptr<impl::Base> clone() const
    {
        return val_ ? val_->clone() : nullptr;
    }

    template <class U>
    U* cast() noexcept
    {
        if (!has_value()) {
            return nullptr;
        }
        if (val_->type_info() == typeid(U)) {
            return static_cast<U*>(val_->addr());
        }
        return nullptr;
    }

    std::unique_ptr<impl::Base> val_;
};

template <class T>
T* any_cast(Any* operand)
{
    return operand ? operand->cast<T>() : nullptr;
}

template <class T>
const T* any_cast(const Any* operand)
{
    return operand ? operand->cast<T>() : nullptr;
}


template <class T>
T any_cast(Any& operand)
{
    if (auto* p = any_cast<T>(&operand)) {
        return *p;
    }
    throw bad_any_cast{};
}

template <class T>
T any_cast(const Any& operand)
{
    if (auto* p = any_cast<T>(&operand)) {
        return *p;
    }
    throw bad_any_cast{};
}

} // ~v2
