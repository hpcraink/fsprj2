#include "entrypoint.h"
#include "../error.h"
#include "../wrapper_defines.h"
#include "../event.h"

#define DEV_DEBUG_ENABLE_LOGS
#include "../debug.h"

#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "stracer/tracer.h"




/* --- Function prototypes for helper functions --- */
static int __tracer_init_uxd_reg_socket(void);
void __tracee_send_tracing_request(void);



/* --- Functions --- */
void stracing_init_tracer(void) {
    DEV_DEBUG_PRINT_MSG("[PARENT:tid=%ld] $$$$ TODO  --  PROBLEM: Each fork'd process runs `init_process` => Tracer tries to register w/ itself ...", gettid());


/* (0) Establish tracer's UXD registration socket */
    int uxd_reg_sock_fd;
/* -> Tracer runs already */
    if (-1 == (uxd_reg_sock_fd = __tracer_init_uxd_reg_socket())) {
        return;
    }

/* -> Tracer didn't run yet */
/* (1) Launch tracer as grandchild */
/* Tracee = parent */
    if (DIE_WHEN_ERRNO( CALL_REAL_POSIX_SYNC(fork)() )) {
        CALL_REAL_POSIX_SYNC(close)(uxd_reg_sock_fd);
        return;
    }

/* Child -> launches stracer executable (which in turn forks again and becomes the tracer) */
    int buf_size = snprintf(NULL, 0, "%d", uxd_reg_sock_fd);
    char uxd_reg_sock_fd_str[buf_size +1];     // VLA
    snprintf(uxd_reg_sock_fd_str, sizeof uxd_reg_sock_fd_str, "%d", uxd_reg_sock_fd);

    CALL_REAL(execle)(TRACER_EXEC_FILENAME, TRACER_EXEC_FILENAME, uxd_reg_sock_fd_str, NULL, NULL);
    LIBIOTRACE_ERROR("exec failed %d", errno);
}

void stracing_register_with_tracer(void) {
    DEV_DEBUG_PRINT_MSG("[TRACEE:tid=%ld] Sending tracing request", gettid());

    // TODO: do_tracee

    __tracee_send_tracing_request();
}



/* -- Registration -- */
#define INIT_UXD_SOCKADDR_STRUCT(SA_STRUCT) do {   \
  memset(&SA_STRUCT, 0, sizeof(SA_STRUCT));        \
  SA_STRUCT.sun_family = AF_UNIX;                  \
  strcpy(SA_STRUCT.sun_path, UXD_SOCKET_FILEPATH); \
} while(0)

static int __tracer_init_uxd_reg_socket(void) {
    int uxd_reg_sock_fd;
    struct sockaddr_un sa;
    for (int i = 0; i < 2; i++) {  /* Max. 2 attempts */
        uxd_reg_sock_fd = DIE_WHEN_ERRNO( CALL_REAL_POSIX_SYNC(socket)(AF_UNIX, SOCK_STREAM, 0) );

        INIT_UXD_SOCKADDR_STRUCT(sa);
        // Bind failed  --> Try to cleanup & start over again
        if (-1 == CALL_REAL_POSIX_SYNC(bind)(uxd_reg_sock_fd, (struct sockaddr*)&sa, sizeof(struct sockaddr_un))) {
            INIT_UXD_SOCKADDR_STRUCT(sa);
            if (-1 == CALL_REAL_POSIX_SYNC(connect)(uxd_reg_sock_fd, (struct sockaddr*)&sa,
                                                    sizeof(struct sockaddr_un))) {   // TODO-ASK: IS "REUSING" `server_fd` OK  ??!
                DEV_DEBUG_PRINT_MSG("[PARENT:tid=%ld] No parallel instance is running, but socket still exists. Trying to clean up and start over again", gettid());
                CALL_REAL_POSIX_SYNC(close)(uxd_reg_sock_fd);
                DIE_WHEN_ERRNO( unlink(UXD_SOCKET_FILEPATH) );    // TODO #2: MAY PREVENT APPLICATION RUNNING LATER   ---> in libiotrace: .dtor section ..   --> tracee unlink if connect fails ..
            } else {
                CALL_REAL_POSIX_SYNC(close)(uxd_reg_sock_fd);
                DEV_DEBUG_PRINT_MSG("[PARENT:tid=%ld] A tracer instance is already running", gettid());
                return -1;
            }

        // Bind succeeded
        } else {
            break;
        }
    }

/* (3) Start listening for connections */
    DIE_WHEN_ERRNO( listen(uxd_reg_sock_fd, UXD_SOCKET_REQUESTS_BACKLOG_SIZE) );
    DEV_DEBUG_PRINT_MSG("[PARENT:tid=%ld] Inited UXD registration socket \"%s\"", gettid(), sa.sun_path);

    return uxd_reg_sock_fd;
}

void __tracee_send_tracing_request(void) {
/* (1) Create socket */
    int server_fd = DIE_WHEN_ERRNO( CALL_REAL_POSIX_SYNC(socket)(AF_UNIX, SOCK_STREAM, 0) );

/* (2) Connect to server using socket */
    struct sockaddr_un sa;
    INIT_UXD_SOCKADDR_STRUCT(sa);
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
