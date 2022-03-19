
set(EXECUTABLE_OUTPUT_PATH ${libiotrace_BINARY_DIR}/src)        # Otherwise executable will be in 'stracing/stracer'


# -- Variables --
list(APPEND STRACER_SOURCE
        common/utils.c
        ipc/uxd_socket.c
        trace/arch/ptrace_utils.c
        trace/ptrace_utils.c
        trace/syscalls.c
        cli.c
        main.c)


# -- CMake options --
set(LINUX_SRC_DIR "/usr/src/linux-5.4.0" CACHE STRING "Location of kernel source used to parse syscalls")


# -- Build targets/commands --
# Build targets/commands: Parse syscalls + generate source
set(GEN_SYSCALLS_SCRIPT "${CMAKE_CURRENT_LIST_DIR}/scripts/gen_stracing_syscalls_table.py")
set(GEN_SYSCALLS_TARGET_DIR "${CMAKE_CURRENT_BINARY_DIR}/trace/generated")

find_package(PythonInterp 3.4 REQUIRED)
add_custom_command(
        COMMENT "Parse syscalls from kernel source and generate source files"
        DEPENDS ${GEN_SYSCALLS_SCRIPT}
        OUTPUT "${GEN_SYSCALLS_TARGET_DIR}/syscallents.c" "${GEN_SYSCALLS_TARGET_DIR}/syscallents.h"

        COMMAND ${CMAKE_COMMAND} -E make_directory ${GEN_SYSCALLS_TARGET_DIR}
        COMMAND ${PYTHON_EXECUTABLE} ${GEN_SYSCALLS_SCRIPT} ${LINUX_SRC_DIR} ${GEN_SYSCALLS_TARGET_DIR})

list(APPEND STRACER_SOURCE
        ${GEN_SYSCALLS_TARGET_DIR}/syscallents.h        # TODO: REVISE
        ${GEN_SYSCALLS_TARGET_DIR}/syscallents.c)


# -- Project --
# WARNING: Changing the name of the executable requires also an update in the source (otherwise `exec` will fail)
add_executable(libiotrace_stracer
        ${STRACER_SOURCE})

target_compile_options(libiotrace_stracer PRIVATE -D_GNU_SOURCE)

# target_link_libraries(libiotrace_stracer PRIVATE unwind-ptrace unwind-generic dw)

# message(STATUS "`libiotrace_stracer` include dirs: ${CMAKE_CURRENT_LIST_DIR} AND ${CMAKE_CURRENT_BINARY_DIR}")
target_include_directories(libiotrace_stracer PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}"
        "${CMAKE_CURRENT_BINARY_DIR}")
