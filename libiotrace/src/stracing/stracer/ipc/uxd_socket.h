#ifndef STRACER_UXD_SOCKET_H
#define STRACER_UXD_SOCKET_H

#include <sys/types.h>
#include "../common/uxd_socket_types.h"

/* -- Function prototypes -- */
int uxd_ipc_receive_new_request(int uxd_reg_sock_fd,
                                uxd_sock_ipc_requests_t *ipc_req_ptr, pid_t *cr_pid_ptr);
void uxd_ipc_sock_fin(int uxd_reg_sock_fd, char* socket_filepath);
int uxd_ipc_block_until_request_or_timeout(int uxd_reg_sock_fd, int timeout_in_msec);

#endif /* STRACER_UXD_SOCKET_H */
