add_executable(test_optional
    main.cxx
    )
target_link_libraries(test_optional
    PUBLIC
        Catch
        cxx_project_options
        # project_warnings
)
