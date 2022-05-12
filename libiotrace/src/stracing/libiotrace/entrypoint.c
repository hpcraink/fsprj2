/**
 * Known ISSUEs:
 *   - Traced programs which have STATICALLY LINKED functions, which in turn are normally
 *     (a) wrapped by libiotrace, and (b) used by libiotrace to trigger an THREAD init
 *     (e.g., `pthread_create`(3)), may not be traced properly (won't send a tracing request)
 *     NOTE: A possible remedy would be to also set the ptrace flags `PTRACE_O_TRACECLONE`, etc.,
 *           (since the process init via '.ctors' is guaranteed)
 *
 * TODOs:
 *   - ...
 */
#include <unistd.h>
#include "../../event.h"

#include <sys/un.h>
#include <sys/prctl.h>

#include "entrypoint.h"
#include "../common/stracer_consts.h"
#include "ipc/uxd_socket.h"

#include "../../common/error.h"  /* NOTE: Already incl. via `event.h` */
//#define DEV_DEBUG_ENABLE_LOGS
#include "../../common/debug.h"

#ifdef FILENAME_RESOLUTION_ENABLED
#  include "tasks/fnres/stracing_fnres.h"
#endif


/* -- Globals / Consts -- */
#define SYSCALLS_TO_BE_TRACED "open,openat,close"           // TODO: Allow user based selection during runtime


/* --- Functions --- */
void stracing_init_stracer(void) {
/* 0. Parent: Establish tracer's UXD registration socket */
    int uxd_reg_sock_fd;
    if (-1 == (uxd_reg_sock_fd = uxd_ipc_parent_sock_init(STRACING_UXD_SOCKET_FILEPATH,
                                                          STRACING_UXD_REG_SOCKET_BACKLOG_SIZE))) {
        DEV_DEBUG_PRINT_MSG("[PARENT:tid=%ld] A stracer instance is already running", gettid());
        return;
    }

    DEV_DEBUG_PRINT_MSG("[PARENT:tid=%ld] Inited UXD registration socket \"%s\" w/ backlog=%d for to be launched stracer",
                        gettid(),
                        STRACING_UXD_SOCKET_FILEPATH, STRACING_UXD_REG_SOCKET_BACKLOG_SIZE);

/* 1a. Parent: `fork`, let child launch stracer, and proceed ... */
    if (DIE_WHEN_ERRNO( CALL_REAL_POSIX_SYNC(fork)() )) {
        CALL_REAL_POSIX_SYNC(close)(uxd_reg_sock_fd);
        return;
    }


/* 1b. Child: `exec` stracer (which will `fork` again to create the grandchild, which will be the actual tracer) */
    /* Prepare `exec` arg: stracer's executable filename + libiotrace linkage info (necessary for stack unwinding) */
    char *exec_arg_stracer_exec_fname,
         *exec_arg_libiotrace_linkage;
{
    char *tmp_current_exec_file_path = DIE_WHEN_ERRNO_VPTR( get_path_to_file_containing_this_fct() ),
         *tmp_buf =                    DIE_WHEN_ERRNO_VPTR( strdup(tmp_current_exec_file_path) );
    const size_t tmp_buf_size =        strlen(tmp_buf) +1;

/* >>   `exec_arg_stracer_exec_fname`: Derive path to stracer's executable from libiotrace so filepath   (!! MUST THEREFORE BE IN SAME DIR !!) */
    DIE_WHEN_ERRNO( dirname_r(tmp_current_exec_file_path, tmp_buf, tmp_buf_size) );
    DIE_WHEN_ERRNO( asprintf(&exec_arg_stracer_exec_fname, "%s/%s",
                             tmp_buf, STRACING_STRACER_EXEC_FILENAME) );

/* >>   `exec_arg_libiotrace_linkage`: Derive executable name from libiotrace so filepath */
    DIE_WHEN_ERRNO( basename_r(tmp_current_exec_file_path, tmp_buf, tmp_buf_size) );
    const char cli_libiotrace_linkage
#ifdef IO_LIB_STATIC
            = STRACER_CLI_LIBIOTRACE_LINKAGE_STATIC;
#else
            = STRACER_CLI_LIBIOTRACE_LINKAGE_SHARED;
#endif /* IO_LIB_STATIC */
    DIE_WHEN_ERRNO( asprintf(&exec_arg_libiotrace_linkage, "-%c=%c:%s",
                             STRACER_CLI_OPTION_LIBIOTRACE_LINKAGE, cli_libiotrace_linkage,
                             tmp_buf) );

    CALL_REAL_ALLOC_SYNC(free)(tmp_current_exec_file_path);
    CALL_REAL_ALLOC_SYNC(free)(tmp_buf);
}

    /* Prepare `exec` arg: Fildes of socket */
    char *exec_arg_sock_fd;
    DIE_WHEN_ERRNO( asprintf(&exec_arg_sock_fd, "-%c=%d", STRACER_CLI_OPTION_SOCKFD, uxd_reg_sock_fd) );

    /* Prepare `exec` arg: Syscall subset */
    char *exec_syscall_subset;
    DIE_WHEN_ERRNO( asprintf(&exec_syscall_subset, "-%c=%s", STRACER_CLI_OPTION_SSUBSET, SYSCALLS_TO_BE_TRACED) );

    /* Perform `exec` */
    DEV_DEBUG_PRINT_MSG("[CHILD:tid=%ld] Launching stracer via `%s %s %s %s %s`", gettid(),
                        exec_arg_stracer_exec_fname, exec_arg_sock_fd, exec_syscall_subset, exec_arg_libiotrace_linkage, exec_arg_tasks);
    CALL_REAL(execle)(exec_arg_stracer_exec_fname,
                      exec_arg_stracer_exec_fname,      /* CLI args */
                      exec_arg_sock_fd,
                      exec_syscall_subset,
                      exec_arg_libiotrace_linkage,
                      "-w",                             //  WARN task    TODO: Runtime selection of to be performed tasks by user
#ifdef FILENAME_RESOLUTION_ENABLED
                      "-f",                             // FNRES task  (CAPTAIN OBVIOUS: requires filename resolution)
#endif
                      NULL,
                      NULL);                            /* Envs (make sure NO `LD_PRELOAD` is passed, otherwise we can't "break out" of libiotrace's tracing) */
    LOG_ERROR_AND_EXIT("stracer `exec` failed -- %s%s", strerror(errno),
                       ENOENT == errno ? ("\nKEEP IN MIND: The stracer's executable may be in the same directory in which libiotrace (dynamically or statically linked) is located") : (""));
}


