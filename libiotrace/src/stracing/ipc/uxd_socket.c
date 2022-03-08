#include <unistd.h>
#include <sys/socket.h>

#include "../../error.h"


/* -- Macros -- */
#define INIT_UXD_SOCKADDR_STRUCT(SA_STRUCT, UXD_SOCKET_FILEPATH) do { \
  memset(&SA_STRUCT, 0, sizeof(SA_STRUCT));                           \
  SA_STRUCT.sun_family = AF_UNIX;                                     \
  strcpy(SA_STRUCT.sun_path, UXD_SOCKET_FILEPATH);                    \
} while(0)


/* -- Functions -- */
int init_uxd_reg_socket(char* socket_filepath, int socket_backlog_size) {
    int uxd_reg_sock_fd;
    struct sockaddr_un sa;
    for (int i = 0; i < 2; i++) {  /* Max. 2 attempts */
        uxd_reg_sock_fd = DIE_WHEN_ERRNO( CALL_REAL_POSIX_SYNC(socket)(AF_UNIX, SOCK_STREAM, 0) );

        INIT_UXD_SOCKADDR_STRUCT(sa, socket_filepath);
        // Bind failed  --> Try to cleanup & start over again
        if (-1 == CALL_REAL_POSIX_SYNC(bind)(uxd_reg_sock_fd, (struct sockaddr*)&sa, sizeof(struct sockaddr_un))) {
            INIT_UXD_SOCKADDR_STRUCT(sa, socket_filepath);
            if (-1 == CALL_REAL_POSIX_SYNC(connect)(uxd_reg_sock_fd, (struct sockaddr*)&sa,
                                                    sizeof(struct sockaddr_un))) {   // TODO-ASK: IS "REUSING" `server_fd` OK  ??!
                CALL_REAL_POSIX_SYNC(close)(uxd_reg_sock_fd);
                DIE_WHEN_ERRNO( unlink(socket_filepath) );
            } else {
                CALL_REAL_POSIX_SYNC(close)(uxd_reg_sock_fd);
                return -1;
            }

            // Bind succeeded
        } else {
            break;
        }
    }

/* (3) Start listening for connections */
    DIE_WHEN_ERRNO( listen(uxd_reg_sock_fd, socket_backlog_size) );

    return uxd_reg_sock_fd;
}


void send_tracing_request(char* socket_filepath) {
/* (1) Create socket */
    int server_fd = DIE_WHEN_ERRNO( CALL_REAL_POSIX_SYNC(socket)(AF_UNIX, SOCK_STREAM, 0) );

/* (2) Connect to server using socket */
    struct sockaddr_un sa;
    INIT_UXD_SOCKADDR_STRUCT(sa, socket_filepath);
    DIE_WHEN_ERRNO( CALL_REAL_POSIX_SYNC(connect)(server_fd, (struct sockaddr*)&sa, sizeof(struct sockaddr_un)) );

    const char* const msg_to_send = "";     // NOTE: Some data must be sent, otherwise pid can't be retrieved
    const size_t msg_to_send_len_bytes = strlen(msg_to_send);
    size_t total_bytes_sent = 0, cur_bytes_sent;
    do {
        cur_bytes_sent = DIE_WHEN_ERRNO( CALL_REAL_POSIX_SYNC(write)(server_fd, msg_to_send + total_bytes_sent, msg_to_send_len_bytes - total_bytes_sent) );
        total_bytes_sent += cur_bytes_sent;
    } while (total_bytes_sent < msg_to_send_len_bytes);

    CALL_REAL_POSIX_SYNC(close)(server_fd);
}
