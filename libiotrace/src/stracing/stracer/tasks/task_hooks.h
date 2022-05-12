#ifndef STRACER_TASK_HOOKS_H
#define STRACER_TASK_HOOKS_H

#include "../cli.h"
#include "../trace/ptrace_utils.h"
#include "../../common/uxd_socket_types.h"
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
 * @brief                        Hook which gets called by stracer when a new tracee is attached
 *
 * @param[in] new_tracee_tid     Tid of new attached tracee
 * @param[in] new_tracee_ipc_request IPC request sent by new attached tracee
 */
void tasks_on_event_attached_tracee(pid_t new_tracee_tid, uxd_sock_ipc_msg_t *new_tracee_ipc_request);

/**
 * @brief                        Hook which gets called by stracer when a tracee has hit a breakpoint
 *
 * @param[in] trapped_tracee_tid Tid of trapped tracee
 * @param[in] read_regs_ptr          Register contents read by stracer
 */
void tasks_on_event_syscall(pid_t trapped_tracee_tid,
                            struct user_regs_struct *read_regs_ptr);

/**
 * @brief                        Hook which gets called by stracer when a tracee has terminated
 *
 * @param[in] trapped_tracee_tid Tid of terminated tracee
 */
void tasks_on_event_tracee_exit(pid_t trapped_tracee_tid);


#endif /* STRACER_TASK_HOOKS_H */
