add_executable(function_search
    function_search_bench.cxx
)
target_link_libraries(function_search
    PUBLIC
        cxx_project_options
        Google::Benchmark
)
