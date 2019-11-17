#include <cgreen/cgreen.h>
#include "intalu.h"

Ensure(integer_1_plus_2_equals_3)
{
    assert_that(intalu_adds8(1, 2), is_equal_to(3));
}

int main(int argc, char **argv) {
    TestSuite *suite = create_test_suite();
    add_test(suite, integer_1_plus_2_equals_3);
    return run_test_suite(suite, create_text_reporter());
}
