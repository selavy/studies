cmake_minimum_required(VERSION 3.0)

project(studies
    VERSION 0.0.1
    LANGUAGES C CXX
    )

# find_package(cgreen REQUIRED)

add_library(Catch INTERFACE)
target_include_directories(Catch INTERFACE "${PROJECT_SOURCE_DIR}/third_party/Catch2/single_include/")
set(CATCH_MAIN "${PROJECT_SOURCE_DIR}/third_party/catch_main.cpp")

add_subdirectory(third_party)
add_subdirectory(integer)
