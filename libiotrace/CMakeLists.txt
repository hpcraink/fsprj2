project(libiotrace)
cmake_minimum_required(VERSION 3.3.2)

enable_language(C)

# XXX For production version, these should be deleted (it's a user-option)
# set(CMAKE_BUILD_TYPE Release)
set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_C_STANDARD 99)

# XXX For production version, the default should be OFF
# And a CI/CD server should compile using cmake -DWITH_UNIT_TESTS
option (WITH_UNIT_TESTS "enable building unit tests" ON)
option (LOGGING "write log (can be changed during execution with functions from libiotrace.h; is only used if ENABLE_OUTPUT is set to LOGFILE or LOGFILE_AND_INFLUXDB)" ON)
option (SENDING "send log to influxdb (can be changed during execution with functions from libiotrace.h; is only used if ENABLE_OUTPUT is set to INFLUXDB or LOGFILE_AND_INFLUXDB)" ON)
option (STACKTRACE_PTR "write stacktrace pointer to log (can be changed during execution with functions from libiotrace.h)" OFF)
option (STACKTRACE_SYMBOL "write stacktrace symbols to log (can be changed during execution with functions from libiotrace.h)" OFF)
option (LOG_WRAPPER_TIME "log start and end time of function wrappers" ON)
option (WITH_STD_IO "log stdin, stdout and stderr IO" ON)
option (WITH_POSIX_IO "build wrappers for Posix-IO" ON)
option (WITH_POSIX_AIO "build wrappers for asynchronous Posix-IO" OFF)
option (WITH_MPI_IO "build wrappers for MPI-IO" OFF)
option (WITH_DL_IO "build wrappers for dlopen() and dlmopen()" OFF)
option (ALL_WRAPPERS_ACTIVE "Activate or deactivate all wrappers" ON)
option (REALTIME "Use the real system time (is affected by discontinuous jumps in the system time (e.g., if the system administrator manually changes the clock), and by the incremental adjustments performed by adjtime and NTP)" ON)
option (ENABLE_INPUT "enable input via REST-API" ON)

set(BUFFER_SIZE "1048576" CACHE STRING "buffer size per process")
set(MAX_FUNCTION_NAME "40" CACHE STRING "max length of function names")
set(MAX_ERROR_TEXT "1024" CACHE STRING "max length of error text for errno number")
set(MAX_MMSG_MESSAGES "20" CACHE STRING "max number of messages send/received over a socket via single function call")
set(MAX_MSG_FILE_DESCRIPTORS "200" CACHE STRING "max number of file descriptors send/received over a socket via single message")
set(MAX_EXEC_ARRAY_LENGTH "1000" CACHE STRING "max number of environment variables processed in wrapper for exec functions")
set(MAX_INFLUX_TOKEN "200" CACHE STRING "max length of influxdb token")
set(MAX_STACKTRACE_DEPTH "50" CACHE STRING "max number of function call entries in a single stack trace")
set(MAX_STACKTRACE_ENTRY_LENGTH "200" CACHE STRING "max length of single entry in stack trace")
set(MAX_MPI_FILE_HINTS "30" CACHE STRING "max number of MPI_Info file hints")
set(MAX_MPI_FILE_HINT_LENGTH "30" CACHE STRING "max length per MPI_Info file hint (key + value)")
set(MAX_MPI_IMESSAGES "1000" CACHE STRING "max number of immediate messages that could be processed in MPI_Waitall and MPI_Testall")
set(PORT_RANGE_MIN "50000" CACHE STRING "begin of port range to control live tracing")
set(PORT_RANGE_MAX "60000" CACHE STRING "end of port range to control live tracing")
set(SELECT_TIMEOUT_SECONDS "1" CACHE STRING "interval in which new threads are added to the communication_thread (reads responses from influxdb)")

set(STACKTRACE_DEPTH "0" CACHE STRING "stacktrace depth for logging (can be changed during execution with functions from libiotrace.h); value must be lower or equal than MAX_STACKTRACE_DEPTH")
set(STACKTRACE_DEPTH_VALUES "0;1;2;3;4;5;6;7;8;9;10;11;12;13;14;15;16;17;18;19;20;21;22;23;24;25;26;27;28;29;30;31;32")
set_property(CACHE STACKTRACE_DEPTH PROPERTY STRINGS ${STACKTRACE_DEPTH_VALUES})

set(ENABLE_OUTPUT "LOGFILE_AND_INFLUXDB" CACHE STRING "include functions to write to one or multiple targets")
set(ENABLE_OUTPUT_VALUES "LOGFILE_AND_INFLUXDB;LOGFILE;INFLUXDB")
set_property(CACHE ENABLE_OUTPUT PROPERTY STRINGS ${ENABLE_OUTPUT_VALUES})



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
    # rm -rf ext
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
