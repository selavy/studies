#include "cstr.h"
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <malloc.h>

// TODO: figure out how to use the feature test macros to see if this function
//       is available
#if 0
#ifndef _GNU_SOURCE
#ifndef reallocarray

#define MUL_NO_OVERFLOW (1UL << (sizeof(size_t) * 4))

static void* reallocarray(void *optr, size_t nmemb, size_t size)
{
    if ((nmemb >= MUL_NO_OVERFLOW || size >= MUL_NO_OVERFLOW) &&
        nmemb > 0 && SIZE_MAX / nmemb < size) {
            errno = ENOMEM;
            return NULL;
    }
    return realloc(optr, size * nmemb);
}

#undef MUL_NO_OVERFLOW

#endif
#endif
#endif

//------------------------------------------------------------------------------
// cstr_alloc_t
//------------------------------------------------------------------------------
static void cstr_free_(void* p, size_t size)
{
    free(p);
}

static const struct cstr_alloc_t malloc_allocator_ =
{
    .calloc       = &calloc,
    .reallocarray = &reallocarray,
    .free         = &cstr_free_,
};

static struct cstr_alloc_t cstr_allocator_ = malloc_allocator_;

static void* calloc_(size_t nmemb, size_t size)
{
    return cstr_allocator_.calloc(nmemb, size);
}

static void* reallocarray_(void* p, size_t nmemb, size_t size)
{
    return cstr_allocator_.reallocarray(p, nmemb, size);
}

static void free_(void* p, size_t size)
{
    cstr_allocator_.free(p, size);
}

static void* calloc_using_reallocarray_(size_t nmemb, size_t size)
{
    void* p = cstr_allocator_.reallocarray(NULL, nmemb, size);
    if (p == NULL) {
        return p;
    }
    // NOTE: because reallocarray succeeded (which does the overflow check),
    //       we know this multiply doesn't overflow
    memset(p, 0, nmemb * size);
    return p;
}

// I don't think I should offer this interface because I need functionality
// like malloc_usable_size() for the user's calloc()
#if 0
static void* reallocarray_using_calloc_(void* p, size_t nmemb, size_t size)
{
#define MUL_NO_OVERFLOW (1UL << (sizeof(size_t) * 4))
    // check for overflow:
    if ((nmemb >= MUL_NO_OVERFLOW || size >= MUL_NO_OVERFLOW) &&
        nmemb > 0 && SIZE_MAX / nmemb < size) {
            errno = ENOMEM;
            return NULL;
    }

    // check if can elide allocation because already have space:
    size_t avail = malloc_usable_size(p);
    size_t need  = nmemb * size;
    if (need < avail) {
        return p;
    }

    // allocate and copy over data:
    void* pnew = calloc_(nmemb, size);
    if (!pnew) {
        return NULL;
    }
    // have to copy entire malloc'd block because don't know how much data
    // was actually there before:
    memcpy(pnew, p, avail);
    free_(p, avail);
    return pnew;
#undef MUL_NO_OVERFLOW
}
#endif

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

