#pragma once

#include <stddef.h>

#define LINALG_EXTERN extern



using f32 = float;
using f64 = double;

// y := a*x + y
// x: vec[f32]
// y: vec[f32]
// a: f32
LINALG_EXTERN
void saxpy(int n, f32 a, const f32* x, int incx, f32* y, int incy);

LINALG_EXTERN
void sdot(int n, const f32* x, int xinc, const f32* y, int incy);
