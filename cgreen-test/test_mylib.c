#include <cgreen/cgreen.h>
#include "mylib.h"

Describe(MyLib);
BeforeEach(MyLib) {}
AfterEach(MyLib) {}

Ensure(MyLib, foo_adds_one) {
	assert_that(mylib_foo(1), is_equal_to(2));
	assert_that(mylib_foo(2), is_equal_to(3));
	assert_that(mylib_foo(5), is_equal_to(6));
}

TestSuite *words_tests() {
    TestSuite *suite = create_test_suite();
    add_test_with_context(suite, MyLib,	foo_adds_one);
    return suite;
}
