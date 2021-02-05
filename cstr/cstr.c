#include "cstr.h"
#include <string.h>
#include <stdlib.h>
#include <assert.h>

void cstr_free_(void* p, size_t size)
{
    free(p);
}

static const struct cstr_alloc_t malloc_allocator_ =
{
    .calloc  = &calloc,
    .realloc = &realloc,
    .free    = &cstr_free_,
};

static struct cstr_alloc_t cstr_allocator_ = malloc_allocator_;

static void* calloc_(size_t nmemb, size_t size)
{
    return cstr_allocator_.calloc(nmemb, size);
}

static void* realloc_(void* p, size_t size)
{
    // TODO: fall back to calloc if no realloc provided
    return cstr_allocator_.realloc(p, size);
}

static void free_(void* p, size_t size)
{
    cstr_allocator_.free(p, size);
}

cstr* cstr_new(const char* const s, const size_t len)
{
    cstr* str = calloc_(1, sizeof(*str));
    if (!str) {
        return NULL;
    }
    return cstr_init(str, s, len);
}

cstr* cstr_init(cstr* str, const char* const s, size_t len)
{
    if (!str) {
        return str;
    }
    str->o.data = calloc_(len + 1, sizeof(*str->o.data));
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
        free_(s->o.data, s->o.capacity);
#ifndef NDEBUG
        s->o.data = NULL;
#endif
    }
    free_(s, sizeof(*s));
}

void cstr_set_allocator(struct cstr_alloc_t a)
{
    // TODO: provide fallback functions if only one of either calloc
    //       or realloc is provided.
    cstr_allocator_ = a;
    assert(cstr_allocator_.calloc  != NULL);
    assert(cstr_allocator_.realloc != NULL);
    assert(cstr_allocator_.free    != NULL);
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

int cstr_cmp(const cstr* s1, const cstr* s2)
{
    return strcmp(cstr_str(s1), cstr_str(s2));
}

int cstr_eq(const cstr* s1, const cstr* s2)
{
    return cstr_cmp(s1, s2) == 0;
}

int cstr_neq(const cstr* s1, const cstr* s2)
{
    return !cstr_eq(s1, s2);
}

int cstr_lt(const cstr* s1, const cstr* s2)
{
    return cstr_cmp(s1, s2) < 0;
}

int cstr_lte(const cstr* s1, const cstr* s2)
{
    return cstr_cmp(s1, s2) <= 0;
}

int cstr_gt(const cstr* s1, const cstr* s2)
{
    return cstr_cmp(s1, s2) > 0;
}

int cstr_gte(const cstr* s1, const cstr* s2)
{
    return cstr_cmp(s1, s2) >= 0;
}
