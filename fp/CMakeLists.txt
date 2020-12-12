add_executable(fp_test fp.cxx fp.test.cxx)
target_link_libraries(fp_test PUBLIC Catch cxx_project_options)
