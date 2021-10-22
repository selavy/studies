#include <iostream>
#include <memory>
#include <type_traits>
#include <stdexcept>
// #include <any>

struct bad_any_cast : std::runtime_error
{
    bad_any_cast() noexcept = default;
};

struct AnyBase
{
    virtual ~AnyBase() noexcept = default;
};

template <class T>
struct AnyImpl : AnyBase
{
    explicit AnyImpl(T&& t) : t_(std::forward<T>(t)) {}

    T* get() noexcept { return &t_; }

    T t_;
};

struct Any
{
    Any() noexcept = default;

    template <class T>
    Any(T&& t) : val_(std::make_unique<AnyImpl<T>>(std::forward<T>(t))) {}

    bool has_value() noexcept { return val_ != nullptr; }

    void reset() noexcept { val_.reset(); }

    template <class T> friend       T  any_cast(Any& operand);
    template <class T> friend       T  any_cast(const Any& operand);
    template <class T> friend       T* any_cast(Any* operand);
    template <class T> friend const T* any_cast(const Any* operand);


private:
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
T any_cast(Any& operand)
{
    if (auto* p = dynamic_cast<AnyImpl<T>*>(operand.val_.get())) {
        return *p->get();
    } else {
        throw bad_any_cast{};
    }
}

template <class T>
T any_cast(const Any& operand)
{
    if (auto* p = dynamic_cast<AnyImpl<T>*>(operand.val_.get())) {
        return *p->get();
    } else {
        throw bad_any_cast{};
    }
}

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


#if 0
namespace sbo {

struct SmallStorage
{
    constexpr static std::size_t kMaxSize = 32;

    constexpr SmallStorage() noexcept {}

    template <class T>
    explicit SmallStorage(T t) noexcept(std::is_nothrow_move_constructible_v<T>)
    {
        new (&buf[0]) T{std::move(t)};
    }

    std::array<char, kMaxSize> buf = {};

};

struct LargeStorage
{

}

} // ~sbo
#endif

template <class T>
void print(T* t)
{
    if (t) {
        std::cout << *t;
    } else {
        std::cout << "unknown";
    }
}

int main(int argc, const char* argv[]) {
    (void)argc;
    (void)argv;

    // auto ss = sbo::SmallStorage(1);

    auto a = Any(12);

    std::cout << "value<int>: ";
    print(any_cast<int>(&a));
    std::cout << std::endl;

    std::cout << "value<double>: ";
    print(any_cast<double>(&a));
    std::cout << std::endl;

    return 0;
}
