#include <unistd.h>
#include <sys/socket.h>

#include "../../common/uxd_socket_types.h"

#include <assert.h>
#include <fcntl.h>
#include "../../../common/error.h"


/* -- Macros -- */
#define INIT_UXD_SOCKADDR_STRUCT(SA_STRUCT, UXD_SOCKET_FILEPATH) do { \
    memset(&SA_STRUCT, 0, sizeof(SA_STRUCT));                         \
    SA_STRUCT.sun_family = AF_UNIX;                                   \
    strcpy(SA_STRUCT.sun_path, UXD_SOCKET_FILEPATH);                  \
} while(0)


/* -- Functions -- */
/* - Internal - */
static int uxd_sock_connect(char* socket_filepath) {
/* 1. Create socket */
    int server_fd = DIE_WHEN_ERRNO( CALL_REAL_POSIX_SYNC(socket)(AF_UNIX, SOCK_STREAM, 0) );

/* 2. Connect to server using socket */
    struct sockaddr_un sa;
    INIT_UXD_SOCKADDR_STRUCT(sa, socket_filepath);
    DIE_WHEN_ERRNO( CALL_REAL_POSIX_SYNC(connect)(server_fd, (struct sockaddr*)&sa, sizeof(struct sockaddr_un)) );

    return server_fd;
}

static void uxd_sock_write(int uxd_reg_sock_fd,
                           uxd_sock_ipc_msg_t* ipc_request) {
    const size_t msg_to_send_len_bytes = sizeof(*ipc_request);
    size_t total_bytes_sent = 0,
           cur_bytes_sent;
    do {
        cur_bytes_sent = DIE_WHEN_ERRNO( CALL_REAL_POSIX_SYNC(write)(uxd_reg_sock_fd, ipc_request + total_bytes_sent, msg_to_send_len_bytes - total_bytes_sent) );
        total_bytes_sent += cur_bytes_sent;
    } while (cur_bytes_sent > 0 && total_bytes_sent < msg_to_send_len_bytes);

    assert( sizeof(*ipc_request) == total_bytes_sent && "Wrote incomplete ipc_request" );
}

static int uxd_sock_read(int conn_fd,
                         uxd_sock_ipc_msg_t *ipc_request) {
    assert( ipc_request && 0 <= conn_fd && "`ipc_request` and `conn_fd` may be valid" );

    memset(ipc_request, 0, sizeof(*ipc_request));

    size_t total_bytes_read = 0,
            cur_bytes_read;
    do {
        cur_bytes_read = DIE_WHEN_ERRNO(CALL_REAL_POSIX_SYNC(read)(conn_fd, ipc_request + total_bytes_read, sizeof(*ipc_request) - total_bytes_read) );
        total_bytes_read += cur_bytes_read;
    } while (cur_bytes_read > 0 && total_bytes_read < sizeof(*ipc_request));

    if (sizeof(*ipc_request) != total_bytes_read) {
        LOG_WARN("Received incomplete ipc-request "
                 "(%zu of %zu expected bytes) -- %s", total_bytes_read, sizeof(*ipc_request), strerror(errno));
        return -1;
    }

    return 0;
}


/* - Public - */
int uxd_ipc_parent_sock_init(char* socket_filepath,
                             int socket_backlog_size) {
    int uxd_reg_sock_fd;
    for (int i = 0; i < 2; i++) {  /* Max. 2 attempts */
        uxd_reg_sock_fd = DIE_WHEN_ERRNO( CALL_REAL_POSIX_SYNC(socket)(AF_UNIX, SOCK_STREAM, 0) );

        struct sockaddr_un sa;
        INIT_UXD_SOCKADDR_STRUCT(sa, socket_filepath);
    /* Bind failed  --> Check whether already running (via `connect`) -> if not: Try to cleanup & start over again */
        if (-1 == CALL_REAL_POSIX_SYNC(bind)(uxd_reg_sock_fd, (struct sockaddr*)&sa, sizeof(struct sockaddr_un))) {
            INIT_UXD_SOCKADDR_STRUCT(sa, socket_filepath);
        /* -> Not running  (but there are still leftovers) */
            if (-1 == CALL_REAL_POSIX_SYNC(connect)(uxd_reg_sock_fd, (struct sockaddr*)&sa,
                                                    sizeof(struct sockaddr_un))) {
                CALL_REAL_POSIX_SYNC(close)(uxd_reg_sock_fd);
                DIE_WHEN_ERRNO( unlink(socket_filepath) );
        /* -> Running */
            } else {
                uxd_sock_ipc_msg_t ipc_request = { .msg_type = PARENT_PROBE_TRACER_RUNNING };  // Inform tracer this was "just a probe"
                uxd_sock_write(uxd_reg_sock_fd, &ipc_request);

                CALL_REAL_POSIX_SYNC(close)(uxd_reg_sock_fd);
                return -1;
            }

    /* Bind succeeded */
        } else {
            DIE_WHEN_ERRNO( CALL_REAL_POSIX_SYNC(fcntl)(uxd_reg_sock_fd, F_SETFL, O_NONBLOCK) );
            break;
        }
    }

/* 2. Start listening for connections */
    DIE_WHEN_ERRNO( listen(uxd_reg_sock_fd, socket_backlog_size) );

    return uxd_reg_sock_fd;
}


void uxd_ipc_tracee_send_tracing_req(char* socket_filepath, int* server_fd_ptr) {
    *server_fd_ptr = uxd_sock_connect(socket_filepath);

    uxd_sock_ipc_msg_t ipc_request = {
            .msg_type = PARENT_REQUEST_TRACING,
            .msg_payload.tracee_tid = gettid()
    };
    uxd_sock_write(*server_fd_ptr, &ipc_request);
}

void uxd_ipc_tracee_block_until_tracing_ack(int server_conn_fd) {
    uxd_sock_ipc_msg_t ipc_request;
    const int rv = uxd_sock_read(server_conn_fd, &ipc_request);

    if (!rv && TRACER_REQUEST_ACCEPTED == ipc_request.msg_type) { return; }
    LOG_ERROR_AND_DIE("Got invalid tracing request acknowledgement");
}
