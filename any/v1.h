#pragma once

#include "util.h"

namespace v1
{

namespace impl
{

struct Base
{
    virtual ~Base() noexcept = default;
    virtual std::unique_ptr<Base> clone() const = 0;
    virtual void* addr() = 0;
    virtual const std::type_info& type_info() const = 0;
};

template <class T>
struct Impl : public Base
{
    explicit Impl(const T& t) : val_(t) {}

    std::unique_ptr<Base> clone() const override
    {
#if 0
        return std::make_unique<Impl<T>>(val_);
#else
        return std::unique_ptr<Base>{new Impl<T>(val_)};
#endif
    }

    void* addr() override
    {
        return &val_;
    }

    const std::type_info& type_info() const override
    {
        return typeid(T);
    }

    T val_;
};

}  // ~impl

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

}  // ~v1
