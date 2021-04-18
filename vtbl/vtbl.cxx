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

enum Index
{
    DTR = 0,
    FOO,
    BAR,
    N_ENTRIES,
};

struct VirtBase { void** vtbl; };
struct BaseC    { VirtBase v; };
struct DerivedC { VirtBase v; };

void BaseC_dtr_(BaseC* b);
void BaseC_foo_(BaseC* b);
int  BaseC_bar_(BaseC* b);

void DerivedC_dtr_(DerivedC* b);
void DerivedC_foo_(DerivedC* b);
int  DerivedC_bar_(DerivedC* b);

void* VTable_BaseC[N_ENTRIES] = {
    [DTR] = (void*)&BaseC_dtr_,
    [FOO] = (void*)&BaseC_foo_,
    [BAR] = (void*)&BaseC_bar_,
};

void* VTable_DerivedC[N_ENTRIES] = {
    [DTR] = (void*)&DerivedC_dtr_,
    [FOO] = (void*)&DerivedC_foo_,
    [BAR] = (void*)&DerivedC_bar_,
};

void BaseC_ctr(BaseC* b)
{
    b->v.vtbl = VTable_BaseC;
    std::cout << "BaseC::ctr()\n";
}

void BaseC_dtr_(BaseC* b)
{
    std::cout << "BaseC::dtr()\n";
}

void BaseC_foo_(BaseC* b)
{
    std::cout << "BaseC::foo()\n";
}

int BaseC_bar_(BaseC* b)
{
    std::cout << "BaseC::bar()\n";
    return 0;
}

void DerivedC_ctr(DerivedC* b)
{
    // Set VTable pointer (so can call virtual functions in initializer list)
    // Call BaseC constructor

    BaseC_ctr((BaseC*)b);
    b->v.vtbl = VTable_DerivedC;
    std::cout << "DerivedC::ctr()\n";
}

void DerivedC_dtr_(DerivedC* b)
{
    BaseC_dtr_((BaseC*)b);
    std::cout << "DerivedC::dtr()\n";
}

void DerivedC_foo_(DerivedC* b)
{
    std::cout << "DerivedC::foo()\n";
}

int DerivedC_bar_(DerivedC* b)
{
    std::cout << "DerivedC::bar()\n";
    return 1;
}

void call_foo(void* x)
{
    VirtBase* v = (VirtBase*)x;
    void*     p = v->vtbl[FOO];
    (*(void (*)())p)();
}

void call_dtr(void* x)
{
    VirtBase* v = (VirtBase*)x;
    void*     p = v->vtbl[DTR];
    (*(void (*)(void*))p)(x);
}

int main(int argc, char** argv)
{
    {
        std::cout << "---------------------------------------------------\n";
        std::cout << "| C++ Version\n";
        std::cout << "---------------------------------------------------\n";

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
    }

    {
        std::cout << "---------------------------------------------------\n";
        std::cout << "| C Version\n";
        std::cout << "---------------------------------------------------\n";

        // Base::foo() via base
        {
            std::cout << "---------------------------------------------------\n";
            std::cout << "| Base::foo() via base pointer\n";
            std::cout << "---------------------------------------------------\n";
            BaseC b;
            BaseC_ctr(&b);
            call_foo(&b);
            call_dtr(&b);
        }

        // Derived::foo() via base
#if 1
        {
            std::cout << "---------------------------------------------------\n";
            std::cout << "| Derived::foo() via base pointer\n";
            std::cout << "---------------------------------------------------\n";
            DerivedC b;
            DerivedC_ctr(&b);
            call_foo(&b);
            call_dtr(&b);
        }
#endif

        // Derived::foo() via base
#if 0
        {
            std::cout << "---------------------------------------------------\n";
            std::cout << "| Derived::foo() via derived pointer\n";
            std::cout << "---------------------------------------------------\n";
            auto b = std::make_unique<DerivedCxx>();
            b->foo();
        }
#endif
    }
    return 0;
}
