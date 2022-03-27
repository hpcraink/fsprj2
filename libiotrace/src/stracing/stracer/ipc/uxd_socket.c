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
    if (-1 == (conn_fd = accept(uxd_reg_sock_fd, NULL, NULL))) {
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
        cur_bytes_read = DIE_WHEN_ERRNO( read(conn_fd, ipc_request + total_bytes_read, sizeof(*ipc_request) - total_bytes_read) );
        total_bytes_read += cur_bytes_read;
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
int uxd_ipc_receive_new_request(int uxd_reg_sock_fd,
                                uxd_sock_ipc_requests_t *ipc_req_ptr, pid_t *cr_pid_ptr) {
/* Accept request from backlog */
    int conn_fd;
    if (-1 == (conn_fd = uxd_sock_accept(uxd_reg_sock_fd))) {
        return -1;      // `-1` = No new request
    }

/* Read request */
    const int read_status = uxd_sock_read(conn_fd, ipc_req_ptr, cr_pid_ptr);
    DIE_WHEN_ERRNO( close(conn_fd) );

    return read_status;
}


void uxd_ipc_sock_fin(int uxd_reg_sock_fd, char* socket_filepath) {
    DIE_WHEN_ERRNO( close(uxd_reg_sock_fd) );
    DIE_WHEN_ERRNO( unlink(socket_filepath) );
}

