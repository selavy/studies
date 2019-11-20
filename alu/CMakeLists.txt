add_library(Alu
    alu.h
    alu.c
    )

add_executable(test_alu
    alu_test.cpp
    ${CATCH_MAIN}
    )
set_target_properties(test_alu PROPERTIES CXX_STANDARD 17)
target_link_libraries(test_alu Catch)
target_link_libraries(test_alu Alu)
