add_library(Dawg dawg.cpp)
target_link_libraries(Dawg PUBLIC cxx_project_options project_warnings)

add_library(Mafsa mafsa.cpp)
target_link_libraries(Mafsa PUBLIC cxx_project_options project_warnings)

add_executable(test_dawg
    test_dawg.cpp
    )
target_link_libraries(test_dawg
    PUBLIC
        cxx_project_options
        Dawg
        Catch
        # project_warnings
)

add_executable(makedawg
    makedawg.cpp
    )
target_link_libraries(makedawg
    PUBLIC
        cxx_project_options
        Dawg
        project_warnings
)