int cstrview_empty(const cstrview v)
{
    return v.begin == v.end;
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

int cstrview_startswith(cstrview v, cstrview prefix)
{
    size_t len1 = cstrview_len(v);
    size_t len2 = cstrview_len(prefix);
    return len2 <= len1
        && memcmp(cstrview_data(v), cstrview_data(prefix), len2) == 0;
}

int cstrview_endswith(cstrview v, cstrview postfix)
{
    size_t len1 = cstrview_len(v);
    size_t len2 = cstrview_len(postfix);
    size_t pos = len1 - len2;
    return len2 <= len1
        && memcmp(cstrview_data(v) + pos, cstrview_data(postfix), len2) == 0;
}

int cstrview_cmp(cstrview v1, cstrview v2)
{
    size_t len1 = cstrview_len(v1);
    size_t len2 = cstrview_len(v2);
    size_t min_ = len1 < len2 ? len1 : len2;
    int r = memcmp(cstrview_data(v1), cstrview_data(v2), min_);
    // TODO: need to protect against len1 or len2 being longer than int
    return r != 0 ? r : (int)len1 - (int)len2;
}

int cstrview_eq(const cstrview v1, const cstrview v2)
{
    return cstrview_cmp(v1, v2) == 0;
}

int cstrview_neq(const cstrview v1, const cstrview v2)
{
    return cstrview_cmp(v1, v2) != 0;
}

int cstrview_gt(const cstrview v1, const cstrview v2)
{
    return cstrview_cmp(v1, v2) > 0;
}

int cstrview_lt(const cstrview v1, const cstrview v2)
{
    return cstrview_cmp(v1, v2) < 0;
}

int cstrview_gte(const cstrview v1, const cstrview v2)
{
    return cstrview_cmp(v1, v2) >= 0;
}

int cstrview_lte(const cstrview v1, const cstrview v2)
{
    return cstrview_cmp(v1, v2) <= 0;
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
    cstr_allocator_ = a;

    if (a.calloc == NULL && a.reallocarray != NULL) {
        cstr_allocator_.calloc = &calloc_using_reallocarray_;
    }

#if 0
    if (a.reallocarray == NULL && a.calloc != NULL) {
        cstr_allocator_.reallocarray = &reallocarray_using_calloc_;
    }
#endif

    assert(cstr_allocator_.calloc       != NULL);
    assert(cstr_allocator_.reallocarray != NULL);
    assert(cstr_allocator_.free         != NULL);
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

int cstr_startswithv(const cstr* s1, const cstrview v)
{
    return cstrview_startswith(cstr_view(s1), v);
}

int cstr_startswith(const cstr* s1, const cstr* s2)
{
    return cstr_startswithv(s1, cstr_view(s2));
}

int cstr_endswithv(const cstr* s1, const cstrview v)
{
    return cstrview_endswith(cstr_view(s1), v);
}

int cstr_endswith(const cstr* s1, const cstr* s2)
{
    return cstr_endswithv(s1, cstr_view(s2));
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
        free_(str, s->o.capacity + 1);
        return s;
    }

    const size_t capacity = cstr_capacity(s);
    if (capacity > size) {
        char* p = reallocarray_(s->o.data, size + 1, sizeof(char));
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
            data = calloc_(newsize + 1, sizeof(char));
            if (!data) {
                return NULL;
            }
            memcpy(&data[0], &s->data[0], cursize);
        } else {
            data = reallocarray_(s->o.data, newsize + 1, sizeof(char));
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
            data = reallocarray_(s->o.data, newsize + 1, sizeof(char));
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

cstr* cstr_insertv(cstr* s, const size_t pos, cstrview v)
{
    size_t oldsize = cstr_len(s);
    size_t addsize = cstrview_len(v);
    size_t newsize = oldsize + addsize; // TODO: potential overflow
    if (pos > oldsize) {
        // TODO: need a better way to communicate this failure
        errno = ERANGE;
        return NULL;
    }
    size_t rest = oldsize - pos;
    if (newsize <= cstr_capacity(s)) {
        char* data = cstr_data(s);
        memmove(&data[pos+addsize], &data[pos], rest);
        memcpy(&data[pos], cstrview_data(v), addsize);
        data[newsize] = '\0';
    } else {
        char* data;
        if (cstr_isinline_(s)) {
            data = calloc_(newsize + 1, sizeof(char));
            if (!data) {
                return NULL;
            }
            memcpy(&data[0], &s->data[0], pos);
            memcpy(&data[pos+addsize], &s->data[pos], rest);
        } else {
            data = reallocarray_(s->o.data, newsize + 1, sizeof(char));
            if (!data) {
                return NULL;
            }
            memmove(&data[pos+addsize], &data[pos], rest);
        }
        memcpy(&data[pos], cstrview_data(v), addsize);
        data[newsize] = '\0';
        s->o.data = data;
        s->o.capacity = newsize + 1;
        *cstr_inline_mark_(s) = 1;
    }
    s->size = newsize;
    return s;
}

cstr* cstr_insert(cstr* s, const size_t pos, const cstr* s2)
{
    return cstr_insertv(s, pos, cstr_view(s2));
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

cstrview cstr_view(const cstr* s)
{
    return cstrview_init(cstr_str(s), cstr_len(s));
}

cstrview cstr_substr(const cstr* s, const size_t pos, const size_t len)
{
    size_t size = cstr_len(s);
    size_t i = pos < size ? pos : size;
    size_t j = i + len < size ? i + len : size;
    const char* data = cstr_str(s);
    assert(0 <= i && i <= size);
    assert(0 <= j && j <= size);
    assert(i <= j);
    return cstrview_fromrange(data + i, data + j);
}
