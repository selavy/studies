#include <super.h>
#include <stdlib.h>
#include <stdio.h>

struct SuperImpl
{
    int a;
    int b;
};
typedef struct SuperImpl SuperImpl;

#define AS_IMPL(x)  ((SuperImpl*)(x))
#define AS_CIMPL(x) ((const SuperImpl*)(x))

Super* super_new()
{
    Super* s = (Super*)malloc(sizeof(*s));
    if (!s) {
        return NULL;
    }
    if (super_init(s) != 0) {
        free(s);
        return NULL;
    }
    return s;
}

void super_del(Super* x)
{
    if (x) {
        super_destroy(x);
    }
    free(x);
}

void super_destroy(Super* s)
{
    SuperImpl* si = AS_IMPL(s);
    si->a = 0;
    si->b = 0;
}

int super_init(Super* s)
{
    SuperImpl* si = AS_IMPL(s);
    si->a = 0;
    si->b = 0;
    return 0;
}

int super_calc(Super* s, int x)
{
    SuperImpl* si = AS_IMPL(s);
    si->a = x;
    si->b = x + 1;
    return si->a + si->b;
}

void super_print(Super* s)
{
    SuperImpl* si = AS_IMPL(s);
    printf("----------------------------------\n");
    printf("| Super\n");
    printf("|     a = %d\n", si->a);
    printf("|     b = %d\n", si->b);
    printf("----------------------------------\n");
}
