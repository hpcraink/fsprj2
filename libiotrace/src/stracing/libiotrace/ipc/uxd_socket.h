#ifndef LIBIOTRACE_STRACING_UXD_SOCKET_H
#define LIBIOTRACE_STRACING_UXD_SOCKET_H

/* -- Function prototypes -- */
/**
 * @brief                                 Creates Unix Domain Socket which will be used for IPC by stracer & tracee(s)
 *                                        The returned fd is then supposed to be passed to the `fork`(2)ed stracer
 *
 * @param[in] socket_filepath             Location where the socket file will be created on the fs
 *                                        NOTE: Doesn't work on all filesystems (e.g., VMWare shares won't work)
 * @param[in] socket_backlog_size         Max # of pending requests for the socket
 * @return int                            `-1` = socket exists and is already used (i.e., stracer runs),
 *                                        `>0` = fd (i.e., the stracer doesn't run yet)
 */
int uxd_ipc_parent_sock_init(char* socket_filepath, int socket_backlog_size);


/**
 * @brief                                 Sends tracing request to stracer via specified \p socket_filepath
 *
 * @param[in] socket_filepath             The location of the socket file (which is used for IPC w/ the stracer)
 * @param[out] server_fd_ptr              The fd opened for sending the request; used later for receiving acknowledgement
 */
void uxd_ipc_tracee_send_tracing_req(char* socket_filepath, int* server_fd_ptr);

/**
 * @brief                                 Blocks until the stracer acknowledges the tracing request
 *
 * @param[in] server_conn_fd              fd opened by tracee when tracing was requested;
 *                                        NOTE: MUST BE CLOSED AFTERWARDS BY CALLER
 */
void uxd_ipc_tracee_block_until_tracing_ack(int server_conn_fd);

#endif /* LIBIOTRACE_STRACING_UXD_SOCKET_H */
