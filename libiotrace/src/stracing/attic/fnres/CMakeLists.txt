# ensure debug info and assert functions are build during compile of tests    --> TODO: LATER ONLY FOR test
set(CMAKE_BUILD_TYPE Debug)


# --- Project config ---
project(ringbuffer-testing)
cmake_minimum_required(VERSION 3.3.2)

enable_language(C)

set(CMAKE_C_STANDARD 11)



set(SCERB_TEST_SOURCES
        iface/common/scerb.c
        testing_scerb.c)
set(SCERB_TEST_COMPILE_OPTIONS -O0)


# - ringbuffer -
list(APPEND SCERB_TEST_SOURCES
        iface/common/scerb.c
        iface/common/sm_ipc_utils.c
        iface/libiotrace/scerb_consumer_rmind.c
        iface/stracer/scerb_producer_rmind.c)
add_subdirectory(rmind-ringbuf)
list(APPEND SCERB_TEST_LINK_OPTIONS libringbuf_hash_object)


# -- Test program --
# - CMake options -
set(STRACE_FNRES_RB_SIZE "16384" CACHE STRING "RingBuffer size for each thread")
list(APPEND SCERB_TEST_COMPILE_OPTIONS -DSTRACE_FNRES_RB_SIZE=${STRACE_FNRES_RB_SIZE})

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    list(APPEND SCERB_TEST_LINK_OPTIONS
            rt pthread)   # Note: POSIX Realtime Extensions + pthread library are required for semaphores
endif()


add_executable(testing_scerb ${SCERB_TEST_SOURCES})
target_compile_options(testing_scerb PRIVATE ${SCERB_TEST_COMPILE_OPTIONS})
target_link_libraries(testing_scerb ${SCERB_TEST_LINK_OPTIONS})
