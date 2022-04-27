
#project(rmind_ringbuf)


# -- Project settings --
#cmake_minimum_required(VERSION 3.3.2)
#set(CMAKE_C_STANDARD 11)
# set(CMAKE_C_COMPILER clang)
#enable_language(C)



# -- Variables --
set(RINGBUF_SOURCES
        ringbuf.c)

# mostly taken 1:1 from https://github.com/rmind/ringbuf/blob/master/src/Makefile
set(RINGBUF_COMPILE_OPTIONS
        -std=c11 -O2 -g -W -Wextra -Werror
        -D_POSIX_C_SOURCE=200809L
        -D_GNU_SOURCE -D_DEFAULT_SOURCE
        -Wno-unknown-warning-option -Wstrict-prototypes -Wmissing-prototypes -Wpointer-arith
        -Wmissing-declarations -Wredundant-decls -Wnested-externs
        -Wshadow -Wcast-qual -Wcast-align -Wwrite-strings
        -Wold-style-definition
        -Wsuggest-attribute=noreturn -Wjump-misses-init
        -Wduplicated-cond -Wmisleading-indentation -Wnull-dereference
        -Wduplicated-branches -Wrestrict
        -fPIC)


# -- "Project" --
add_library(libringbuf_hash_object OBJECT  ${RINGBUF_SOURCES})
target_compile_options(libringbuf_hash_object PUBLIC ${RINGBUF_COMPILE_OPTIONS})
target_include_directories(libringbuf_hash_object SYSTEM INTERFACE ${CMAKE_CURRENT_LIST_DIR})      # TODO: Revise (should be `include`able via <ringbuf/header.h>)
