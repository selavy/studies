add_library(DArray darray.h darray.cpp)
target_link_libraries(DArray PUBLIC cxx_project_options)

add_executable(mkarrays mkarrays.cpp)
target_link_libraries(mkarrays
    PUBLIC
        cxx_project_options
        DArray
)

add_executable(test_arrays test_arrays.cpp)
target_link_libraries(test_arrays
    PUBLIC
        cxx_project_options
        Catch
        DArray
)
