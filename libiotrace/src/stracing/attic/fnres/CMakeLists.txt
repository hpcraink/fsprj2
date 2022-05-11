# ensure debug info and assert functions are build during compile of tests    --> TODO: LATER ONLY FOR test
set(CMAKE_BUILD_TYPE Debug)


# --- Project config ---
project(ringbuffer-testing)
cmake_minimum_required(VERSION 3.3.2)

enable_language(C)

set(CMAKE_C_STANDARD 11)



set(SCERB_TEST_SOURCES
        testing_scerb.c)
set(SCERB_TEST_COMPILE_OPTIONS -O0)


# - ringbuffer -


# -- Test program --
# - CMake options -


if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    list(APPEND SCERB_TEST_LINK_OPTIONS
            rt pthread)   # Note: POSIX Realtime Extensions + pthread library are required for semaphores
endif()


add_executable(testing_scerb ${SCERB_TEST_SOURCES})
target_compile_options(testing_scerb PRIVATE ${SCERB_TEST_COMPILE_OPTIONS})
target_link_libraries(testing_scerb ${SCERB_TEST_LINK_OPTIONS})
