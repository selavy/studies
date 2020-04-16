add_library(Dawg dawg.cpp)
target_link_libraries(Dawg PUBLIC cxx_project_options PRIVATE project_warnings)

add_library(Mafsa mafsa.cpp)
target_link_libraries(Mafsa PUBLIC cxx_project_options PRIVATE project_warnings)

add_executable(test_dawg
    test_dawg.cpp
    )
target_link_libraries(test_dawg
    PUBLIC
        cxx_project_options
        Dawg
        Mafsa
        Catch
)

add_executable(makedawg
    makedawg.cpp
    )
target_link_libraries(makedawg
    PUBLIC
        cxx_project_options
        Dawg
        Mafsa
        project_warnings
)
