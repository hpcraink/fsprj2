project(libiotrace)
cmake_minimum_required(VERSION 3.4)

enable_language(C)

# XXX For production version, these should be deleted (it's a user-option)
# set(CMAKE_BUILD_TYPE Release)
set(CMAKE_BUILD_TYPE Debug)

# XXX For production version, the default should be OFF
# And a CI/CD server should compile using cmake -DWITH_UNIT_TESTS
option (WITH_UNIT_TESTS "enable building unit tests" ON)
option (LOGGING "write log (can be changed during execution with functions from libiotrace.h)" ON)
option (LOG_WRAPPER_TIME "log start and end time of function wrappers" ON)
option (WITH_STD_IO "log stdin, stdout and stderr IO" ON)
option (WITH_POSIX_IO "build wrappers for Posix-IO" ON)
option (WITH_POSIX_AIO "build wrappers for asynchronous Posix-IO" OFF)
option (WITH_MPI_IO "build wrappers for MPI-IO" OFF)
option (WITH_DL_IO "build wrappers for dlopen() and dlmopen()" OFF)

set(BUFFER_SIZE "1048576" CACHE STRING "buffer size per process")

set(STACKTRACE_DEPTH "0" CACHE STRING "stacktrace depth for logging")
set(STACKTRACE_DEPTH_VALUES "0;1;2;3;4;5;6;7;8;9;10;11;12;13;14;15;16;17;18;19;20;21;22;23;24;25;26;27;28;29;30;31;32")
set_property(CACHE STACKTRACE_DEPTH PROPERTY STRINGS ${STACKTRACE_DEPTH_VALUES})

if(WITH_MPI_IO)
    find_package(MPI REQUIRED)
    if(MPI_C_FOUND)
        include_directories(${MPI_C_INCLUDE_DIRS})
    else()
        message(SEND_ERROR "Missing MPI-Package.")
    endif()
endif()

include(CheckIncludeFiles)
include(WriteCompilerDetectionHeader)
include(${libiotrace_SOURCE_DIR}/cmake/CheckCompilerAttributes.cmake)
CheckCompilerAttributes() 


# XXX ToDo: check for __uint128_t support (only on 64-bit architecture and some compiler-versions)


include_directories(${libiotrace_SOURCE_DIR}/include)
include_directories(${libiotrace_BINARY_DIR}/include)

add_subdirectory(src)

if (WITH_UNIT_TESTS)
    # XXX Develop a better way to check for/include cunit
    # XXX list(APPEND CMAKE_PREFIX_PATH "${CUNIT_EXT_DIR}/")
    # find_package(CUnit REQUIRED)
    #
    # XXX Bail out by just adding the cunit git as submodule
    # git rm --cached ext/cunit
    # rm -f ext
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
