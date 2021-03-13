#include <iostream>
#include <memory>

struct BaseCxx
{
    BaseCxx() noexcept          { std::cout << "BaseCxx\n"; }
    explicit BaseCxx(int x) noexcept { std::cout << "BaseCxx: " << x << "\n"; }
    virtual ~BaseCxx() noexcept { std::cout << "~BaseCxx\n"; }
    virtual void foo() { std::cout << "BaseCxx::foo()\n"; }
    virtual int  bar() { std::cout << "BaseCxx::bar()\n"; return 0; }
};

struct DerivedCxx : BaseCxx
{
    // DerivedCxx() noexcept          { std::cout << "DerivedCxx\n"; }
    DerivedCxx() noexcept
        : BaseCxx(bar())
    {
        std::cout << "DerivedCxx\n";
    }
    virtual ~DerivedCxx() noexcept { std::cout << "~DerivedCxx\n"; }
    void foo() override { std::cout << "DerivedCxx::foo()\n"; }
    int  bar() override { std::cout << "DerivedCxx::bar()\n"; return 1; }
};

struct BaseC    { void* vtbl; };
struct DerivedC { void* vtbl; };

enum VTableIndex
{
    DTR = 0,
    FOO,
    BAR,
    N_ENTRIES,
};

void BaseC_foo_impl(BaseC* x)
{
    std::cout << "BaseC::foo()\n";
}

int BaseC_bar_impl(BaseC* x)
{
    std::cout << "BaseC::bar()\n";
    return 0;
}

void BaseC_dtor_impl(BaseC* x)
{
    std::cout << "~BaseC\n";
}

void* BaseC_VTable[N_ENTRIES] = {
    [DTR] = (void*)&BaseC_dtor_impl,
    [FOO] = (void*)&BaseC_foo_impl,
    [BAR] = (void*)&BaseC_bar_impl,
};

void BaseC_ctor(BaseC* x)
{
    x->vtbl = &BaseC_VTable;
    std::cout << "BaseC\n";
}

void do_dtor(void* x)
{
    BaseC* xx = (BaseC*)x;
    xx->vtbl[DTR](xx);
}

void do_foo(void* x)
{
    BaseC* xx = (BaseC*)x;
    xx->vtbl[FOO](xx);
}

int do_bar(void* x)
{
    BaseC* xx = (BaseC*)x;
    return ((int (*)(BaseC*))xx->vtbl[BAR])(xx);
}


int main(int argc, char** argv)
{
    // Base::foo() via base
    {
        std::cout << "---------------------------------------------------\n";
        std::cout << "| Base::foo() via base pointer\n";
        std::cout << "---------------------------------------------------\n";
        auto b = std::make_unique<BaseCxx>();
        b->foo();
    }

    // Derived::foo() via base
    {
        std::cout << "---------------------------------------------------\n";
        std::cout << "| Derived::foo() via base pointer\n";
        std::cout << "---------------------------------------------------\n";
        std::unique_ptr<BaseCxx> b{new DerivedCxx};
        b->foo();
    }

    // Derived::foo() via base
    {
        std::cout << "---------------------------------------------------\n";
        std::cout << "| Derived::foo() via derived pointer\n";
        std::cout << "---------------------------------------------------\n";
        auto b = std::make_unique<DerivedCxx>();
        b->foo();
    }

    return 0;
}
