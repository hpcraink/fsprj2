cmake_minimum_required(VERSION 3.27)
project(performance_code C)

set(CMAKE_C_STANDARD 11)

add_executable(performance_code main.c)
add_executable(performance_code2 main2.c)

target_compile_options(performance_code PRIVATE -O0)