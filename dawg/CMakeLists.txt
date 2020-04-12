add_executable(test_dawg
    test_dawg.cpp
    )
target_link_libraries(test_dawg
    PUBLIC
        # Catch
        # Alu
        cxx_project_options
        # project_warnings
)
