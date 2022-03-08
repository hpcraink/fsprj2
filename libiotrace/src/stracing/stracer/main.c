#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <signal.h>

#include "cli.h"
#include "tracer.h"
#include "common/error.h"
#define DEV_DEBUG_ENABLE_LOGS
#include "common/debug.h"


/* --- Function prototypes for helper functions --- */
static pid_t __tracer_check_for_new_tracees(int uxd_reg_sock_fd);
static void __tracer_fin_uxd_reg_socket(int uxd_reg_sock_fd);



int main(int argc, char** argv) {
/* (0) Parse CLI args */
    cli_args parsed_cli_args;
    parse_cli_args(argc, argv, &parsed_cli_args);

    DEV_DEBUG_PRINT_MSG("[TRACER] Got:\n`uxd_reg_sock_fd`=%d", parsed_cli_args.uxd_reg_sock_fd);
    const int uxd_reg_sock_fd = parsed_cli_args.uxd_reg_sock_fd;


/* TODO: CHECK WHETHER VALID FILDES ... */


/* (1) Fork grandchild (which will be the actual tracer) */
/* Child -> not used */
    if (DIE_WHEN_ERRNO( fork() )) {
        close(uxd_reg_sock_fd);
        pause();
        _exit(0);
    }

/* Grandchild = Tracer */
    kill(getppid(), SIGKILL);
    DEV_DEBUG_PRINT_MSG("[TRACER:pid=%d] Ready for tracing requests ..", getpid());



/* (2) Start tracing .. */
    // TODO do_tracer, containing following fct ..
    for (int i = 0; i < 3; i++) {           // For testing only ..
        sleep(1);
        __tracer_check_for_new_tracees(uxd_reg_sock_fd);
    }



    // TODO: Register at_exit -> cleanup
    __tracer_fin_uxd_reg_socket(uxd_reg_sock_fd);

    return 0;
}



/* --- Functions --- */
/* -- Registration -- */
static pid_t __tracer_check_for_new_tracees(int uxd_reg_sock_fd) {
    int sockfd;
    if (-1 == (sockfd = accept4(uxd_reg_sock_fd, NULL, NULL, SOCK_NONBLOCK))) {
        if (EWOULDBLOCK == errno) {
            return -1;
        }
        LOG_ERROR_AND_EXIT("[TRACER] `accept4` - checking for tracees");
    }

    struct ucred cr; socklen_t cr_len = sizeof (cr);
    DIE_WHEN_ERRNO( getsockopt(sockfd, SOL_SOCKET, SO_PEERCRED, &cr, &cr_len) );

    close(sockfd);


    DEV_DEBUG_PRINT_MSG("[TRACER] Received tracing request from pid=%d", (pid_t)cr.pid);
    return (pid_t)cr.pid;
}

static void __tracer_fin_uxd_reg_socket(int uxd_reg_sock_fd) {
    close(uxd_reg_sock_fd);
    DIE_WHEN_ERRNO( unlink(UXD_SOCKET_FILEPATH) );    // TODO #2: MAY PREVENT APPLICATION RUNNING LATER   ---> in libiotrace: .dtor section ..   --> tracee unlink if connect fails ..
}
