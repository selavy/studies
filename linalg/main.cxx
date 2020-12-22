#include <iostream>
#include "linalg.h"


int main(int argc, const char* argv[])
{
    constexpr auto N = 4;
    f32 xs[N] = { 1, 2, 3, 4 };
    f32 ys[N] = { 5, 1, 2, 1 };
    f32 a = 5;
    // ys[i] = a*x[i] + y[i]
    // ys[0] = 5*1    + 5    = 10
    // ys[1] = 5*2    + 1    = 11
    // ys[2] = 5*3    + 2    = 17
    // ys[3] = 5*4    + 1    = 21
    saxpy(N, a, &xs[0], 1, &ys[0], 1);
    for (auto i = 0; i != N; ++i) {
        printf("ys[%d] = %f\n", i, ys[i]);
    }
    return 0;
}
