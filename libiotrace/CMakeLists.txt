project(libiotrace)
cmake_minimum_required(VERSION 3.3.2)

enable_language(C)

# XXX For production version, these should be deleted (it's a user-option)
# set(CMAKE_BUILD_TYPE Release)
# set(CMAKE_BUILD_TYPE Debug)

add_compile_options(
    -Wall
    -Wextra
    -pedantic
    -pedantic-errors
    -Wformat=2
    -Wformat-signedness
    -Walloc-zero
    -Wdouble-promotion
    -Wduplicated-cond
    -Wfloat-equal
    -Winit-self
    -Wstrict-prototypes
    -Wundef
    -Wunused-macros
    -Wswitch-enum
    -Wswitch-default
    -Werror
    -frecord-gcc-switches
)

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
option (STRACING_ENABLED "trace syscalls" OFF)     # Install required libs: `sudo apt install -y libunwind-dev libdw-dev`
option (WITH_POSIX_IO "build wrappers for Posix-IO" ON)
option (WITH_POSIX_AIO "build wrappers for asynchronous Posix-IO" OFF)
option (WITH_MPI_IO "build wrappers for MPI-IO" OFF)
option (WITH_DL_IO "build wrappers for dlopen() and dlmopen()" OFF)
option (WITH_ALLOC "build wrappers for memory allocations" OFF)
option (FILENAME_RESOLUTION_ENABLED "create mapping b/w file handles and filenames during runtime" OFF)
option (ALL_WRAPPERS_ACTIVE "Activate or deactivate all wrappers" ON)
option (REALTIME "Use the real system time (is affected by discontinuous jumps in the system time (e.g., if the system administrator manually changes the clock), and by the incremental adjustments performed by adjtime and NTP)" ON)
option (ENABLE_REMOTE_CONTROL "Remotely control wrappers (toggle on/off) via REST-API" ON)
option (ENABLE_FILESYSTEM_METADATA "writes mount points to LOGFILE and/or to INFLUXDB if ENABLE_OUTPUT is set to LOGFILE, INFLUXDB or LOGFILE_AND_INFLUXDB" ON)

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


# needed for the RTLD_NEXT macro and other GNU functions
set(COMPILE_OPTIONS
    -D_GNU_SOURCE
)

#compile using cmake -DLOGGING for write log (can be changed during execution with functions from libiotrace.h)
if(LOGGING)
    list(APPEND COMPILE_OPTIONS -DLOGGING)
endif()

#compile using cmake -DSENDING for sending log to influxdb (can be changed during execution with functions from libiotrace.h)
if(SENDING)
    list(APPEND COMPILE_OPTIONS -DSENDING)
endif()

#compile using cmake -DLOG_WRAPPER_TIME for logging start and end time of function wrappers
if(LOG_WRAPPER_TIME)
    list(APPEND COMPILE_OPTIONS -DLOG_WRAPPER_TIME)
endif()

list(APPEND COMPILE_OPTIONS -DBUFFER_SIZE=${BUFFER_SIZE})
list(APPEND COMPILE_OPTIONS -DMAX_FUNCTION_NAME=${MAX_FUNCTION_NAME})
list(APPEND COMPILE_OPTIONS -DMAX_ERROR_TEXT=${MAX_ERROR_TEXT})
list(APPEND COMPILE_OPTIONS -DMAX_MMSG_MESSAGES=${MAX_MMSG_MESSAGES})
list(APPEND COMPILE_OPTIONS -DMAX_MSG_FILE_DESCRIPTORS=${MAX_MSG_FILE_DESCRIPTORS})
list(APPEND COMPILE_OPTIONS -DMAX_EXEC_ARRAY_LENGTH=${MAX_EXEC_ARRAY_LENGTH})
list(APPEND COMPILE_OPTIONS -DMAX_INFLUX_TOKEN=${MAX_INFLUX_TOKEN})
list(APPEND COMPILE_OPTIONS -DMAX_STACKTRACE_DEPTH=${MAX_STACKTRACE_DEPTH})
list(APPEND COMPILE_OPTIONS -DMAX_STACKTRACE_ENTRY_LENGTH=${MAX_STACKTRACE_ENTRY_LENGTH})
list(APPEND COMPILE_OPTIONS -DMAX_MPI_FILE_HINTS=${MAX_MPI_FILE_HINTS})
list(APPEND COMPILE_OPTIONS -DMAX_MPI_FILE_HINT_LENGTH=${MAX_MPI_FILE_HINT_LENGTH})
list(APPEND COMPILE_OPTIONS -DMAX_MPI_IMESSAGES=${MAX_MPI_IMESSAGES})
list(APPEND COMPILE_OPTIONS -DPORT_RANGE_MIN=${PORT_RANGE_MIN})
list(APPEND COMPILE_OPTIONS -DPORT_RANGE_MAX=${PORT_RANGE_MAX})
list(APPEND COMPILE_OPTIONS -DSELECT_TIMEOUT_SECONDS=${SELECT_TIMEOUT_SECONDS})

if(ALL_WRAPPERS_ACTIVE)
    list(APPEND COMPILE_OPTIONS -DALL_WRAPPERS_ACTIVE=${ALL_WRAPPERS_ACTIVE})
