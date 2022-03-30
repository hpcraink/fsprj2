#ifndef STRACER_UXD_SOCKET_H
#define STRACER_UXD_SOCKET_H

#include <sys/types.h>
#include "../common/uxd_socket_types.h"

/* -- Function prototypes -- */
/**
 * @brief                                 Checks non-blockingly for new IPC messages from tracees
 *
 * @param[in] uxd_reg_sock_fd             Fd of the Unix Domain Socket used for IPC
 * @param[out] tracee_conn_fd_ptr         The fd opened for accepting & checking the sent message;
 *                                        may be used to sending responses to tracee
 *                                        NOTE: MUST BE CLOSED BY CALLER AT SOME POINT
 * @param[out] ipc_req_ptr                The received IPC message
 * @param[out] cr_pid_ptr                 Optional, will contain the pid of the sender (i.e., the tracee)
 * @return int                            `-1` if no new requests, `-2` = invalid request, `0` = new request
 */
int uxd_ipc_tracer_recv_new_request(int uxd_reg_sock_fd, int* tracee_conn_fd_ptr,
                                    uxd_sock_ipc_msg_t *ipc_req_ptr, pid_t *cr_pid_ptr);

/**
 * @brief                                 Sends tracing request acknowledgement to tracee
 *
 * @param tracee_conn_fd                  Fd opened to read the request from tracee;
 *                                        NOTE: MAY BE CLOSED AFTERWARDS
 */
void uxd_ipc_tracer_send_tracing_ack(int tracee_conn_fd);

/**
 * @brief                                 Blocks until specified timeout has expired OR a new request has been sent
 *
 * @param[in] uxd_reg_sock_fd             Fd of the Unix Domain Socket used for IPC
 * @param[in] timeout_in_ms               Timeout in msec
 * @return int                            `0` = No new request, `1` = New request pending
 */
int uxd_ipc_tracer_block_until_request_or_timeout(int uxd_reg_sock_fd, int timeout_in_ms);

/**
 * @brief                                 Closes Unix Domain Socket used for IPC and performs cleanup
 *
 * @param[in] uxd_reg_sock_fd             Fd of the Unix Domain Socket used for IPC
 * @param[in] socket_filepath             Location where the socket file was created on the fs
 */
void uxd_ipc_tracer_sock_fin(int uxd_reg_sock_fd, char* socket_filepath);

#endif /* STRACER_UXD_SOCKET_H */
