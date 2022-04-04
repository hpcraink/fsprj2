#ifndef STRACER_TASK_HOOKS_H
#define STRACER_TASK_HOOKS_H

#include "../cli.h"
#include "../trace/ptrace_utils.h"
#include <sys/types.h>


/* -- Function prototypes -- */
/**
 * @brief                        Hook which gets called by stracer during init, before tracing
 *                               Used to perform init's for different tracing tasks
 *
 * @param[in] cli_args_ptr       CLI args, containing selected tasks (which shall be inited)
 *                               Will be used throughout the runtime of stracer (must therefore
 *                               be properly allocated)
 */
void tasks_on_stracer_init(cli_args_t* cli_args_ptr);

/**
 * @brief                        Hook which gets called by stracer during cleanup
 */
void tasks_on_stracer_fin(void);

/**
 * @brief                        Hook which gets called by stracer when a tracee has hit a breakpoint
 *
 * @param[in] trapped_tracee_tid Tid of trapped tracee
 * @param[in] user_regs_struct   Register contents read by stracer
 */
void tasks_on_event_syscall(pid_t trapped_tracee_tid,
                            struct user_regs_struct *read_regs);

/**
 * @brief                        Hook which gets called by stracer when a tracee has terminated
 *
 * @param[in] trapped_tracee_tid Tid of terminated tracee
 */
void tasks_on_event_tracee_exit(pid_t trapped_tracee_tid);


#endif /* STRACER_TASK_HOOKS_H */
