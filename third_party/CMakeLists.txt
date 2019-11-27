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
