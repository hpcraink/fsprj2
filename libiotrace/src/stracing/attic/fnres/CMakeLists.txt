# --- Project config ---
project(ringbuffer-testing)
cmake_minimum_required(VERSION 3.3.2)

enable_language(C)

set(CMAKE_C_STANDARD 11)


# --- Actual executable ---
set(SOURCES
            iface/scerb.c
            testing_scerb.c)
set(COMPILE_OPTIONS -O0)


# -- Actual ringbuffer --
list(APPEND SOURCES
        iface/scerb.c
        iface/scerb_rmind_ringbuf.c)
add_subdirectory(rmind-ringbuf)
list(APPEND LINK_OPTIONS libringbuf_hash_object)


# -- Test program --
if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    list(APPEND LINK_OPTIONS
            rt pthread)   # Note: POSIX Realtime Extensions + pthread library are required for semaphores
endif()


add_executable(testing_scerb ${SOURCES})
target_compile_options(testing_scerb PRIVATE ${COMPILE_OPTIONS})
target_link_libraries(testing_scerb ${LINK_OPTIONS})
