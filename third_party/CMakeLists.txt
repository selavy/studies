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
