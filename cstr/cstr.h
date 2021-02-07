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
    void* (*calloc      )(size_t nmemb, size_t size);
    void* (*reallocarray)(void* p, size_t nmemb, size_t size);
    void  (*free        )(void* p, size_t size);
};

//------------------------------------------------------------------------------
// cstrview
//------------------------------------------------------------------------------

cstrview    cstrview_make(const char* s);
cstrview    cstrview_init(const char* s, size_t len);
cstrview    cstrview_fromrange(const char* begin, const char* end);
const char* cstrview_str(cstrview v);
const char* cstrview_data(cstrview v);
size_t      cstrview_len(cstrview v);
size_t      cstrview_size(cstrview v);
size_t      cstrview_length(cstrview v);
int         cstrview_empty(cstrview v);
cstr        cstrview_tostr(cstrview v);
char*       cstrview_to_cstring(cstrview v); // caller takes ownership

// Mutators
cstrview cstrview_take(cstrview v, size_t n);
cstrview cstrview_drop(cstrview v, size_t n);
// cstrview cstrview_substr(cstrview v, size_t pos, size_t len);
// cstrview cstrview_split(cstrview v, char c);

// Comparisons:
int cstrview_startswith(cstrview v, cstrview prefix);
int cstrview_endswith(cstrview v, cstrview postfix);
int cstrview_cmp(cstrview v1, cstrview v2);
int cstrview_eq(cstrview v1, cstrview v2);
int cstrview_neq(cstrview v1, cstrview v2);
int cstrview_gt(cstrview v1, cstrview v2);
int cstrview_lt(cstrview v1, cstrview v2);
int cstrview_gte(cstrview v1, cstrview v2);
int cstrview_lte(cstrview v1, cstrview v2);

//------------------------------------------------------------------------------
// cstr
//------------------------------------------------------------------------------

// Initialization / Destruction
cstr* cstr_new(const char* s, size_t len);
cstr* cstr_new2(const char* s);
void  cstr_del(cstr* s);

cstr  cstr_make(const char* s, size_t len);
cstr  cstr_make2(const char* s);
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
      char* cstr_data(cstr* s);

// Comparison Functions:
int cstr_cmp(const cstr* s1, const cstr* s2);
int cstr_eq(const cstr* s1, const cstr* s2);
int cstr_neq(const cstr* s1, const cstr* s2);
int cstr_lt(const cstr* s1, const cstr* s2);
int cstr_lte(const cstr* s1, const cstr* s2);
int cstr_gt(const cstr* s1, const cstr* s2);
int cstr_gte(const cstr* s1, const cstr* s2);
int cstr_startswithv(const cstr* s1, cstrview v);
int cstr_startswith(const cstr* s1, const cstr* s2);
int cstr_endswithv(const cstr* s1, cstrview v);
int cstr_endswith(const cstr* s1, const cstr* s2);

// Mutators:
cstr* cstr_copy(const cstr* s);
cstr* cstr_shrink_to_fit(cstr* s);
cstr* cstr_appendv(cstr* s, cstrview v);
cstr* cstr_append(cstr* s, const cstr* s2);
cstr* cstr_prependv(cstr* s, cstrview v);
cstr* cstr_prepend(cstr* s, const cstr* s2);
cstr* cstr_insertv(cstr* s, size_t pos, cstrview v); // insert `v` at position `pos`
cstr* cstr_insert(cstr* s, size_t pos, const cstr* s2);
cstr* cstr_take(cstr* s, size_t n); // take up to n from front
cstr* cstr_drop(cstr* s, size_t n); // drop up to n from front

// Views:
cstrview cstr_view(const cstr* s);
cstrview cstr_substr(const cstr* s, size_t pos, size_t len);

// Private Functions:
char*  cstr_inline_mark_(cstr* s);
int    cstr_isinline_(const cstr* s);
size_t cstr_max_inline_size();
void   cstr_reset_allocator_to_default_();

#ifdef __cplusplus
}  // extern "C"
#endif

#endif // CSTR__H_
