# --- Project config ---
project(ringbuffer-testing)
cmake_minimum_required(VERSION 3.3.2)

enable_language(C)

set(CMAKE_C_STANDARD 11)



add_subdirectory(ext/lwrb/lwrb)


# --- Actual executable ---
set(SOURCES
            iface/scerb.c
            iface/scerb_lwrb.c
            testing_scerb.c)

set(LINK_OPTIONS
            lwrb)
if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    list(APPEND LINK_OPTIONS
            rt pthread)   # Note: POSIX Realtime Extensions + pthread library are required for semaphores
endif()

add_executable(testing_scerb ${SOURCES})
target_link_libraries(testing_scerb ${LINK_OPTIONS})
