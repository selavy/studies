add_subdirectory(super)

add_executable(test_super test_super.cxx)
target_link_libraries(test_super
    PUBLIC
        Super::Super
        cxx_project_options
        project_warnings
)
