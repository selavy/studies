#include <iostream>
#include <cstdlib>

int main(int argc, const char** argv)
{
    (void)argc;
    (void)argv;

    srand(0);
    printf("value = %d\n", rand());
    printf("value = %d\n", rand());
    printf("value = %d\n", rand());
    printf("value = %d\n", rand());

    return 0;
}
