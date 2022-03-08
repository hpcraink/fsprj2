#include <assert.h>

#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <libgen.h>

#include "entrypoint.h"
#include "../error.h"
#include "../event.h"
#include "../wrapper_defines.h"
#include "../utils.h"

#define DEV_DEBUG_ENABLE_LOGS
#include "../debug.h"

#include "stracer/tracer.h"


/* --- Globals / Consts --- */
static const char* const STRACER_EXEC_FILENAME = "libiotrace_stracer";
#define SYSCALLS_TO_BE_TRACED "open"


/* --- Function prototypes for helper functions --- */
static int __tracer_init_uxd_reg_socket(void);
void __tracee_send_tracing_request(void);



/* --- Functions --- */
void stracing_init_tracer(char *ld_preload_env_val) {
    assert(ld_preload_env_val);                         // TODO: REVISE `assert`

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
  /* Prepare args for tracer's `argv` */
    /* Arg: executable filename */
    char *exec_arg_exec_fname;
{
    char* ld_preload_basedir;
    DIE_WHEN_ERRNO_VPTR( (ld_preload_basedir = strdup(ld_preload_env_val)) );           // TODO: SECURITY CONCERNS (LD_PRELOAD IS PASSED BY USER)
    DIE_WHEN_ERRNO( dirname_n(ld_preload_basedir, strlen(ld_preload_basedir) +1) );

    DIE_WHEN_ERRNO( asprintf(&exec_arg_exec_fname, "%s/%s", ld_preload_basedir, STRACER_EXEC_FILENAME) );
    CALL_REAL_ALLOC_SYNC(free)(ld_preload_basedir);
}

    /* Arg: Fildes of socket */
    char *exec_arg_sock_fd;
    DIE_WHEN_ERRNO( asprintf(&exec_arg_sock_fd, "-%c=%d", CLI_OPTION_SOCKFD, uxd_reg_sock_fd) );

    /* Arg: Syscall subset */
    char *exec_syscall_subset;
    DIE_WHEN_ERRNO( asprintf(&exec_syscall_subset, "-%c=%s", CLI_OPTION_SSUBSET, SYSCALLS_TO_BE_TRACED) );

    DEV_DEBUG_PRINT_MSG("[CHILD:tid=%ld] Launching stracer via \"%s %s %s\" ...", gettid(),
                        exec_arg_exec_fname, exec_arg_sock_fd, exec_syscall_subset);
    CALL_REAL(execle)(exec_arg_exec_fname,
                      exec_arg_exec_fname,  /* CLI args */
                      exec_arg_sock_fd,
                      exec_syscall_subset,
                      NULL,
                      NULL);                    /* Envs (make sure NO `LD_PRELOAD` is passed) */
    LIBIOTRACE_ERROR("`exec` failed (errno=%d)", errno);
}

void stracing_register_with_tracer(void) {
    DEV_DEBUG_PRINT_MSG("[TRACEE:tid=%ld] Sending tracing request", gettid());

    // TODO: do_tracee

    __tracee_send_tracing_request();
}



/* -- Registration -- */
/* - !!!  WARNING REGARDING SOCKET: The socket cannot be created on every fs (e.g., VMWare shares - 'dialout' in `ls -lah`)  !!! - */
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
                DIE_WHEN_ERRNO( unlink(UXD_SOCKET_FILEPATH) );
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
    DIE_WHEN_ERRNO( listen(uxd_reg_sock_fd, UXD_REG_SOCKET__BACKLOG_SIZE) );
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