endif()

if(REALTIME)
    list(APPEND COMPILE_OPTIONS -DREALTIME)
endif()

if(ENABLE_REMOTE_CONTROL)
    list(APPEND COMPILE_OPTIONS -DENABLE_REMOTE_CONTROL)
endif()

if(ENABLE_FILESYSTEM_METADATA)
    list(APPEND COMPILE_OPTIONS -DENABLE_FILESYSTEM_METADATA)
endif()

list(APPEND COMPILE_OPTIONS -DSTACKTRACE_DEPTH=${STACKTRACE_DEPTH})
list(APPEND COMPILE_OPTIONS -DENABLE_OUTPUT=${ENABLE_OUTPUT})

#compile using cmake -DSTACKTRACE_PTR for writing stacktrace pointer to log
if(STACKTRACE_PTR)
    list(APPEND COMPILE_OPTIONS -DSTACKTRACE_PTR)
endif()

#compile using cmake -DSTACKTRACE_SYMBOL for writing stacktrace symbols to log
if(STACKTRACE_SYMBOL)
    list(APPEND COMPILE_OPTIONS -DSTACKTRACE_SYMBOL)
endif()

#compile using cmake -DSTRACING_ENABLED for syscall tracing
if (STRACING_ENABLED)
    list(APPEND COMPILE_OPTIONS -DSTRACING_ENABLED)
else()
    # Remove CMake options from cache
    unset(STRACING_LINUX_SRC_DIR CACHE)
    unset(STRACING_UXD_SOCKET_FILEPATH CACHE)
    unset(STRACING_UXD_REG_SOCKET_BACKLOG_SIZE CACHE)
    unset(STRACING_STRACER_LOG_OUTPUT CACHE)
    unset(STRACING_STRACER_TIMEOUT_IN_MSEC CACHE)
endif()

#compile using cmake -DWITH_STD_IO for stdin, stdout and stderr
if(WITH_STD_IO)
    list(APPEND COMPILE_OPTIONS -DWITH_STD_IO)
endif()

#compile using cmake -DWITH_POSIX_IO for Posix-IO
if(WITH_POSIX_IO)
    list(APPEND COMPILE_OPTIONS -DWITH_POSIX_IO)
endif()

#compile using cmake -DWITH_POSIX_AIO for asynchronous Posix-IO
if(WITH_POSIX_AIO)
    if(HAVE_AIO_H)
        list(APPEND COMPILE_OPTIONS -DWITH_POSIX_AIO)
    else()
        message(SEND_ERROR "Missing aio.h for asynchronous Posix-IO.")
    endif()
endif()

#compile using cmake -DWITH_MPI_IO for MPI-IO
if(WITH_MPI_IO)
    list(APPEND COMPILE_OPTIONS -DWITH_MPI_IO)
endif()

#compile using cmake -DWITH_DL_IO for dlopen()
if(WITH_DL_IO)
    list(APPEND COMPILE_OPTIONS -DWITH_DL_IO)
endif()

#compile using cmake -DWITH_ALLOC for memory allocations
if(WITH_ALLOC)
    list(APPEND COMPILE_OPTIONS -DWITH_ALLOC)
endif()

# Compile using cmake -DFILENAME_RESOLUTION_ENABLED for creating mappings b/w file handles and filenames during runtime
if (FILENAME_RESOLUTION_ENABLED)
    list(APPEND COMPILE_OPTIONS -DFILENAME_RESOLUTION_ENABLED)
endif()

if(WITH_MPI_IO)
    find_package(MPI REQUIRED)
    if(MPI_C_FOUND)
        include_directories(${MPI_C_INCLUDE_DIRS})
    else()
        message(SEND_ERROR "Missing MPI-Package.")
    endif()
endif()

include(CheckIncludeFiles)
include(${libiotrace_SOURCE_DIR}/cmake/CheckCompilerAttributes.cmake)
include(${libiotrace_SOURCE_DIR}/cmake/CheckRDTSC.cmake)
CheckCompilerAttributes() 
CheckRDTSC()


# XXX ToDo: check for __uint128_t support (only on 64-bit architecture and some compiler-versions)


include_directories(${libiotrace_SOURCE_DIR}/include)
include_directories(${libiotrace_BINARY_DIR}/include)

add_subdirectory(src)

include(${libiotrace_SOURCE_DIR}/cmake/AddGitSubModule.cmake)

if (WITH_UNIT_TESTS)
    # Solution from https://www.scivision.dev/cmake-git-submodule/
    #
    # If this fails, start anew by adding the cunit git as submodule
    # git rm --cached ext/cunit
    # rm -rf ext
    # git submodule add https://gitlab.com/cunity/cunit.git ext/cunit

    # A better way than just plain COMPILING our own version of CUnit
    # might be to find_package(CUnit REQUIRED) and check for pre-installed version.

    add_git_submodule(ext/cunit)
    
    add_subdirectory(ext)
    
    set(CMAKE_PREFIX_PATH "${PROJECT_SOURCE_DIR}/ext/cunit/CUnit/;${CMAKE_PREFIX_PATH}")
    
    enable_testing()
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
