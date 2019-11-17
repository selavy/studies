#include <cgreen/cgreen.h>
#include "intalu.h"

#define ASIZE(x) (sizeof(x) / sizeof(x[0]))

Ensure(integer_8bit_plus_one)
{
    int8_t vs[] = { 1, 2, 3, 4, 5, 6, 100, 125 };
    for (size_t i = 0; i < ASIZE(vs); ++i) {
        int8_t a = vs[i];
        int8_t b = 1;
        int8_t e = a + b;
        assert_that(intalu_adds8(a, b), is_equal_to(e));
    }
}

int main(int argc, char **argv) {
    TestSuite *suite = create_test_suite();
    add_test(suite, integer_8bit_plus_one);
    return run_test_suite(suite, create_text_reporter());
}
