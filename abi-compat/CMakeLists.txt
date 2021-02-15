add_subdirectory(libsuper)

add_executable(test_super test_super.cxx)
target_link_libraries(test_super
    PUBLIC
        LibSuper
        cxx_project_options
        project_warnings
)
