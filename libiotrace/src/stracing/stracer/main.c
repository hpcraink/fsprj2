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


# include <time.h>


// TODO do_tracer, containing following fct ..
/*
TODO: Issues:
        ...
 */


#define TESTING_DISABLE_LOGFILE   // TODO: RMV LATER ...


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

        uxd_sock_ipc_requests_t ipc_request;
        if (-1 == receive_new_uxd_ipc_request(uxd_reg_sock_fd,
                                              &ipc_request, NULL)) {
            nanosleep((const struct timespec[]){{0, 250000000L}}, NULL);        // TESTING (reduce spinning) ...
            continue;
        }

        switch (ipc_request.request_type) {
            case PROBE_TRACER_RUNNING:
                DEV_DEBUG_PRINT_MSG("I'm still running (pid=%d)..", tracer_pid);
                break;

            case TRACEE_REQUEST_TRACING:
                DEV_DEBUG_PRINT_MSG("Received tracing request from tid=%d", ipc_request.payload.tracee_tid);
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
