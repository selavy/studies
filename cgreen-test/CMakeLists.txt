add_library(MyLib mylib.c)
add_executable(cgreen-test test_mylib.c)
target_link_libraries(cgreen-test MyLib CGreen)
