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
cstrview cstrview_make(const char* s)
{
    return cstrview_init(s, strlen(s));
}

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

const char* cstrview_data(cstrview v)
{
    return cstrview_str(v);
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

char* cstrview_to_cstring(cstrview v)
{
    size_t size = cstrview_len(v);
    char* s = calloc(size + 1, sizeof(char));
    if (!s) {
        return NULL;
    }
    memcpy(s, v.begin, size);
    s[size] = '\0';
    return s;
}

cstrview cstrview_take(cstrview v, size_t n)
{
    size_t size = cstrview_len(v);
    n = n < size ? n : size;
    v.end -= n;
    return v;
}

cstrview cstrview_drop(cstrview v, size_t n)
{
    size_t size = cstrview_len(v);
    n = n < size ? n : size;
    v.begin += n;
    return v;
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

cstr* cstr_new2(const char* const s)
{
    return cstr_new(s, strlen(s));
}

cstr cstr_make(const char* const s, size_t len)
{
    cstr str;
    cstr_init(&str, s, len);
    return str;
}

cstr cstr_make2(const char* const s)
{
    return cstr_make(s, strlen(s));
}

void cstr_destroy(cstr* s)
{
    if (!cstr_isinline_(s)) {
        free_(s->o.data, s->o.capacity + 1);
#ifndef NDEBUG
        s->o.data = NULL;
#endif
    }
}

cstr* cstr_init(cstr* str, const char* const s, size_t len)
{
    if (!str) {
        return NULL;
    }
    if (len <= CSTR_INLINE_SIZE) { // SSO
        // TODO: revisit if this is better. thought is that it is 3 x 8B writes
        //       but not sure what codegen I get
        memset(&str->data[0], '\0', sizeof(str->data));
        memcpy(&str->data[0], s, len);
    } else {                          // out of line:
        str->o.data = calloc_(len + 1, sizeof(*str->o.data));
        if (!str->o.data) {
            return NULL;
        }
        memcpy(str->o.data, s, sizeof(*s) * len);
        str->o.data[len] = '\0';
        str->o.capacity = len;
        str->data[CSTR_INLINE_SIZE] = 1;
    }
    str->size = len;
    return str;
}

void cstr_del(cstr* s)
{
    if (s) {
        cstr_destroy(s);
        free_(s, sizeof(*s));
    }
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

void cstr_reset_allocator_to_default_()
{
    cstr_allocator_ = malloc_allocator_;
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

char* cstr_data(cstr* s)
{
    return cstr_mstr(s);
}

cstrview cstr_view(const cstr* s)
{
    return cstrview_init(cstr_str(s), cstr_len(s));
}

int cstr_isinline_(const cstr* s)
{
    return s->data[CSTR_INLINE_SIZE] == '\0';
}

char* cstr_inline_mark_(cstr* s)
{
    return &s->data[CSTR_INLINE_SIZE];
}

size_t cstr_max_inline_size()
{
    return CSTR_INLINE_SIZE;
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

cstr* cstr_shink_to_fit(cstr* s)
{
    if (cstr_isinline_(s)) {
        return s;
    }

    const size_t size = cstr_size(s);
    if (size <= CSTR_INLINE_SIZE) {
        char*  str = s->o.data;
        size_t cap = s->o.capacity;
        memset(&s->data[0], '\0', CSTR_INLINE_SIZE + 1);
        memcpy(&s->data[0], str, size);
        free_(str, s->o.capacity);
        return s;
    }

    const size_t capacity = cstr_capacity(s);
    if (capacity > size) {
        char* p = realloc_(s->o.data, size + 1);
        if (p != NULL) {
            s->o.data = p;
            s->o.capacity = size;
        }
    }
    return s;
}

cstr* cstr_appendv(cstr* s, const cstrview v)
{
    // NOTE: sizes and capacities don't include the NULL terminator
    const size_t cursize  = cstr_len(s);
    const size_t addsize  = cstrview_len(v);
    const size_t newsize  = cursize + addsize;
    const size_t capacity = cstr_capacity(s);
    const int    isinline = cstr_isinline_(s);
    if (newsize <= capacity) {
        char* data = isinline ? s->data : s->o.data;
        memcpy(&data[cursize], cstrview_data(v), addsize);
        data[newsize] = '\0';
    } else {
        char* data;
        if (isinline) {
            data = calloc_(newsize + 1, sizeof(char));
            if (!data) {
                return NULL;
            }
            memcpy(&data[0], &s->data[0], cursize);
        } else {
            data = realloc_(s->o.data, (newsize + 1) * sizeof(char));
            if (!data) {
                return NULL;
            }
        }
        memcpy(&data[cursize], cstrview_data(v), addsize);
        data[newsize] = '\0';
        s->o.data     = data;
        s->o.capacity = newsize;
        *cstr_inline_mark_(s) = 1;
    }
    s->size = newsize;
    return s;
}

cstr* cstr_append(cstr* s, const cstr* s2)
{
    return cstr_appendv(s, cstr_view(s2));
}

cstr* cstr_prependv(cstr* s, cstrview v)
{
    size_t oldsize = cstr_len(s);
    size_t addsize = cstrview_len(v);
    size_t newsize = oldsize + addsize; // TODO: potential overflow

    if (newsize <= cstr_capacity(s)) {
        char* data = cstr_data(s);
        memmove(&data[addsize], &data[0], oldsize);
        memcpy(&data[0], cstrview_data(v), addsize);
        data[newsize] = '\0';
    } else {
        char* data;
        if (cstr_isinline_(s)) {
            data = calloc_(newsize + 1, sizeof(char));
            if (!data) {
                return NULL;
            }
            memcpy(&data[addsize], &s->data[0], oldsize);
        } else {
            data = realloc_(s->o.data, (newsize + 1) * sizeof(char));
            if (!data) {
                return NULL;
            }
            memmove(&data[addsize], &data[0], oldsize);
        }
        memcpy(&data[0], cstrview_data(v), addsize);
        data[newsize] = '\0';
        s->o.data = data;
        s->o.capacity = newsize;
        *cstr_inline_mark_(s) = 1;
    }
    s->size = newsize;
    return s;
}

cstr* cstr_prepend(cstr* s, const cstr* s2)
{
    return cstr_prependv(s, cstr_view(s2));
}

cstr* cstr_take(cstr* s, size_t n)
{
    const size_t oldsize = cstr_size(s);
    const size_t newsize = n < oldsize ? n : oldsize;
    char* data = cstr_data(s);
    data[newsize] = '\0';
    s->size = newsize;
    return s;
}

cstr* cstr_drop(cstr* s, size_t n)
{
    const size_t oldsize = cstr_size(s);
    n = n < oldsize ? n : oldsize;
    const size_t newsize = oldsize - n;
    char* data = cstr_data(s);
    memmove(&data[0], &data[n], newsize);
    data[newsize] = '\0';
    s->size = newsize;
    return s;
}
