#include <unistd.h>
#include <sys/socket.h>

#include "uxd_socket.h"

#include <assert.h>
#include <errno.h>
#include "../common/error.h"


/* -- Functions -- */
/* - Helpers - */
static int uxd_sock_accept(int uxd_reg_sock_fd) {
    int conn_fd;
    if (-1 == (conn_fd = accept4(uxd_reg_sock_fd, NULL, NULL, SOCK_NONBLOCK))) {
        if (EAGAIN == errno || EWOULDBLOCK == errno) {
            return -1;      // No new connections in backlog ...
        }
        LOG_ERROR_AND_EXIT("`accept4` -- %s", strerror(errno));
    }

    return conn_fd;
}

static int uxd_sock_read(int conn_fd,
                          uxd_sock_ipc_requests_t *ipc_request,
                          pid_t *cr_pid) {
    assert( ipc_request && 0 <= conn_fd && "`ipc_request` and `conn_fd` may be valid" );

    memset(ipc_request, 0, sizeof(*ipc_request));

    size_t total_bytes_read = 0,
           cur_bytes_read;
    do {
        const ssize_t status = read(conn_fd, ipc_request + total_bytes_read, sizeof(*ipc_request) - total_bytes_read);

        if (0 <= status) {
            cur_bytes_read = status;
            total_bytes_read += cur_bytes_read;

        } else if (-1 == status) {
            if (EAGAIN == errno || EWOULDBLOCK == errno) {              // TODO: REVISE (hard spinning as long as requested hasn't been sent completely)
                continue;       // Spin .. since we don't block (see `SOCK_NONBLOCK` flag in `accept4`)
            }
            LOG_ERROR_AND_EXIT("`read` -- %s", strerror(errno));
        }
    } while (cur_bytes_read > 0 && total_bytes_read < sizeof(*ipc_request));

    if (sizeof(*ipc_request) != total_bytes_read) {
        LOG_WARN("Received incomplete ipc-request "
                 "(%zu of %zu expected bytes) -- %s", total_bytes_read, sizeof(*ipc_request), strerror(errno));
        return -1;
    }

    if (cr_pid) {
        struct ucred cr; socklen_t cr_len = sizeof (cr);
        DIE_WHEN_ERRNO( getsockopt(conn_fd, SOL_SOCKET, SO_PEERCRED, &cr, &cr_len) );
        *cr_pid = (pid_t)cr.pid;
    }

    return 0;
}


/* - Public - */
int receive_new_uxd_ipc_events(int uxd_reg_sock_fd,
                               uxd_sock_ipc_requests_t *ipc_req_ptr, pid_t *cr_pid_ptr) {
/* Accept request from backlog */
    int conn_fd;
    if (-1 == (conn_fd = uxd_sock_accept(uxd_reg_sock_fd))) {
        return -1;
    }

/* Read request (if any) */
    const int read_status = uxd_sock_read(conn_fd, ipc_req_ptr, cr_pid_ptr);
    close(conn_fd);

    return read_status;
}


void fin_uxd_reg_socket(int uxd_reg_sock_fd, char* socket_filepath) {
    close(uxd_reg_sock_fd);
    DIE_WHEN_ERRNO( unlink(socket_filepath) );
}
