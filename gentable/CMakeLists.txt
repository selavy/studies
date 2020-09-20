add_executable(test_gentable test_gentable.cpp)
target_link_libraries(test_gentable PUBLIC Catch cxx_project_options fmt::fmt)


add_executable(test_simd simdtests.cpp)
target_link_libraries(test_simd PUBLIC cxx_project_options fmt::fmt)
target_compile_options(test_simd PUBLIC -march=corei7-avx -mavx -mtune=corei7-avx)
