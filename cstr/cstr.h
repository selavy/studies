#ifndef CSTR__H_
#define CSTR__H_

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CSTR_INLINE_SIZE 20

struct cstr_t
{
    uint32_t size; // 4
    union {
        struct {
            uint32_t capacity;
            char*    data;
        } o;

        char     data[CSTR_INLINE_SIZE];
    };
};
typedef struct cstr_t cstr;

struct cstr_alloc_t
{
    void* (*calloc )(size_t nmemb, size_t size);
    void* (*realloc)(void* p, size_t size);
    void  (*free   )(void* p, size_t size);
};

// Initialization / Destruction
cstr* cstr_new(const char* s, size_t len);
cstr* cstr_init(cstr* str, const char* s, size_t len);
void  cstr_del(cstr* s);
void  cstr_set_allocator(struct cstr_alloc_t a);

// Length / Capacity Accessors:
size_t cstr_size(const cstr* s);
size_t cstr_len(const cstr* s);
size_t cstr_length(const cstr* s);
// NOTE: does not include NULL terminator. i.e. capacity available for chars
size_t cstr_capacity(const cstr* s);

// String Accessors:
const char* cstr_str(const cstr* s);
char*  cstr_mstr(cstr* s);

// Comparison Functions:
int cstr_cmp(const cstr* s1, const cstr* s2);
int cstr_eq(const cstr* s1, const cstr* s2);
int cstr_neq(const cstr* s1, const cstr* s2);
int cstr_lt(const cstr* s1, const cstr* s2);
int cstr_lte(const cstr* s1, const cstr* s2);
int cstr_gt(const cstr* s1, const cstr* s2);
int cstr_gte(const cstr* s1, const cstr* s2);

int cstr_isinline_(const cstr* s);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif // CSTR__H_
