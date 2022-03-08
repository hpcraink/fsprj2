#ifndef STRACER_UXD_SOCKET_H
#define STRACER_UXD_SOCKET_H

#include <sys/types.h>

/* -- Function prototypes -- */
pid_t check_for_new_tracees(int uxd_reg_sock_fd);
void fin_uxd_reg_socket(int uxd_reg_sock_fd, char* socket_filepath);

#endif /* STRACER_UXD_SOCKET_H */