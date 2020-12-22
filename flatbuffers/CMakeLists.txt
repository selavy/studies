add_executable(monsters monsters_generated.h main.cpp)

# add_subdirectory(flatbuffers)

# TODO: get a FindFlatbuffers.cmake
target_link_libraries(monsters PUBLIC cxx_project_options PRIVATE flatbuffers)
