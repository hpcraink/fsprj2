project(libiotrace)
cmake_minimum_required(VERSION 3.4)

enable_language(C)

# XXX For production version, these should be deleted (it's a user-option)
# set(CMAKE_BUILD_TYPE Release)
set(CMAKE_BUILD_TYPE Debug)

# XXX For production version, the default should be OFF
# And a CI/CD server should compile using cmake -DWITH_UNIT_TESTS
option (WITH_UNIT_TESTS "enable building unit tests" ON) 

include(CheckIncludeFiles)
include(WriteCompilerDetectionHeader)
include(${libiotrace_SOURCE_DIR}/cmake/CheckCompilerAttributes.cmake)
CheckCompilerAttributes() 


include_directories(${libiotrace_BINARY_DIR}/include)

add_subdirectory(src)

if (WITH_UNIT_TESTS)
    # XXX Develop a better way to check for/include cunit
    # XXX list(APPEND CMAKE_PREFIX_PATH "${CUNIT_EXT_DIR}/")
    # find_package(CUnit REQUIRED)
    #
    # XXX Bail out by just adding the cunit git as submodule
    # git submodule add https://gitlab.com/cunity/cunit.git ext/cunit
    add_subdirectory(ext/cunit)
endif()
add_subdirectory(test)

# configure a header file to pass some of the CMake settings
# to the source code
configure_file (
  "${libiotrace_SOURCE_DIR}/include/libiotrace_config.h.in"
  "${libiotrace_BINARY_DIR}/include/libiotrace_config.h"
)

################################
install(FILES ${libiotrace_SOURCE_DIR}/include/libiotrace.h
              ${libiotrace_BINARY_DIR}/include/libiotrace_config.h
        DESTINATION include
        CONFIGURATIONS Release RelWithDebInfo Debug
)
