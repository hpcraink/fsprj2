#ifndef LIBIOTRACE_STRACING_UXD_SOCKET_H
#define LIBIOTRACE_STRACING_UXD_SOCKET_H

/* -- Function prototypes for helper functions -- */
int uxd_ipc_sock_init(char* socket_filepath, int socket_backlog_size);
void uxd_ipc_send_tracing_request(char* socket_filepath) ;

#endif /* LIBIOTRACE_STRACING_UXD_SOCKET_H */
