#include <iostream>

#include "util.h"
#include "v1.h"
// #include <any>

#if 0

struct AnyBase
{
    virtual ~AnyBase() noexcept = default;
    virtual std::unique_ptr<AnyBase> clone() const = 0;
};

template <class T>
struct AnyImpl : AnyBase
{
    explicit AnyImpl(T t) : t_(std::move(t)) {}

    std::unique_ptr<AnyBase> clone() const override
    {
        return std::unique_ptr<AnyBase>{new AnyImpl<T>(t_)};
    }

          T* get()       noexcept { return &t_; }
    const T* get() const noexcept { return &t_; }

    T t_;
};

struct Any
{
    Any() noexcept = default;

    template <class T>
    Any(T t) : val_(std::make_unique<AnyImpl<T>>(std::move(t))) {}

    Any(const Any& other) : val_{std::move(other.clone())} {}

    bool has_value() const noexcept { return val_ != nullptr; }

    void reset() noexcept { val_.reset(); }

    template <class T> friend       T  any_cast(Any& operand);
    template <class T> friend       T  any_cast(const Any& operand);
    template <class T> friend       T* any_cast(Any* operand);
    template <class T> friend const T* any_cast(const Any* operand);


private:
    std::unique_ptr<AnyBase> clone() const
    {
        return val_ ? val_->clone() : nullptr;
    }

    template <class T>
    T* cast() noexcept
    {
        if (auto* p = dynamic_cast<AnyImpl<T>*>(val_.get())) {
            return p->get();
        } else {
            return nullptr;
        }
    }

    std::unique_ptr<AnyBase> val_;
};

template <class T>
T* any_cast(Any* operand)
{
    if (auto* p = dynamic_cast<AnyImpl<T>*>(operand->val_.get())) {
        return p->get();
    } else {
        return nullptr;
    }
}

template <class T>
const T* any_cast(const Any* operand)
{
    if (auto* p = dynamic_cast<AnyImpl<T>*>(operand->val_.get())) {
        return p->get();
    } else {
        return nullptr;
    }
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

namespace sbo {

struct AnyBase
{
    using CopyFn = std::unique_ptr<AnyBase> (*)(const AnyBase*);
    using DtorFn = void (*)(AnyBase*);


    CopyFn copy_;
    DtorFn dtor_;
};

} // ~sbo

#endif

template <class T>
void print(T* t) {
    if (t) {
        std::cout << *t;
    } else {
        std::cout << "unknown";
    }
}

// template <class Any, class F>
// void test(F any_cast)
// {
//     auto a = Any(12);
//
//     std::cout << "value<int>: ";
//     print(any_cast<int>(&a));
//     std::cout << std::endl;
//
//     std::cout << "value<double>: ";
//     print(any_cast<double>(&a));
//     std::cout << std::endl;
//
//     auto b = a;
//     std::cout << "value<int>: ";
//     print(any_cast<int>(&b));
//     std::cout << std::endl;
//
//     std::cout << "value<int>: " << any_cast<int>(b) << std::endl;
// }

// constexpr bool kDebug = true;
constexpr bool kDebug = false;

#define TEST(Any, any_cast)                                                 \
    do {                                                                    \
        int expect = 12;                                                    \
        auto a = Any(expect);                                               \
        auto* t = any_cast<int>(&a);                                        \
        if (kDebug) {                                                       \
            std::cout << "value<int>: ";                                    \
            print(t);                                                       \
            std::cout << std::endl;                                         \
        }                                                                   \
        if (!t) {                                                           \
            std::cerr << "ERROR: any_cast<int> returned null" << std::endl; \
            exit(EXIT_FAILURE);                                             \
        }                                                                   \
        if (*t != expect) {                                                 \
            std::cerr                                                       \
                << "ERROR: any_cast<int>() did not return original value: " \
                << *t << " != " << expect << std::endl;                     \
            exit(EXIT_FAILURE);                                             \
        }                                                                   \
        auto* d = any_cast<double>(&a);                                     \
        if (kDebug) {                                                       \
            std::cout << "value<double>: ";                                 \
            print(d);                                                       \
            std::cout << std::endl;                                         \
        }                                                                   \
        if (d) {                                                            \
            std::cerr << "any_cast<double>(int) returned a value: " << *d   \
                      << std::endl;                                         \
            exit(EXIT_FAILURE);                                             \
        }                                                                   \
        auto b = a;                                                         \
        auto* x = any_cast<int>(&b);                                        \
        if (kDebug) {                                                       \
            std::cout << "value<int>: ";                                    \
            print(x);                                                       \
            std::cout << std::endl;                                         \
        }                                                                   \
        auto e = any_cast<int>(b);                                          \
        if (kDebug) {                                                       \
            std::cout << "value<int>: " << e << std::endl;                  \
        }                                                                   \
        if (e != expect) {                                                  \
            std::cerr << "any_cast<int>() returned wrong value: " << e      \
                      << " != " << expect << std::endl;                     \
        }                                                                   \
        std::cout << "PASSED.\n";                                           \
    } while (0)

int main(int argc, const char* argv[]) {
    (void)argc;
    (void)argv;

    TEST(v1::Any, v1::any_cast);

#if 0
    auto a = Any(12);

    std::cout << "value<int>: ";
    print(any_cast<int>(&a));
    std::cout << std::endl;

    std::cout << "value<double>: ";
    print(any_cast<double>(&a));
    std::cout << std::endl;

    auto b = a;
    std::cout << "value<int>: ";
    print(any_cast<int>(&b));
    std::cout << std::endl;

    std::cout << "value<int>: " << any_cast<int>(b) << std::endl;
#endif

    return 0;
}
