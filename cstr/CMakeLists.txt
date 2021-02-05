add_library(CStr cstr.c)
target_link_libraries(CStr INTERFACE c_project_options)
target_include_directories(CStr PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(cstr_test cstr.test.cxx)
target_link_libraries(cstr_test PUBLIC Catch cxx_project_options CStr)
