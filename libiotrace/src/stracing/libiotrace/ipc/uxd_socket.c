#include <unistd.h>
#include <sys/socket.h>

#include "../../common/uxd_socket_types.h"

#include <assert.h>
#include <fcntl.h>
#include "../../../error.h"


/* -- Macros -- */
#define INIT_UXD_SOCKADDR_STRUCT(SA_STRUCT, UXD_SOCKET_FILEPATH) do { \
  memset(&SA_STRUCT, 0, sizeof(SA_STRUCT));                           \
  SA_STRUCT.sun_family = AF_UNIX;                                     \
  strcpy(SA_STRUCT.sun_path, UXD_SOCKET_FILEPATH);                    \
} while(0)


/* -- Functions -- */
/* - Helpers - */
static int uxd_sock_connect(char* socket_filepath) {
/* (1) Create socket */
    int server_fd = DIE_WHEN_ERRNO( CALL_REAL_POSIX_SYNC(socket)(AF_UNIX, SOCK_STREAM, 0) );

/* (2) Connect to server using socket */
    struct sockaddr_un sa;
    INIT_UXD_SOCKADDR_STRUCT(sa, socket_filepath);
    DIE_WHEN_ERRNO( CALL_REAL_POSIX_SYNC(connect)(server_fd, (struct sockaddr*)&sa, sizeof(struct sockaddr_un)) );

    return server_fd;
}

static void uxd_sock_write(int uxd_reg_sock_fd,
                           uxd_sock_ipc_requests_t* ipc_request) {
    const size_t msg_to_send_len_bytes = sizeof(*ipc_request);
    size_t total_bytes_sent = 0,
           cur_bytes_sent;
    do {
        cur_bytes_sent = DIE_WHEN_ERRNO( CALL_REAL_POSIX_SYNC(write)(uxd_reg_sock_fd, ipc_request + total_bytes_sent, msg_to_send_len_bytes - total_bytes_sent) );
        total_bytes_sent += cur_bytes_sent;
    } while (cur_bytes_sent > 0 && total_bytes_sent < msg_to_send_len_bytes);

    assert( sizeof(*ipc_request) == total_bytes_sent && "Wrote incomplete ipc_request" );
}


/* - Public - */
int uxd_ipc_sock_init(char* socket_filepath,
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
                uxd_sock_ipc_requests_t ipc_request = { .request_type = PROBE_TRACER_RUNNING };  // Inform tracer this was "just a probe"
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

/* (3) Start listening for connections */
    DIE_WHEN_ERRNO( listen(uxd_reg_sock_fd, socket_backlog_size) );

    return uxd_reg_sock_fd;
}


void uxd_ipc_send_tracing_request(char* socket_filepath) {
    int server_fd = uxd_sock_connect(socket_filepath);

    uxd_sock_ipc_requests_t ipc_request = {
            .request_type = TRACEE_REQUEST_TRACING,
            .payload.tracee_tid = gettid()
    };
    uxd_sock_write(server_fd, &ipc_request);

    CALL_REAL_POSIX_SYNC(close)(server_fd);
}
