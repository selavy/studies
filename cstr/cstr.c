#include "cstr.h"
#include <string.h>
#include <stdlib.h>
#include <assert.h>

//------------------------------------------------------------------------------
// cstr_alloc_t
//------------------------------------------------------------------------------
static void cstr_free_(void* p, size_t size)
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

//------------------------------------------------------------------------------
// cstrview
//------------------------------------------------------------------------------
cstrview cstrview_init(const char* s, size_t len)
{
    cstrview v;
    v.begin = s;
    v.end   = s + len;
    return v;
}

cstrview cstrview_fromrange(const char* begin, const char* end)
{
    cstrview v;
    v.begin = begin;
    v.end   = end;
    return v;
}

const char* cstrview_str(const cstrview v)
{
    return v.begin;
}

size_t cstrview_len(const cstrview v)
{
    return v.end - v.begin;
}

size_t cstrview_size(const cstrview v)
{
    return cstrview_len(v);
}

size_t cstrview_length(const cstrview v)
{
    return cstrview_len(v);
}

cstr cstrview_tostr(cstrview v)
{
    return cstr_make(cstrview_str(v), cstrview_len(v));
}

//------------------------------------------------------------------------------
// cstr
//------------------------------------------------------------------------------
cstr* cstr_new(const char* const s, const size_t len)
{
    cstr* str = calloc_(1, sizeof(*str));
    if (!str) {
        return NULL;
    }
    if (!cstr_init(str, s, len)) {
        free(str);
        return NULL;
    }
    return str;
}

cstr cstr_make(const char* s, size_t len)
{
    cstr str;
    cstr_init(&str, s, len);
    return str;
}

cstr* cstr_init(cstr* str, const char* const s, size_t len)
{
    if (!str) {
        return NULL;
    }
    str->o.data = calloc_(len + 1, sizeof(*str->o.data));
    if (!str->o.data) {
        return NULL;
    }
    memcpy(str->o.data, s, sizeof(*s) * len);
    str->o.data[len] = '\0';
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

cstrview cstr_view(const cstr* s)
{
    return cstrview_init(cstr_str(s), cstr_len(s));
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

cstr* cstr_copy(const cstr* s)
{
    return cstr_new(cstr_str(s), cstr_size(s));
}