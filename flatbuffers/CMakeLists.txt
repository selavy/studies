add_executable(monsters monsters_generated.h main.cpp)
# TODO: get a FindFlatbuffers.cmake
target_link_libraries(monsters PUBLIC cxx_project_options PRIVATE flatbuffers)
