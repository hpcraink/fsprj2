cmake_minimum_required(VERSION 3.3.2)
project(libiotrace C CXX)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 17)

find_program(CMAKE_C_COMPILER NAMES $ENV{CC} gcc PATHS ENV PATH NO_DEFAULT_PATH)        # Find compiler based on env-var, and not soley based on `/usr/bin/cc`  (which causes issues on systems where the compiler is loaded via `module load <compiler-module>`)
find_program(CMAKE_CXX_COMPILER NAMES $ENV{CXX} g++ PATHS ENV PATH NO_DEFAULT_PATH)


# -----------------------------------------------------------------------------
# Compile options
# -----------------------------------------------------------------------------
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
    message(STATUS "No build type selected, defaulting to ${CMAKE_BUILD_TYPE}")
endif()

#include(CheckIPOSupported)                               # WARNING: !!  Requires changing `cmake_minimum_required` 2 newer version (e.g., 3.21)  !!
#check_ipo_supported(RESULT lto_supported)
#if (lto_supported)
#    #set_property(TARGET foo PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
#    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)         # Enable LTO by default (I.E., 4 all targets (SEE: https://stackoverflow.com/a/47370726))
#else()
#    message(WARNING "LTO isn't supported")
#endif()

add_compile_options(
    -Wall
    -Wextra
    -pedantic
    -pedantic-errors
    -Wformat=2
    -Wdouble-promotion
    -Wfloat-equal
    -Winit-self
    $<$<COMPILE_LANGUAGE:C>:-Wstrict-prototypes>
    -Wundef
    -Wunused-macros
    -Wswitch-enum
    -Wswitch-default
    -Werror
)
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")                 # Definitions which don't seem 2 work w/ `clang`
    add_compile_options(
#            -march=native -mtune=native                  # ALWAYS optimize 4 local machine  (since built binaries won't be distributed anyways)
            -Walloc-zero
            -Wformat-signedness
            -Wduplicated-cond
            -frecord-gcc-switches
    )
endif()

include_directories(${libiotrace_SOURCE_DIR}/include
                    ${libiotrace_BINARY_DIR}/include)


# -----------------------------------------------------------------------------
# Subdirs containing targets
# -----------------------------------------------------------------------------
add_subdirectory(src)
add_subdirectory(test)


# configure a header file to pass some of the CMake settings
# to the source code
configure_file(
  "${libiotrace_SOURCE_DIR}/include/libiotrace_config.h.in"
  "${libiotrace_BINARY_DIR}/include/libiotrace_config.h"
)



message(STATUS "")
message(STATUS "Build type            : ${CMAKE_BUILD_TYPE}")
message(STATUS "C Compiler exec       : ${CMAKE_C_COMPILER}")
message(STATUS "")
