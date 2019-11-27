cmake_minimum_required(VERSION 3.0)

project(studies
    VERSION 0.0.1
    LANGUAGES C CXX
    )

# Test Infrastructure:
add_subdirectory(third_party)

# Studies:
add_subdirectory(alu)
add_subdirectory(cgreen-test)
