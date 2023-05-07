

# --- stracer ---
set(EXECUTABLE_OUTPUT_PATH ${libiotrace_BINARY_DIR}/src)        # Otherwise executable will be in 'stracing/stracer'

# -- Variables --
list(APPEND STRACER_SOURCE
        stracer/common/utils.c
        stracer/ipc/uxd_socket.c
        stracer/trace/arch/ptrace_utils.c
        stracer/trace/ptrace_utils.c
        stracer/trace/syscalls.c
        stracer/trace/tracing.c
        stracer/tasks/task_hooks.c
        stracer/tasks/aux/unwind.c
        stracer/cli.c
        stracer/main.c)

list(APPEND STRACER_LINK_OPTIONS
        unwind-ptrace
        unwind-generic
        dw)

list(APPEND STRACER_COMPILE_OPTIONS
        -D_GNU_SOURCE)


# -- CMake options --
set(STRACING_LINUX_SRC_DIR "/usr/src/linux-5.15.0" CACHE STRING "Location of kernel source used to parse syscalls")
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


# - For task "lsep" -
set(STRACING_FNRES_RB_SIZE "16384" CACHE STRING "RingBuffer size for each thread")    # NOTE: CMake option is ALWAYS required for stracing
set(STRACING_FNRES_SCERBMAP_SIZE "1000" CACHE STRING "# of threads which shall be straced")
list(APPEND STRACER_COMPILE_OPTIONS
        -DSTRACING_FNRES_RB_SIZE=${STRACING_FNRES_RB_SIZE}
        -DSTRACING_FNRES_SCERBMAP_SIZE=${STRACING_FNRES_SCERBMAP_SIZE})

list(APPEND STRACER_SOURCE
        stracer/tasks/lsep/scerb/scerb_ipc_utils.c
        stracer/tasks/lsep/scerb/scerb_producer_rmind.c
        stracer/tasks/lsep/scerbmap/scerbmap_atomic_hash.c
        stracer/tasks/lsep/stracing_lsep.c)
list(APPEND STRACER_LINK_OPTIONS
        libringbuf_object
        atomichash-object)


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
set(STRACER_EXEC_FILENAME "libiotrace_stracer")
add_executable(${STRACER_EXEC_FILENAME} ${STRACER_SOURCE})

target_include_directories(libiotrace_stracer PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/stracer"
        "${CMAKE_CURRENT_BINARY_DIR}/stracer")

target_compile_options(libiotrace_stracer PRIVATE
        ${STRACER_COMPILE_OPTIONS})

target_link_libraries(libiotrace_stracer PRIVATE ${STRACER_LINK_OPTIONS})



# --- libiotrace ---
list(APPEND LIBIOTRACE_SOURCE
        stracing/libiotrace/ipc/uxd_socket.c
        stracing/libiotrace/entrypoint.c)
list(APPEND LIBIOTRACE_COMPILE_OPTIONS
        -DSTRACING_STRACER_EXEC_FILENAME="${STRACER_EXEC_FILENAME}"
        -DSTRACING_UXD_SOCKET_FILEPATH="${STRACING_UXD_SOCKET_FILEPATH}"
        -DSTRACING_UXD_REG_SOCKET_BACKLOG_SIZE=${STRACING_UXD_REG_SOCKET_BACKLOG_SIZE})

# - For task "lsep" -
if (FILENAME_RESOLUTION_ENABLED)
    list(APPEND LIBIOTRACE_SOURCE
            stracing/libiotrace/tasks/lsep/scerb/scerb_ipc_utils.c
            stracing/libiotrace/tasks/lsep/scerb/scerb_consumer_rmind.c
            stracing/libiotrace/tasks/lsep/stracing_lsep.c)
    list(APPEND LIBIOTRACE_COMPILE_OPTIONS -DSTRACING_FNRES_RB_SIZE=${STRACING_FNRES_RB_SIZE})
    list(APPEND LIBIOTRACE_SHARED_LINK_OPTIONS libringbuf_object)
    list(APPEND LIBIOTRACE_STATIC_LINK_OPTIONS libringbuf_object)

    set(LIBIOTRACE_SHARED_LINK_OPTIONS ${LIBIOTRACE_SHARED_LINK_OPTIONS} PARENT_SCOPE)
    set(LIBIOTRACE_STATIC_LINK_OPTIONS ${LIBIOTRACE_STATIC_LINK_OPTIONS} PARENT_SCOPE)
endif()

set(LIBIOTRACE_SOURCE ${LIBIOTRACE_SOURCE} PARENT_SCOPE)
set(LIBIOTRACE_COMPILE_OPTIONS ${LIBIOTRACE_COMPILE_OPTIONS} PARENT_SCOPE)
