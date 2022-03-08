/**
 * CLI args parsing
 */
#ifndef STRACER_CLI_H_
#define STRACER_CLI_H_

#include <stdbool.h>
#include <sys/types.h>

#include "trace/generated/syscallents.h"


/* -- Types -- */
typedef struct {
    int uxd_reg_sock_fd;

    bool trace_only_syscall_subset;
    bool syscall_subset_to_be_traced[SYSCALLS_ARR_SIZE];

    bool warn_not_traced_syscalls;
} cli_args;


/* -- Function prototypes -- */
void parse_cli_args(int argc, char** argv,
                    cli_args* parsed_cli_args_ptr);
void print_parsed_cli_args(cli_args*);

#endif /* STRACER_CLI_H_ */
