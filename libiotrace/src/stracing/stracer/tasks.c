#include <stdio.h>
#include "tasks.h"
#include "trace/syscalls.h"
#include "trace/unwind.h"

#include "common/error.h"

/* -- Consts -- */
#define LIBIOTRACE_STATIC_WRAPPERS_PREFIX "__wrap_"


/*- -- Function prototypes -- */
void print_syscall(FILE *output,
                   pid_t tid, struct user_regs_struct *read_regs);


/* -- Functions -- */
void do_requested_tasks(cli_args_t *parsed_cli_args,
                        bool unwind_inited,
                        pid_t trapped_tracee_tid, struct user_regs_struct *read_regs) {

    const bool ioevent_was_traced = (unwind_inited) ? ( unwind_ioevent_was_traced(trapped_tracee_tid,
                                                        parsed_cli_args->unwind_module_name,
                                                        parsed_cli_args->unwind_static_linkage ? (LIBIOTRACE_STATIC_WRAPPERS_PREFIX) : (NULL)) )
                                                    : ( false );

/* -- TASK: WARN -- */
    if (parsed_cli_args->task_warn_not_traced_ioevents && !ioevent_was_traced) {
        LOG_WARN("An ioevent wasn't traced by libiotrace, indicated by the following syscall:");
        print_syscall(stderr, trapped_tracee_tid, read_regs);
    }


/* TODO: FNRES   ...  */
}



/* - Helpers - */
void print_syscall(FILE *output,
                   pid_t tid, struct user_regs_struct *read_regs) {
    const char* scall_name = syscalls_get_name(USER_REGS_STRUCT_SC_NO( (*read_regs) ));
    fprintf(output, "[%d] ", tid);
    fprintf(output, "%s(", (scall_name) ? (scall_name) : ("UNKNOWN"));
    syscalls_print_args(tid, read_regs);
    fprintf(output, ")");

    const long syscall_rtn_val = USER_REGS_STRUCT_SC_RTNVAL( (*read_regs) );
    fprintf(output, " = %ld\n", syscall_rtn_val);
}
