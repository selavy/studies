add_executable(test_integer
    intalu.h
    intalu.cpp
    intalu_test.cpp
    ${CATCH_MAIN}
    # "${PROJECT_SOURCE_DIR}/third_party/catch_main.cpp"
    )
set_target_properties(test_integer PROPERTIES CXX_STANDARD 17)
target_link_libraries(test_integer Catch)
# target_include_directories(test_integer PUBLIC "${PROJECT_SOURCE_DIR}/third_party/Catch2/single_include/")
# target_link_libraries(integer PUBLIC ${CGREEN_LIBRARIES})
# target_include_directories(integer PUBLIC ${CGREEN_INCLUDE_DIRS})
