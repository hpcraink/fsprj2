#ifndef LIBIOTRACE_STRACING_UXD_SOCKET_H
#define LIBIOTRACE_STRACING_UXD_SOCKET_H

/* -- Function prototypes for helper functions -- */
int init_uxd_ipc_socket(char* socket_filepath, int socket_backlog_size);
void send_tracing_uxd_ipc_request(char* socket_filepath) ;

#endif /* LIBIOTRACE_STRACING_UXD_SOCKET_H */
