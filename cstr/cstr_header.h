#pragma once

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <malloc.h>

#define CSTR_FORCE_INLINE [[gnu::always_inline]] inline

#define CSTR_INLINE_SIZE 19
struct cstr_t
{
    uint32_t size = 0;
    union {
        struct {
            char*  data;
            size_t capacity;
        } o;
        char data[CSTR_INLINE_SIZE + 1] = { 0 };
    } __attribute__((packed));
};
typedef struct cstr_t cstr;
static_assert(sizeof(cstr) == (4 + CSTR_INLINE_SIZE + 1));

struct cstrview_t
{
    const char* begin;
    const char* end;
};
typedef struct cstrview_t cstrview;
static_assert(sizeof(cstrview) == 2*sizeof(void*));

// put this back in later:
#if 0
struct cstr_alloc_t
{
    void* (*calloc      )(size_t nmemb, size_t size);
    void* (*reallocarray)(void* p, size_t nmemb, size_t size);
    void  (*free        )(void* p, size_t size);
};
#endif

CSTR_FORCE_INLINE cstr     cstr_make(const char* s, size_t len) noexcept;
CSTR_FORCE_INLINE cstr*    cstr_init(cstr* str, const char* s, size_t len) noexcept;
CSTR_FORCE_INLINE size_t   cstr_len(const cstr* s) noexcept;
CSTR_FORCE_INLINE cstr*    cstr_appendv(cstr* s, cstrview v) noexcept;
CSTR_FORCE_INLINE cstr*    cstr_append(cstr* s, const cstr* s2) noexcept;
CSTR_FORCE_INLINE cstrview cstr_view(const cstr* s) noexcept;
// TODO: mark this constexpr
CSTR_FORCE_INLINE cstrview cstrview_init(const char* s, size_t len) noexcept;
// TODO: mark this constexpr
CSTR_FORCE_INLINE size_t   cstrview_len(cstrview v) noexcept;
CSTR_FORCE_INLINE size_t   cstr_capacity(const cstr* s) noexcept;
CSTR_FORCE_INLINE char*    cstr_inline_mark_(cstr* s) noexcept;
// TODO: mark constexpr
CSTR_FORCE_INLINE int      cstr_isinline_(const cstr* s) noexcept;
// TODO: mark contexpr
CSTR_FORCE_INLINE const char* cstrview_data(cstrview v) noexcept;
// TODO: mark contexpr
CSTR_FORCE_INLINE const char* cstr_str(const cstr* s) noexcept;
CSTR_FORCE_INLINE cstr* cstr_prependv(cstr* s, cstrview v) noexcept;
CSTR_FORCE_INLINE cstr* cstr_prepend(cstr* s, const cstr* s2) noexcept;
CSTR_FORCE_INLINE char* cstr_data(cstr* s) noexcept;

//------------------------------------------------------------------------------
// Implementation
//------------------------------------------------------------------------------

CSTR_FORCE_INLINE void* calloc_(size_t nmemb, size_t size) noexcept
{
    printf("WTF WHY ARE WE ALLOCATING!!!\n");
    return NULL;
    // return calloc(nmemb, size);
}

CSTR_FORCE_INLINE void* reallocarray_(void* p, size_t nmemb, size_t size) noexcept
{
    return NULL;
    // return reallocarray(p, nmemb, size);
}

CSTR_FORCE_INLINE void free_(void* p, size_t size) noexcept
{
    free(p/*, size*/);
}

cstrview cstr_view(const cstr* s) noexcept
{
    return cstrview_init(cstr_str(s), cstr_len(s));
}

cstr cstr_make(const char* const s, size_t len) noexcept
{
    cstr str;
    cstr_init(&str, s, len);
    return str;
}

cstr* cstr_init(cstr* str, const char* const s, size_t len) noexcept
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
        str->o.data = (char*)calloc_(len + 1, sizeof(*str->o.data));
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

size_t cstr_len(const cstr* s) noexcept
{
    return s->size;
}

cstr* cstr_append(cstr* s, const cstr* s2) noexcept
{
    return cstr_appendv(s, cstr_view(s2));
}

cstrview cstrview_init(const char* s, size_t len) noexcept
{
    cstrview v;
    v.begin = s;
    v.end   = s + len;
    return v;
}

cstr* cstr_appendv(cstr* s, const cstrview v) noexcept
{
    // NOTE: sizes and capacities don't include the NULL terminator
    const size_t cursize  = cstr_len(s);
    const size_t addsize  = cstrview_len(v);
    const size_t newsize  = cursize + addsize; // TODO: potential overflow
    const size_t capacity = cstr_capacity(s);
    const int    isinline = cstr_isinline_(s);
    if (newsize <= capacity) {
        char* data = isinline ? s->data : s->o.data;
        memcpy(&data[cursize], cstrview_data(v), addsize);
        data[newsize] = '\0';
    } else {
        char* data;
        if (isinline) {
            data = (char*)calloc_(newsize + 1, sizeof(char));
            if (!data) {
                return NULL;
            }
            memcpy(&data[0], &s->data[0], cursize);
        } else {
            data = (char*)reallocarray_(s->o.data, newsize + 1, sizeof(char));
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

size_t cstrview_len(const cstrview v) noexcept
{
    return v.end - v.begin;
}

size_t cstr_capacity(const cstr* s) noexcept
{
    return cstr_isinline_(s) != 0 ?
        CSTR_INLINE_SIZE : s->o.capacity;
}

int cstr_isinline_(const cstr* s) noexcept
{
    return s->data[CSTR_INLINE_SIZE] == '\0';
}

char* cstr_inline_mark_(cstr* s) noexcept
{
    return &s->data[CSTR_INLINE_SIZE];
}

const char* cstrview_data(cstrview v) noexcept
{
    return v.begin;
}

const char* cstr_str(const cstr* s) noexcept
{
    return cstr_isinline_(s) != 0 ?
        s->data : s->o.data;
}

cstr* cstr_prepend(cstr* s, const cstr* s2) noexcept
{
    return cstr_prependv(s, cstr_view(s2));
}

cstr* cstr_prependv(cstr* s, cstrview v) noexcept
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
            data = (char*)calloc_(newsize + 1, sizeof(char));
            if (!data) {
                return NULL;
            }
            memcpy(&data[addsize], &s->data[0], oldsize);
        } else {
            data = (char*)reallocarray_(s->o.data, newsize + 1, sizeof(char));
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

char* cstr_data(cstr* s) noexcept
{
    return cstr_isinline_(s) != 0 ?
        s->data : s->o.data;
}
