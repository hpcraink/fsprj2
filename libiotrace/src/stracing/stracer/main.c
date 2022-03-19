#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

#include "../common/stracer.h"
#include "cli.h"
#include "ipc/uxd_socket.h"

#include "common/error.h"
#define DEV_DEBUG_ENABLE_LOGS
#include "common/debug.h"


int main(int argc, char** argv) {

/* (0) Parse CLI args */
    cli_args parsed_cli_args;
    parse_cli_args(argc, argv, &parsed_cli_args);
#ifdef DEV_DEBUG_ENABLE_LOGS
    print_parsed_cli_args(&parsed_cli_args);
#endif
    const int uxd_reg_sock_fd = parsed_cli_args.uxd_reg_sock_fd;

/* (0.1) Check whether fildes is valid  (TODO: check whether socket) */
    if (fcntl(uxd_reg_sock_fd, F_GETFL) < 0 && EBADF == errno) {
        LOG_ERROR_AND_EXIT("Invalid socket fildes");
    }


/* (1) Fork grandchild (which will be the actual tracer) */
    if (DIE_WHEN_ERRNO( fork() )) {     /* Child -> not used */
        close(uxd_reg_sock_fd);
        pause();
        _exit(0);
    }

    kill(getppid(), SIGKILL);   /* Grandchild = Tracer */


/* (2) Start tracing .. */
    const pid_t tracer_pid = getpid();
    DEV_DEBUG_PRINT_MSG("[TRACER:pid=%d] Ready for tracing requests ..", tracer_pid);

    // TODO do_tracer, containing following fct ..
    /*
      Test #1: `(cd test && IOTRACE_LOG_NAME=stracing_tasks_test1 LD_PRELOAD=../src/libiotrace_shared.so ./stracing_trace_descendants)`
        Issues:
            - How to check w/o unblocking
            - Each request -> 2x registered by tracer
                <<stracer>> [DEBUG] `main` (/mnt/hgfs/fpj/fsprj2/libiotrace/src/stracing/stracer/main.c:50): Received tracing request from pid=7672.
                <<libiotrace>> [DEBUG] `stracing_register_with_stracer` (/mnt/hgfs/fpj/fsprj2/libiotrace/src/stracing/entrypoint.c:80): [PARENT:tid=7672] Sending tracing request.
                <<stracer>> [DEBUG] `main` (/mnt/hgfs/fpj/fsprj2/libiotrace/src/stracing/stracer/main.c:50): Received tracing request from pid=7672.

      Test #2: `(cd test && IOTRACE_LOG_NAME=stracing_tasks_test1 LD_PRELOAD=../src/libiotrace_shared.so ./stracing_trace_pthread_fork --fork)`
         Issue:
            - Sends pid, not tid
                <<libiotrace>> [DEBUG] `stracing_init_stracer` (/mnt/hgfs/fpj/fsprj2/libiotrace/src/stracing/entrypoint.c:29): [PARENT:tid=7827] A stracer instance is already running.
                <<stracer>> [DEBUG] `main` (/mnt/hgfs/fpj/fsprj2/libiotrace/src/stracing/stracer/main.c:50): Received tracing request from pid=7827.
                <<libiotrace>> [DEBUG] `stracing_register_with_stracer` (/mnt/hgfs/fpj/fsprj2/libiotrace/src/stracing/entrypoint.c:80): [PARENT:tid=7827] Sending tracing request.
                Creating additional thread ...
                <<stracer>> [DEBUG] `main` (/mnt/hgfs/fpj/fsprj2/libiotrace/src/stracing/stracer/main.c:50): Received tracing request from pid=7827.
                tid =  7827, pid =  7827, ppid =  3017, pgid =  7827, sid =  3017 [Thread-0]
                tid =  7828, pid =  7827, ppid =  3017, pgid =  7827, sid =  3017 [Thread-1]
     */
    for (;;) {           // For testing only ..
        pid_t new_tracee;
        if (-1 != (new_tracee = check_for_new_tracees(uxd_reg_sock_fd))) {
            DEV_DEBUG_PRINT_MSG("Received tracing request from pid=%d", new_tracee);
        } else {
            LOG_DEBUG("I'm still running (pid=%d)..", tracer_pid);
        }
    }


/* (3) Cleanup on exit .. */
    // TODO: Register at_exit -> cleanup
    fin_uxd_reg_socket(uxd_reg_sock_fd, UXD_SOCKET_FILEPATH);

    return 0;
}
