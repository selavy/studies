add_library(MyLib mylib.c)
add_executable(cgreen-test
	test_mylib.c
	all_tests.c
	)

target_link_libraries(cgreen-test
    MyLib
    ${CGREEN_LIBRARIES}
    )
if (${CGREEN_INCLUDE_DIRS})
    target_include_directories(cgreen-test
        ${CGREEN_INCLUDE_DIRS}
        )
endif()
