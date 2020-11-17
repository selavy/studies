add_library(unrandom SHARED unrandom.c)

add_executable(ld_preload main.cxx)
target_link_libraries(ld_preload cxx_project_options)

# To run:
# $ export LD_PRELOAD=./ld_preload/libunrandom.so
# $ ./ld_preload/ld_preload
