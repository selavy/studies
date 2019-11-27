add_executable(wc-serial wc_serial.c)
add_executable(wc-simd   wc_simd.c)
target_compile_options(wc-simd PUBLIC -march=native)
