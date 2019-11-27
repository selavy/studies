cmake_minimum_required(VERSION 3.0)

project(studies
    VERSION 0.0.1
    LANGUAGES C CXX
    )

# Test Infrastructure:
add_subdirectory(third_party)

option(USE_SYSTEM_CGREEN "Use system install version of CGreen" OFF)
message("-- Use system cgreen? ${USE_SYSTEM_CGREEN}")

if (USE_SYSTEM_CGREEN)
	# To use system version of CGreen:
	find_package(cgreen REQUIRED)
else ()
    add_subdirectory(third_party/cgreen)
    set(CGREEN_LIBRARIES    cgreen_shared)
    set(CGREEN_INCLUDE_DIRS )
    target_include_directories(${CGREEN_LIBRARIES}
        PUBLIC
            $<INSTALL_INTERFACE:include>
            $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/third_party/cgreen/include>
    )
endif (USE_SYSTEM_CGREEN)

# Studies:
add_subdirectory(alu)
add_subdirectory(cgreen-test)
