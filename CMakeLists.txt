cmake_minimum_required(VERSION 3.0)

project(studies
    VERSION 0.0.1
    LANGUAGES C CXX
    )

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/")

add_library(project_warnings INTERFACE)
include(cmake/CompilerWarnings.cmake)
set_project_warnings(project_warnings)

add_library(cxx_project_options INTERFACE)
target_compile_features(cxx_project_options INTERFACE cxx_std_17)
# include(cmake/Sanitizers.cmake)
# if (CMAKE_BUILD_TYPE STREQUAL "Debug")
#     message("-- enabling sanitizers")
#     enable_sanitizers(cxx_project_options)
# endif (CMAKE_BUILD_TYPE STREQUAL "Debug")

find_package(ZLIB REQUIRED)

# Test Infrastructure:
add_subdirectory(third_party)

# Studies:
add_subdirectory(alu)
# add_subdirectory(cgreen-test)
add_subdirectory(wc)
add_subdirectory(dawg)
add_subdirectory(flatbuffers)
add_subdirectory(mafsa)
add_subdirectory(diffs)
add_subdirectory(ld_preload)
# add_subdirectory(gentable)

# Introduction To Algorithms - Cormen, Leiserson, Rivest, Stein
add_subdirectory(ch32_string_matching)
