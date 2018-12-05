project(libiotrace)
cmake_minimum_required(VERSION 3.4)

enable_language(C)

include(CheckIncludeFiles)
include(WriteCompilerDetectionHeader)

# set(CMAKE_BUILD_TYPE Release)
set(CMAKE_BUILD_TYPE Debug)
 
include_directories(${PROJECT_BINARY_DIR}/include)

add_subdirectory(src)
add_subdirectory(test)

# configure a header file to pass some of the CMake settings
# to the source code
configure_file (
  "${PROJECT_SOURCE_DIR}/include/libiotrace_config.h.in"
  "${PROJECT_BINARY_DIR}/include/libiotrace_config.h"
)

################################
install(FILES ${CMAKE_SOURCE_DIR}/include/libiotrace.h
              ${CMAKE_CURRENT_BINARY_DIR}/include/libiotrace.h
        DESTINATION include
        CONFIGURATIONS Release RelWithDebInfo Debug
)

