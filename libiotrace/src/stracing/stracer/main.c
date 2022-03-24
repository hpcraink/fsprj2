#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

#include "cli.h"
#include "ipc/uxd_socket.h"

#include "common/error.h"
#define DEV_DEBUG_ENABLE_LOGS
#include "common/debug.h"



#define TESTING_DISABLE_LOGFILE   // TODO: RMV LATER ...

// TODO do_tracer, containing following fct ..
/*
TODO: Issues:

  Test #1: `(cd test && IOTRACE_LOG_NAME=stracing_tasks_test1 LD_PRELOAD=../src/libiotrace_shared.so ./stracing_trace_pthread_fork --pthread)`
    <<libiotrace>> [DEBUG] `stracing_init_stracer` (/mnt/hgfs/fpj/fsprj2/libiotrace/src/stracing/libiotrace/entrypoint.c:30): [PARENT:tid=32713] A stracer instance is already running.
    <<stracer>> [DEBUG] `main` (/mnt/hgfs/fpj/fsprj2/libiotrace/src/stracing/stracer/main.c:87): I'm still running (pid=32694)...
    <<libiotrace>> [DEBUG] `stracing_register_with_stracer` (/mnt/hgfs/fpj/fsprj2/libiotrace/src/stracing/libiotrace/entrypoint.c:85): [PARENT:tid=32713] Sending tracing request.
    Creating additional thread ...
    <<stracer>> [DEBUG] `main` (/mnt/hgfs/fpj/fsprj2/libiotrace/src/stracing/stracer/main.c:91): Received tracing request from pid=32713.
    tid = 32713, pid = 32713, ppid =  3032, pgid = 32713, sid =  3032 [Thread-0]
    tid = 32714, pid = 32713, ppid =  3032, pgid = 32713, sid =  3032 [Thread-1]


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


   No spinning ??!
 */



int main(int argc, char** argv) {
#ifndef TESTING_DISABLE_LOGFILE
/* Setup a logfile (since we can't log to console) */
    DIE_WHEN_ERRNO( close(STDIN_FILENO) );
    FILE* stdout_logfile = (FILE*)DIE_WHEN_ERRNO_VPTR( freopen(STRACING_STRACER_LOGFILE, "a+", stdout) );
    FILE* stderr_logfile = (FILE*)DIE_WHEN_ERRNO_VPTR( freopen(STRACING_STRACER_LOGFILE, "a+", stderr) );
    if (0 != setvbuf(stdout_logfile, NULL, _IONBF, 0) ||
        0 != setvbuf(stderr_logfile, NULL, _IONBF, 0)) {
        LOG_ERROR_AND_EXIT("Couldn't set buffering options for logfile");
    }
#endif /* TESTING_DISABLE_LOGFILE */

/* (0) Parse CLI args */
    cli_args_t parsed_cli_args;
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
#ifndef TESTING_DISABLE_LOGFILE
        fclose(stdout_logfile);
        fclose(stderr_logfile);
#endif /* TESTING_DISABLE_LOGFILE */
        close(uxd_reg_sock_fd);
        pause();
        _exit(0);
    }

    kill(getppid(), SIGKILL);   /* Grandchild = Tracer */


/* (2) Start tracing .. */
    const pid_t tracer_pid = getpid();
    DEV_DEBUG_PRINT_MSG("Ready for tracing requests (running under pid=%d) ..", tracer_pid);

    for (;;) {           // For testing only ..
        uxd_sock_ipc_requests_t ipc_request; pid_t cr_pid;
        if (-1 == receive_new_uxd_ipc_events(uxd_reg_sock_fd,
                                                &ipc_request, &cr_pid)) {
            LOG_DEBUG("Spinning");
            continue;
        }

        switch (ipc_request.request) {
            case PROBE_TRACER_RUNNING:
                LOG_DEBUG("I'm still running (pid=%d)..", tracer_pid);
                break;

            case TRACEE_REQUEST_TRACING:
                DEV_DEBUG_PRINT_MSG("Received tracing request from pid=%d", cr_pid);
                break;

            default:
                LOG_WARN("Invalid ipc-request");
        }
    }


/* (3) Cleanup on exit .. */
    // TODO: Register at_exit -> cleanup
    fin_uxd_reg_socket(uxd_reg_sock_fd, STRACING_UXD_SOCKET_FILEPATH);
#ifndef TESTING_DISABLE_LOGFILE
    fclose(stdout_logfile);
    fclose(stderr_logfile);
#endif /* TESTING_DISABLE_LOGFILE */

    return 0;
}
