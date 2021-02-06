#ifndef CSTR__H_
#define CSTR__H_

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CSTR_INLINE_SIZE 19

#define CSTR_STATIC_ASSERT_(COND, MSG) \
    typedef char static_assertion_##MSG[(!!(COND)) * 2 - 1]
#define CSTR_COMPILE_TIME_ASSERT3_(X, L) CSTR_STATIC_ASSERT_(X, at_line_##L)
#define CSTR_COMPILE_TIME_ASSERT2_(X, L) CSTR_COMPILE_TIME_ASSERT3_(X, L)
#define CSTR_STATIC_ASSERT(X) CSTR_COMPILE_TIME_ASSERT2_(X, __LINE__)

struct cstr_t
{
    uint32_t size;
    union {
        struct {
            char*  data;
            size_t capacity;
        } o;
        char data[CSTR_INLINE_SIZE + 1];
    } __attribute__((packed));
};
typedef struct cstr_t cstr;
CSTR_STATIC_ASSERT(sizeof(cstr) == (4 + CSTR_INLINE_SIZE + 1));

struct cstrview_t
{
    const char* begin;
    const char* end;
};
typedef struct cstrview_t cstrview;
CSTR_STATIC_ASSERT(sizeof(cstrview) == 2*sizeof(void*));

struct cstr_alloc_t
{
    void* (*calloc )(size_t nmemb, size_t size);
    void* (*realloc)(void* p, size_t size);
    void  (*free   )(void* p, size_t size);
};

//------------------------------------------------------------------------------
// cstrview
//------------------------------------------------------------------------------
cstrview    cstrview_init(const char* s, size_t len);
cstrview    cstrview_fromrange(const char* begin, const char* end);
const char* cstrview_str(cstrview v);
size_t      cstrview_len(cstrview v);
size_t      cstrview_size(cstrview v);
size_t      cstrview_length(cstrview v);
cstr        cstrview_tostr(cstrview v);

//------------------------------------------------------------------------------
// cstr
//------------------------------------------------------------------------------

// Initialization / Destruction
cstr* cstr_new(const char* s, size_t len);
void  cstr_del(cstr* s);

cstr  cstr_make(const char* s, size_t len);
void  cstr_destroy(cstr* s);

cstr* cstr_init(cstr* str, const char* s, size_t len);
void  cstr_set_allocator(struct cstr_alloc_t a);

// Length / Capacity Accessors:
size_t cstr_size(const cstr* s);
size_t cstr_len(const cstr* s);
size_t cstr_length(const cstr* s);
// NOTE: does not include NULL terminator. i.e. capacity available for chars
size_t cstr_capacity(const cstr* s);

// String Accessors:
const char* cstr_str(const cstr* s);
      char* cstr_mstr(cstr* s);
cstrview    cstr_view(const cstr* s);

// Comparison Functions:
int cstr_cmp(const cstr* s1, const cstr* s2);
int cstr_eq(const cstr* s1, const cstr* s2);
int cstr_neq(const cstr* s1, const cstr* s2);
int cstr_lt(const cstr* s1, const cstr* s2);
int cstr_lte(const cstr* s1, const cstr* s2);
int cstr_gt(const cstr* s1, const cstr* s2);
int cstr_gte(const cstr* s1, const cstr* s2);

// Mutators:
cstr* cstr_copy(const cstr* s);

// Private Functions:
int    cstr_isinline_(const cstr* s);
size_t cstr_max_inline_size();
void   cstr_reset_allocator_to_default_();

#ifdef __cplusplus
}  // extern "C"
#endif

#endif // CSTR__H_
