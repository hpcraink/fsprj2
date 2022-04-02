#include <unistd.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <stdint.h>

#include "uxd_socket.h"
#include "../common/utils.h"

#include <assert.h>
#include <errno.h>
#include "../common/error.h"
//#define DEV_DEBUG_ENABLE_LOGS
#include "../common/debug.h"


/* -- Functions -- */
/* - Internal - */
static int uxd_sock_accept(int uxd_reg_sock_fd) {
    int conn_fd;
    if (-1 == (conn_fd = accept(uxd_reg_sock_fd, NULL, NULL))) {
        if (EAGAIN == errno || EWOULDBLOCK == errno) {
            return -1;      /* No new connections in backlog ... */
        }
        LOG_ERROR_AND_EXIT("`accept` -- %s", strerror(errno));
    }

    return conn_fd;
}

static void uxd_sock_write(int uxd_reg_sock_fd,
                           uxd_sock_ipc_msg_t* ipc_request) {
    const size_t msg_to_send_len_bytes = sizeof(*ipc_request);
    size_t total_bytes_sent = 0,
            cur_bytes_sent;
    do {
        cur_bytes_sent = DIE_WHEN_ERRNO( write(uxd_reg_sock_fd, ipc_request + total_bytes_sent, msg_to_send_len_bytes - total_bytes_sent) );
        total_bytes_sent += cur_bytes_sent;
    } while (cur_bytes_sent > 0 && total_bytes_sent < msg_to_send_len_bytes);

    assert( sizeof(*ipc_request) == total_bytes_sent && "Wrote incomplete ipc_request" );
}

static int uxd_sock_read(int conn_fd,
                         uxd_sock_ipc_msg_t *ipc_request,
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
int uxd_ipc_tracer_recv_new_request(int uxd_reg_sock_fd, int* tracee_conn_fd_ptr,
                                    uxd_sock_ipc_msg_t *ipc_req_ptr, pid_t *cr_pid_ptr) {
/* Accept request from backlog */
    if (-1 == (*tracee_conn_fd_ptr = uxd_sock_accept(uxd_reg_sock_fd))) {
        return -1;                      /* `-1` = No new request(s) */
    }

/* Read request */
    const int rv = uxd_sock_read(*tracee_conn_fd_ptr, ipc_req_ptr, cr_pid_ptr);

    return (rv == -1) ? (-2) : (0);     /* `-2` = incomplete (thus, invalid) request, `0` = valid request */
}

void uxd_ipc_tracer_send_tracing_ack(int tracee_conn_fd) {
    uxd_sock_write(
            tracee_conn_fd,
            (uxd_sock_ipc_msg_t[]){{ .msg_type = TRACER_REQUEST_ACCEPTED }});
}

int uxd_ipc_tracer_block_until_request_or_timeout(int uxd_reg_sock_fd,
                                                  int timeout_in_ms) {
    struct pollfd fd;
    memset(&fd, 0, sizeof(fd));
    fd.events = POLLIN;
    fd.fd = uxd_reg_sock_fd;


    for (int remaining_time_in_ms = timeout_in_ms; ; ) {
        const uint64_t start_time_in_ns = gettime();
        const int rv = poll(&fd, 1, remaining_time_in_ms);
        const uint64_t end_time_in_ns = gettime();

        if (-1 == rv) {
            if (EINTR == errno) {       /* Interrupted by signal (relevant when signal handlers have been registered) */
                remaining_time_in_ms -= (int)(end_time_in_ns - start_time_in_ns) / 1000000;
                DEV_DEBUG_PRINT_MSG("`remaining_time_in_ms`=%d ms: Got interrupted up by signal, "
                                    "sleeping again ...", remaining_time_in_ms);
                continue;
            }
            LOG_ERROR_AND_EXIT("`poll` -- %s", strerror(errno));
        }

        return ((rv > 0) && (fd.revents & POLLIN));
    }
}

void uxd_ipc_tracer_sock_fin(int uxd_reg_sock_fd, char* socket_filepath) {
    DIE_WHEN_ERRNO( close(uxd_reg_sock_fd) );
    DIE_WHEN_ERRNO( unlink(socket_filepath) );
}
