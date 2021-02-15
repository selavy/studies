#include <iostream>
#include <super/super.h>

int main(int argc, const char** argv)
{
    (void)argc;
    (void)argv;

    auto s = super::make(1, 2);

    if (s.x == 0) {
        std::cout << "s.x == 0" << std::endl;
    } else {
        std::cout << "s.x != 0" << std::endl;
    }

    return 0;
}
