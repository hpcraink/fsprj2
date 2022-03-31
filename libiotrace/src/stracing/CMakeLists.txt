

# --- stracer ---
set(EXECUTABLE_OUTPUT_PATH ${libiotrace_BINARY_DIR}/src)        # Otherwise executable will be in 'stracing/stracer'

# -- Variables --
list(APPEND STRACER_SOURCE
        stracer/common/utils.c
        stracer/ipc/uxd_socket.c
        stracer/trace/arch/ptrace_utils.c
        stracer/trace/ptrace_utils.c
        stracer/trace/syscall_types.c
        stracer/trace/syscalls.c
        stracer/trace/tracing.c
        stracer/trace/unwind.c
        stracer/cli.c
        stracer/main.c)

set(STRACER_COMPILE_OPTIONS
        -D_GNU_SOURCE)


# -- CMake options --
set(STRACING_LINUX_SRC_DIR "/usr/src/linux-5.4.0" CACHE STRING "Location of kernel source used to parse syscalls")
set(STRACING_UXD_SOCKET_FILEPATH "libiotrace-tracer.sock" CACHE STRING "Filename of socket which is used for IPC b/w tracer & tracee")
set(STRACING_UXD_REG_SOCKET_BACKLOG_SIZE 5000 CACHE STRING "Backlog size for uxd socket")
set(STRACING_STRACER_LOG_OUTPUT "LOGFILE" CACHE STRING "Where the log output of the stracer should go")
set_property(CACHE STRACING_STRACER_LOG_OUTPUT PROPERTY STRINGS "CONSOLE;LOGFILE")
set(STRACING_STRACER_TIMEOUT_IN_MSEC 10000 CACHE STRING "Timeout in ms after which stracer will exit if no tracees are attached")

list(APPEND STRACER_COMPILE_OPTIONS
        -DUXD_SOCKET_FILEPATH="${STRACING_UXD_SOCKET_FILEPATH}"
        -DUXD_REG_SOCKET_BACKLOG_SIZE=${STRACING_UXD_REG_SOCKET_BACKLOG_SIZE}
        -DEXIT_TIMEOUT_IN_MS=${STRACING_STRACER_TIMEOUT_IN_MSEC})

if (STRACING_STRACER_LOG_OUTPUT STREQUAL "LOGFILE")
    list(APPEND STRACER_COMPILE_OPTIONS "-DUSE_LOGFILE")
endif()


# -- Build targets/commands --
# Build targets/commands: Parse syscalls + generate source
set(GEN_SYSCALLS_SCRIPT "${CMAKE_CURRENT_LIST_DIR}/stracer/scripts/gen_stracing_syscalls_table.py")
set(GEN_SYSCALLS_TARGET_DIR "${CMAKE_CURRENT_BINARY_DIR}/stracer/trace/generated")

find_package(PythonInterp 3.4 REQUIRED)
add_custom_command(
        COMMENT "Parse syscalls from kernel source and generate source files"
        DEPENDS ${GEN_SYSCALLS_SCRIPT}
        OUTPUT "${GEN_SYSCALLS_TARGET_DIR}/syscallents.c" "${GEN_SYSCALLS_TARGET_DIR}/syscallents.h"
        COMMAND ${CMAKE_COMMAND} -E make_directory ${GEN_SYSCALLS_TARGET_DIR}
        COMMAND ${PYTHON_EXECUTABLE} ${GEN_SYSCALLS_SCRIPT} ${STRACING_LINUX_SRC_DIR} ${GEN_SYSCALLS_TARGET_DIR})

list(APPEND STRACER_SOURCE
        ${GEN_SYSCALLS_TARGET_DIR}/syscallents.h        # TODO: REVISE
        ${GEN_SYSCALLS_TARGET_DIR}/syscallents.c)


# -- Project --
# WARNING: Changing the name of the executable requires also an update in the source (otherwise `exec` will fail)
add_executable(libiotrace_stracer
        ${STRACER_SOURCE})

target_include_directories(libiotrace_stracer PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/stracer"
        "${CMAKE_CURRENT_BINARY_DIR}/stracer")

target_compile_options(libiotrace_stracer PRIVATE
        ${STRACER_COMPILE_OPTIONS})

target_link_libraries(libiotrace_stracer PRIVATE unwind-ptrace unwind-generic dw iberty)        # TODO: RMV C++ demangling



# --- libiotrace ---
list(APPEND SOURCE
        stracing/libiotrace/ipc/uxd_socket.c
        stracing/libiotrace/entrypoint.c)
set(SOURCE ${SOURCE} PARENT_SCOPE)

list(APPEND COMPILE_OPTIONS
        -DSTRACING_UXD_SOCKET_FILEPATH="${STRACING_UXD_SOCKET_FILEPATH}"
        -DSTRACING_UXD_REG_SOCKET_BACKLOG_SIZE=${STRACING_UXD_REG_SOCKET_BACKLOG_SIZE})
set(COMPILE_OPTIONS ${COMPILE_OPTIONS} PARENT_SCOPE)
