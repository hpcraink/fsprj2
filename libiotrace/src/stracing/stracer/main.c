#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

#include "cli.h"
#include "ipc/uxd_socket.h"
// #include "trace/tracing.h"

#include "common/error.h"
#define DEV_DEBUG_ENABLE_LOGS
#include "common/debug.h"

/* -- Consts -- */
#ifdef USE_LOGFILE
#  define LOGFILE_NAME "libiotrace_stracer.log"
#endif /* USE_LOGFILE */



int main(int argc, char** argv) {
/* (0) Init */
    DIE_WHEN_ERRNO( close(STDIN_FILENO) );

#ifdef USE_LOGFILE
/* (0.1) Setup a logfile (since we can't log to console; must happen asap to ensure ALL logs get 'routed' to correct location) */
    FILE* stdout_logfile = (FILE*)DIE_WHEN_ERRNO_VPTR( freopen(LOGFILE_NAME, "a+", stdout) );
    FILE* stderr_logfile = (FILE*)DIE_WHEN_ERRNO_VPTR( freopen(LOGFILE_NAME, "a+", stderr) );
    if (0 != setvbuf(stdout_logfile, NULL, _IONBF, 0) ||
        0 != setvbuf(stderr_logfile, NULL, _IONBF, 0)) {
        LOG_ERROR_AND_EXIT("Couldn't set buffering options for logfile");
    }
#endif /* USE_LOGFILE */

/* (0.2) Parse CLI args */
    cli_args_t parsed_cli_args;
    parse_cli_args(argc, argv, &parsed_cli_args);
#ifdef DEV_DEBUG_ENABLE_LOGS
    print_parsed_cli_args(&parsed_cli_args);
#endif
    const int uxd_reg_sock_fd = parsed_cli_args.uxd_reg_sock_fd;

/* (0.3) Check whether fildes is valid  (TODO: check whether socket) */
    if (fcntl(uxd_reg_sock_fd, F_GETFL) < 0 && EBADF == errno) {
        LOG_ERROR_AND_EXIT("Invalid socket fildes");
    }


/* (0.4) Daemonize tracer, i.e., fork grandchild (which will be the actual tracer) */
    if (DIE_WHEN_ERRNO( fork() )) {     /* Child -> not used */
#ifdef USE_LOGFILE
        fclose(stdout_logfile);
        fclose(stderr_logfile);
#endif /* USE_LOGFILE */
        close(uxd_reg_sock_fd);
        pause();
        _exit(0);
    }
    kill(getppid(), SIGKILL);   /* Grandchild = Tracer */


/* (2) Setup */
//    unwind_init();


/* (3) Start tracing .. */
    const pid_t tracer_pid = getpid();
    DEV_DEBUG_PRINT_MSG("Ready for tracing requests (running under pid=%d) ..", tracer_pid);

    for (;;) {           // For testing only ..
        static int tracee_count = 0;

    /* (3.0) Check for new ipc requests */
        uxd_sock_ipc_requests_t ipc_request;
        if (-1 == uxd_ipc_receive_new_request(uxd_reg_sock_fd,
                                              &ipc_request, NULL)) {
            continue;
        }

        switch (ipc_request.request_type) {
            case PROBE_TRACER_RUNNING:
                DEV_DEBUG_PRINT_MSG("I'm still running (pid=%d)..", tracer_pid);
                break;

            case TRACEE_REQUEST_TRACING:
                DEV_DEBUG_PRINT_MSG("Received tracing request from tid=%d", ipc_request.payload.tracee_tid);
//                tracing_attach_tracee(ipc_request.payload.tracee_tid);
//                tracee_count++;
//                DEV_DEBUG_PRINT_MSG("Attached tracee w/ tid=%d", ipc_request.payload.tracee_tid);
                break;

            default:
                LOG_WARN("Invalid ipc-request");
        }
}

    /* (3.1) Check whether we can exit */
        if (0 == tracee_count) {
            DEV_DEBUG_PRINT_MSG("TIMEOUT: No tracees, will terminate in %d msec", EXIT_TIMEOUT_IN_MS);
            if (uxd_ipc_block_until_request_or_timeout(uxd_reg_sock_fd, EXIT_TIMEOUT_IN_MS)) {
                continue;
            }
            DEV_DEBUG_PRINT_MSG("TIMEOUT: Has lapsed, terminating ...");
            break;
        }


    /* (3.2) TODO: Trace */
        // ...
    }


/* (3) Cleanup */
    uxd_ipc_sock_fin(uxd_reg_sock_fd, UXD_SOCKET_FILEPATH);
#ifdef USE_LOGFILE
    fclose(stdout_logfile);
    fclose(stderr_logfile);
#endif /* USE_LOGFILE */

    return 0;
}
