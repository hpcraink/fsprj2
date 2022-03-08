#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

#include "stracer.h"
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
    DEV_DEBUG_PRINT_MSG("[TRACER:pid=%d] Ready for tracing requests ..", getpid());


/* (2) Start tracing .. */
    // TODO do_tracer, containing following fct ..
    for (int i = 0; i < 3; i++) {           // For testing only ..
        pid_t new_tracee;
        if (-1 != (new_tracee = check_for_new_tracees(uxd_reg_sock_fd))) {
            DEV_DEBUG_PRINT_MSG("[TRACER] Received tracing request from pid=%d", new_tracee);
        }

        sleep(1);
    }


/* (3) Cleanup on exit .. */
    // TODO: Register at_exit -> cleanup
    fin_uxd_reg_socket(uxd_reg_sock_fd, UXD_SOCKET_FILEPATH);

    return 0;
}
