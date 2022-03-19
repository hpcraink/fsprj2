#ifndef LIBIOTRACE_STRACER_CONSTS_H
#define LIBIOTRACE_STRACER_CONSTS_H

/* -- Constants / Globals -- */
#define STRACER_EXEC_FILENAME "libiotrace_stracer"

/* - !!!  WARNING REGARDING SOCKET: The socket CANNOT be created on every fs (e.g., VMWare shares - 'dialout' in `ls -lah`)  !!! - */
#define UXD_SOCKET_FILEPATH "libiotrace-tracer.sock"
#define UXD_REG_SOCKET_BACKLOG_SIZE 5000

/* - CLI flags - */
#define STRACER_CLI_OPTION_SOCKFD  's'
#define STRACER_CLI_OPTION_SSUBSET 'e'
#define STRACER_CLI_OPTION_WARN    'w'

#endif /* LIBIOTRACE_STRACER_CONSTS_H */
