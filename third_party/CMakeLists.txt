# TODO: PR to cgreen to allow using it as a submodule
# add_subdirectory(cgreen)

#
# Catch2
#
add_subdirectory(Catch2)
add_library(Catch catch_main.cpp)
target_include_directories(Catch PUBLIC "Catch2/single_include/")

#
# CGreen
#
# # find_package(cgreen REQUIRED)
# add_subdirectory(cgreen)
# add_library(CGreen cgreen_main.c)
# target_link_libraries(CGreen PUBLIC cgreen_shared)
# target_include_directories(CGreen
#     PUBLIC
#         $<INSTALL_INTERFACE:include>
#         $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/cgreen/include>
# )

option(BENCHMARK_ENABLE_TESTING OFF)
option(BENCHMARK_ENABLE_EXCEPTIONS ON)
option(BENCHMARK_ENABLE_LTO ON)
option(BENCHMARK_USE_LIBCXX OFF)
option(BENCHMARK_BUILD_32_BITS OFF)
option(BENCHMARK_ENABLE_INSTALL OFF)
option(BENCHMARK_DOWNLOAD_DEPENDENCIES OFF)
option(BENCHMARK_ENABLE_GTEST_TESTS OFF)
add_subdirectory(benchmark)
add_library(Google::Benchmark ALIAS benchmark)

#
# LibFMT
#
add_subdirectory(fmt)

#
# FlatBuffers
#
add_subdirectory(flatbuffers)
