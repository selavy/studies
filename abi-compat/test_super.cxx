#include <iostream>
#include "super.h"

int main(int argc, const char** argv)
{
    (void)argc;
    (void)argv;

    auto* super = super_new();
    if (!super) {
        super_del(super);
        std::cerr << "error: failed to create super object" << std::endl;
        return 0;
    }
    auto result = super_calc(super, 42);
    std::cout << "result: " << result << "\n";
    super_print(super);
    super_del(super);
    return 0;
}
