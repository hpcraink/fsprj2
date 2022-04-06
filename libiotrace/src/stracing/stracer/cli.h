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

    bool unwind_static_linkage;
    char* unwind_module_name;

    bool task_warn_not_traced_ioevents;
} cli_args_t;


/* -- Function prototypes -- */
void parse_cli_args(int argc, char** argv,
                    cli_args_t* parsed_cli_args_ptr);
void print_parsed_cli_args(cli_args_t*);

#endif /* STRACER_CLI_H_ */
