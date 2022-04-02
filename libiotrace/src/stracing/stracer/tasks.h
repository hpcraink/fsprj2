#ifndef LIBIOTRACE_TASKS_H
#define LIBIOTRACE_TASKS_H

#include <sys/types.h>
#include "cli.h"
#include "trace/ptrace_utils.h"


/* -- Function prototypes -- */
void do_requested_tasks(cli_args_t *parsed_cli_args,
                        bool unwind_inited,
                        pid_t trapped_tracee_tid, struct user_regs_struct *read_regs);

#endif /* LIBIOTRACE_TASKS_H */
