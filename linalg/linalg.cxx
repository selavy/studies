#include "linalg.h"

void saxpy(int n, f32 a, const f32* x, const int incx, f32* y, const int incy)
{
    int ix = 0, iy = 0;
    for (int i = 0; i != n; ++i) {
        y[iy] = a*x[ix] + y[iy];
        ix += incx;
        iy += incy;
    }
}

void sdot(int n, const f32* x, int xinc, const f32* y, int incy)
{
    f32 r = 0;
    int ix = 0, iy = 0;
    for (int i = 0; i != n; ++i) {
        r += x[ix] * y[iy];
    }
    return r;
}
