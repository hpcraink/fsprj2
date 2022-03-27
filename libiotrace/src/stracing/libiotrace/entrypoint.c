#include <unistd.h>
#include "../../event.h"

#include <sys/un.h>
#include <sys/prctl.h>
#include <sys/wait.h>

#include "entrypoint.h"
#include "../common/stracer.h"
#include "ipc/uxd_socket.h"

#include <assert.h>
#include "../../error.h"  /* NOTE: Already incl. via `event.h` */
#define DEV_DEBUG_ENABLE_LOGS
#include "../../debug.h"


/* -- Globals / Consts -- */
#define STRACER_EXEC_FILENAME "libiotrace_stracer"
#define SYSCALLS_TO_BE_TRACED "open"


/* --- Functions --- */
void stracing_init_stracer(char *ld_preload_env_val) {
    assert(ld_preload_env_val);                         // TODO: REVISE `assert`

/* (0) Parent: Establish tracer's UXD registration socket */
    int uxd_reg_sock_fd;
    if (-1 == (uxd_reg_sock_fd = init_uxd_ipc_socket(STRACING_UXD_SOCKET_FILEPATH, STRACING_UXD_REG_SOCKET_BACKLOG_SIZE))) {
        DEV_DEBUG_PRINT_MSG("[PARENT:tid=%ld] A stracer instance is already running", gettid());
        return;
    }
    DEV_DEBUG_PRINT_MSG("[PARENT:tid=%ld] Inited UXD registration socket \"%s\" w/ backlog=%d for stracer",
                        gettid(),
                        STRACING_UXD_SOCKET_FILEPATH, STRACING_UXD_REG_SOCKET_BACKLOG_SIZE);

/* (1) Parent: `fork`, let child launch stracer, and proceed ... */
    if (DIE_WHEN_ERRNO( CALL_REAL_POSIX_SYNC(fork)() )) {
        CALL_REAL_POSIX_SYNC(close)(uxd_reg_sock_fd);
        return;
    }

/* (2) Child: `exec` stracer (which will `fork` again to create the grandchild, which will be the actual tracer) */
    /* Prepare `exec` arg: executable filename */
    char *exec_arg_exec_fname;
{
    char* ld_preload_basedir;
    DIE_WHEN_ERRNO_VPTR( (ld_preload_basedir = strdup(ld_preload_env_val)) );           // TODO: SECURITY CONCERNS (LD_PRELOAD IS PASSED BY USER)
    DIE_WHEN_ERRNO( dirname_n(ld_preload_basedir, strlen(ld_preload_basedir) +1) );

    DIE_WHEN_ERRNO( asprintf(&exec_arg_exec_fname, "%s/%s", ld_preload_basedir, STRACER_EXEC_FILENAME) );
    CALL_REAL_ALLOC_SYNC(free)(ld_preload_basedir);
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
                        exec_arg_exec_fname, exec_arg_sock_fd, exec_syscall_subset, exec_arg_tasks);
    CALL_REAL(execle)(exec_arg_exec_fname,
                      exec_arg_exec_fname,      /* CLI args */
                      exec_arg_sock_fd,
                      exec_syscall_subset,
                      exec_arg_tasks,
                      NULL,
                      NULL);                    /* Envs (make sure NO `LD_PRELOAD` is passed) */
    LIBIOTRACE_ERROR("`exec` failed (errno=%d)", errno);
}


void stracing_register_with_stracer(void) {
/* (0) Prepare tracee 4 being tracer -> Set permissions (ONLY PERTINENT when Yama ptrace_scope = 1 AND `PTRACE_ATTACH` is used) */
//    DIE_WHEN_ERRNO( prctl(PR_SET_PTRACER, PR_SET_PTRACER_ANY) );

/* (1) Send tracing request */
    DEV_DEBUG_PRINT_MSG("[PARENT:tid=%ld] Sending tracing request", gettid());
    send_tracing_uxd_ipc_request(STRACING_UXD_SOCKET_FILEPATH);

/* (2) Wait for tracer to wake us up ...  */
//    while (wait(NULL) < 0 && EINTR == errno);
}
