add_executable(integer
    intalu.h
    intalu.c
    intalu_test.c
    )
target_link_libraries(integer PUBLIC ${CGREEN_LIBRARIES})
target_include_directories(integer PUBLIC ${CGREEN_INCLUDE_DIRS})
