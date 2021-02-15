#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define SUPER_API extern

typedef void Super;

SUPER_API Super* super_new();
SUPER_API void   super_del(Super* x);
SUPER_API void   super_destroy(Super* s);
SUPER_API int    super_init(Super* s);
SUPER_API int    super_calc(Super* s, int x);
SUPER_API void   super_print(Super* s);

#ifdef __cplusplus
}  // extern "C"
#endif
