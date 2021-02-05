#include "cstr.h"
#include <string.h>
#include <stdlib.h>

cstr* cstr_new(const char* const s, const size_t len)
{
    cstr* str = malloc(sizeof(*str));
    if (!str) {
        return NULL;
    }
    str->o.data = malloc(sizeof(*str->o.data) * (len + 1));
    if (!str->o.data) {
        free(str);
        return NULL;
    }
    memcpy(str->o.data, s, sizeof(*s) * len);
    str->o.capacity = len;
    str->size = len;
    return str;
}

void cstr_del(cstr* s)
{
    if (!s) {
        return;
    }
    if (!cstr_isinline_(s)) {
        free(s->o.data);
#ifndef NDEBUG
        s->o.data = NULL;
#endif
    }
    free(s);
}

size_t cstr_size(const cstr* s)
{
    return s->size;
}

size_t cstr_len(const cstr* s)
{
    return cstr_size(s);
}

size_t cstr_length(const cstr* s)
{
    return cstr_size(s);
}

size_t cstr_capacity(const cstr* s)
{
    return cstr_isinline_(s) != 0 ?
        CSTR_INLINE_SIZE : s->o.capacity;
}

const char* cstr_str(const cstr* s)
{
    return cstr_isinline_(s) != 0 ?
        s->data : s->o.data;
}

char* cstr_mstr(cstr* s)
{
    return cstr_isinline_(s) != 0 ?
        s->data : s->o.data;
}

int cstr_isinline_(const cstr* s)
{
    return 0;
}
