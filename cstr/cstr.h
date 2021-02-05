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

cstr*  cstr_new(const char* s, size_t len);
void   cstr_del(cstr* s);
size_t cstr_size(const cstr* s);
size_t cstr_len(const cstr* s);
size_t cstr_length(const cstr* s);
// NOTE: does not include NULL terminator. i.e. capacity available for chars
size_t cstr_capacity(const cstr* s);
const char* cstr_str(const cstr* s);
char*  cstr_mstr(cstr* s);

int cstr_isinline_(const cstr* s);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif // CSTR__H_
