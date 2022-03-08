#include <unistd.h>
#include <sys/socket.h>

#include "uxd_socket.h"

#include <errno.h>
#include "../common/error.h"


/* -- Functions -- */
pid_t check_for_new_tracees(int uxd_reg_sock_fd) {
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

    return (pid_t)cr.pid;
}


void fin_uxd_reg_socket(int uxd_reg_sock_fd, char* socket_filepath) {
    close(uxd_reg_sock_fd);
    DIE_WHEN_ERRNO( unlink(socket_filepath) );
}
