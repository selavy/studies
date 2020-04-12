add_library(Alu
    alu.h
    alu.c
    )

add_executable(test_alu
    alu_test.cpp
    )
target_link_libraries(test_alu
    PUBLIC
        Catch
        Alu
        cxx_project_options
        # project_warnings
)
