cmake_minimum_required(VERSION 3.0)

project(LibSuper
    VERSION 0.0.1
    LANGUAGES C
)

# add_library(c_project_options INTERFACE)
# target_compile_features(c_project_options INTERFACE c_std_99)

add_library(LibSuper
    src/super.c
    )
target_include_directories(LibSuper
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_compile_features(LibSuper INTERFACE c_std_99)
# target_link_libraries(LibSuper
#     PRIVATE
#         c_project_options
# )
