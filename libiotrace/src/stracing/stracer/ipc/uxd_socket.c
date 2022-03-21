#include <unistd.h>
#include <sys/socket.h>

#include "uxd_socket.h"
#include "../../common/uxd_socket_types.h"

#include <assert.h>
#include <errno.h>
#include "../common/error.h"


/* -- Functions -- */
/* - Helpers - */
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
        LOG_WARN("Received incomplete or invalid ipc-request "
                 "(%zu of %zu expected bytes)", total_bytes_read, sizeof(*ipc_request));
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
pid_t check_for_new_tracees(int uxd_reg_sock_fd) {
    int conn_fd;
    if (-1 == (conn_fd = accept4(uxd_reg_sock_fd, NULL, NULL, SOCK_NONBLOCK))) {
        if (EWOULDBLOCK == errno) {
            return -1;
        }
        LOG_ERROR_AND_EXIT("[TRACER] `accept4` - checking for tracees");
    }

    uxd_sock_ipc_requests_t ipc_request; pid_t cr_pid;
    const int status = uxd_sock_read(conn_fd, &ipc_request, &cr_pid);
    close(conn_fd);

    return (0 == status && TRACEE_REQUEST_TRACING == ipc_request.request)
           ? cr_pid
           : -1;
}


void fin_uxd_reg_socket(int uxd_reg_sock_fd, char* socket_filepath) {
    close(uxd_reg_sock_fd);
    DIE_WHEN_ERRNO( unlink(socket_filepath) );
}