void stracing_register_with_stracer(void) {
/* 0. SETUP */
  /* Set tracing permissions (only necessary when Yama ptrace_scope = 1; check current settings: `cat /proc/sys/kernel/yama/ptrace_scope`) */
    DIE_WHEN_ERRNO( prctl(PR_SET_PTRACER, PR_SET_PTRACER_ANY) );
#ifdef FILENAME_RESOLUTION_ENABLED
  /* Create scerb for receiving syscall events (traced by stracer) */
    stracing_fnres_init();
    DEV_DEBUG_PRINT_MSG("[PARENT:tid=%ld] Created scerb for fnres", gettid());
#endif

/* 1. Send tracing request to stracer */
    DEV_DEBUG_PRINT_MSG("[PARENT:tid=%ld] Sending tracing request", gettid());
    int server_conn_fd;
    uxd_ipc_tracee_send_tracing_req(STRACING_UXD_SOCKET_FILEPATH, &server_conn_fd);

/* 2. Block execution until stracer acknowledges request (i.e., until we're attached) */
    uxd_ipc_tracee_block_until_tracing_ack(server_conn_fd);
    CALL_REAL_POSIX_SYNC(close)(server_conn_fd);

    DEV_DEBUG_PRINT_MSG("[TRACEE:tid=%ld] Got attached by stracer, proceeding ...", gettid());
}


void stracing_fin(void) {
#ifdef FILENAME_RESOLUTION_ENABLED
    stracing_fnres_fin();
    DEV_DEBUG_PRINT_MSG("[PARENT:tid=%ld] Detached scerb for fnres", gettid());
#endif
}
