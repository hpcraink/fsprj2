#ifndef STRACER_UXD_SOCKET_H
#define STRACER_UXD_SOCKET_H

#include <sys/types.h>
#include "../common/uxd_socket_types.h"

/* -- Function prototypes -- */
int receive_new_uxd_ipc_request(int uxd_reg_sock_fd,
                                uxd_sock_ipc_requests_t *ipc_req_ptr, pid_t *cr_pid_ptr);
void fin_uxd_reg_socket(int uxd_reg_sock_fd, char* socket_filepath);

#endif /* STRACER_UXD_SOCKET_H */
