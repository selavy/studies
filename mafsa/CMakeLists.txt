add_library(Arrays
    iconv.h
    tarray_util.h
    tarray_util.cpp

    darray.h
    darray.cpp
    darray2.h
    darray2.cpp
	darray3.h
	darray3.cpp

    tarraysep.h
    tarraysep.cpp
    tarraydelta.h
    tarraydelta.cpp
    tarray.h
    tarray.cpp

    mafsa.h
    mafsa.cpp

    darray_generated.h
    tarray_generated.h
    mafsa_generated.h
)
target_link_libraries(Arrays PUBLIC cxx_project_options ZLIB::ZLIB)

add_executable(mkarrays mkarrays.cpp)
target_link_libraries(mkarrays
    PUBLIC
        cxx_project_options
        Arrays
)

add_executable(test_arrays test_arrays.cpp)
target_link_libraries(test_arrays
    PUBLIC
        cxx_project_options
        Catch
        Arrays
)

add_executable(bench_arrays bench_data.h bench_arrays.cpp)
target_link_libraries(bench_arrays
    PUBLIC
        cxx_project_options
        Arrays
        Google::Benchmark
)

add_executable(fuzzish fuzzish.cpp)
target_link_libraries(fuzzish
    PUBLIC
        cxx_project_options
        Arrays
)
