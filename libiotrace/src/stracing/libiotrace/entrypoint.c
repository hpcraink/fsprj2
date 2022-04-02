/**
 * Known ISSUEs:
 *   - Traced programs which have STATICALLY LINKED functions, which in turn are normally
 *     (a) wrapped by libiotrace, and (b) used by libiotrace to perform an THREAD init
 *     (e.g., `pthread_create`(3)), may not be traced properly (won't send a tracing request)
 *     NOTE: A possible solution would be to set `PTRACE_O_TRACECLONE`, etc. for tracing options
 *           in stracer (since the process init via '.ctors' is guaranteed)
 *
 * TODOs:
 *   - ...
 */
#include <unistd.h>
#include "../../event.h"

#include <sys/un.h>
#include <sys/prctl.h>

#include "entrypoint.h"
#include "../common/stracer.h"
#include "ipc/uxd_socket.h"

#include <assert.h>
#include "../../error.h"  /* NOTE: Already incl. via `event.h` */
#define DEV_DEBUG_ENABLE_LOGS
#include "../../debug.h"


/* -- Globals / Consts -- */
#define SYSCALLS_TO_BE_TRACED "open,openat"


/* --- Functions --- */
void stracing_init_stracer(void) {
/* (0) Parent: Establish tracer's UXD registration socket */
    int uxd_reg_sock_fd;
    if (-1 == (uxd_reg_sock_fd = uxd_ipc_parent_sock_init(STRACING_UXD_SOCKET_FILEPATH,
                                                          STRACING_UXD_REG_SOCKET_BACKLOG_SIZE))) {
        DEV_DEBUG_PRINT_MSG("[PARENT:tid=%ld] A stracer instance is already running", gettid());
        return;
    }

    DEV_DEBUG_PRINT_MSG("[PARENT:tid=%ld] Inited UXD registration socket \"%s\" w/ backlog=%d for to be launched stracer",
                        gettid(),
                        STRACING_UXD_SOCKET_FILEPATH, STRACING_UXD_REG_SOCKET_BACKLOG_SIZE);

/* (1) Parent: `fork`, let child launch stracer, and proceed ... */
    if (DIE_WHEN_ERRNO( CALL_REAL_POSIX_SYNC(fork)() )) {
        CALL_REAL_POSIX_SYNC(close)(uxd_reg_sock_fd);
        return;
    }


/* (2) Child: `exec` stracer (which will `fork` again to create the grandchild, which will be the actual tracer) */
    /* Prepare `exec` arg: stracer executable filename + TODO: current executable filename (for stack unwinding) */
    char *exec_arg_stracer_exec_fname;
{
    char* current_exec_file_path = DIE_WHEN_ERRNO_VPTR( get_path_to_file_containing_this_fct() );
{   /* >>   Derive path to stracer's executable from libiotrace so filepath   (!! MUST THEREFORE BE IN SAME DIR !!) */
    char* current_exec_dir_path = DIE_WHEN_ERRNO_VPTR(strdup(current_exec_file_path) );
    DIE_WHEN_ERRNO( dirname_r(current_exec_file_path, current_exec_dir_path, strlen(current_exec_dir_path) + 1) );
    DIE_WHEN_ERRNO( asprintf(&exec_arg_stracer_exec_fname, "%s/%s", current_exec_dir_path, STRACING_STRACER_EXEC_FILENAME) );
    CALL_REAL_ALLOC_SYNC(free)(current_exec_dir_path);
}

    // TODO: current executable filename (for stack unwinding)

    CALL_REAL_ALLOC_SYNC(free)(current_exec_file_path);
}

    /* Prepare `exec` arg: Fildes of socket */
    char *exec_arg_sock_fd;
    DIE_WHEN_ERRNO( asprintf(&exec_arg_sock_fd, "-%c=%d", STRACER_CLI_OPTION_SOCKFD, uxd_reg_sock_fd) );

    /* Prepare `exec` arg: Syscall subset */
    char *exec_syscall_subset;
    DIE_WHEN_ERRNO( asprintf(&exec_syscall_subset, "-%c=%s", STRACER_CLI_OPTION_SSUBSET, SYSCALLS_TO_BE_TRACED) );

    /* Prepare `exec` arg: Tasks  (TODO: allow user to choose during runtime) */
    const char* const exec_arg_tasks = "-w";

    /* Perform `exec` */
    DEV_DEBUG_PRINT_MSG("[CHILD:tid=%ld] Launching stracer via `%s %s %s %s`", gettid(),
                        exec_arg_stracer_exec_fname, exec_arg_sock_fd, exec_syscall_subset, exec_arg_tasks);
    CALL_REAL(execle)(exec_arg_stracer_exec_fname,
                      exec_arg_stracer_exec_fname,      /* CLI args */
                      exec_arg_sock_fd,
                      exec_syscall_subset,
                      exec_arg_tasks,
                      NULL,
                      NULL);                    /* Envs (make sure NO `LD_PRELOAD` is passed) */
    LIBIOTRACE_ERROR("`exec` of stracer failed -- %s", strerror(errno));
}


void stracing_register_with_stracer(void) {
/* (0) Set tracing permissions (only necessary when Yama ptrace_scope = 1; check: `cat /proc/sys/kernel/yama/ptrace_scope`) */
    DIE_WHEN_ERRNO( prctl(PR_SET_PTRACER, PR_SET_PTRACER_ANY) );

/* (1) Send tracing request */
    DEV_DEBUG_PRINT_MSG("[PARENT:tid=%ld] Sending tracing request", gettid());
    int server_conn_fd;
    uxd_ipc_tracee_send_tracing_req(STRACING_UXD_SOCKET_FILEPATH, &server_conn_fd);

/* (2) Block until it's tracer sends acknowledgement (i.e., we're now being actively traced) */
    uxd_ipc_tracee_block_until_tracing_ack(server_conn_fd);
    CALL_REAL_POSIX_SYNC(close)(server_conn_fd);

    DEV_DEBUG_PRINT_MSG("[TRACEE:tid=%ld] Got attached by stracer, proceeding ...", gettid());
}
