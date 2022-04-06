#include <assert.h>
#include <stdio.h>

#include "task_hooks.h"
#include "../trace/syscalls.h"
#include "unwind.h"

#include "../common/utils.h"

#include "../common/error.h"
//#define DEV_DEBUG_ENABLE_LOGS
#include "../common/debug.h"


/* -- Globals -- */
static cli_args_t* g_cli_args_ptr;

static bool g_unwind_inited;


/*- -- Function prototypes -- */
static void print_syscall(FILE *output,
                          pid_t tid, struct user_regs_struct *read_regs);


/* -- Functions -- */
void tasks_on_stracer_init(cli_args_t* cli_args_ptr) {
    assert( cli_args_ptr && "`cli_args_ptr` may not be NULL" );
    assert( !g_cli_args_ptr && "Got already init" );

    g_cli_args_ptr = cli_args_ptr;
    DEV_DEBUG_PRINT_MSG(">>> Tasks: Init (selected tasks -- warn=%d)", g_cli_args_ptr->task_warn_not_traced_ioevents);

/* Unwinding functionality */
    if (g_cli_args_ptr->task_warn_not_traced_ioevents) {    // All tasks which require stack unwinding
        unwind_init();
        g_unwind_inited = true;
    }

/* ... */
}


void tasks_on_stracer_fin(void) {
    assert( g_cli_args_ptr && "Not inited yet" );

    DEV_DEBUG_PRINT_MSG(">>> Tasks: Fin");

/* Unwinding functionality */
    if (g_unwind_inited) {
        unwind_fin();
        g_unwind_inited = false;
    }

/* ... */
    g_cli_args_ptr = NULL;
}


void tasks_on_event_attached_tracee(__attribute__((unused))pid_t new_tracee_tid) {
//    assert( g_cli_args_ptr && "Not inited yet" );

/* ... */
}


void tasks_on_event_syscall(pid_t trapped_tracee_tid,
                            struct user_regs_struct *read_regs) {
    assert( g_cli_args_ptr && "Not inited yet" );

#define LIBIOTRACE_STATIC_WRAPPERS_PREFIX "__wrap_"
    const bool ioevent_was_traced = (g_unwind_inited) ? ( unwind_ioevent_was_traced(trapped_tracee_tid,
                                                                                    g_cli_args_ptr->unwind_module_name,
                                                                                    g_cli_args_ptr->unwind_static_linkage ? (LIBIOTRACE_STATIC_WRAPPERS_PREFIX) : (NULL)) ) :
                                                        ( false );

/* -- TASK: WARN -- */
    if (g_cli_args_ptr->task_warn_not_traced_ioevents && !ioevent_was_traced) {
        LOG_WARN("An ioevent wasn't traced by libiotrace, indicated by the following syscall (time=%lu):", gettime());
        print_syscall(stderr, trapped_tracee_tid, read_regs);
    }


/* -- TODO: TASK: FNRES ... -- */
}


void tasks_on_event_tracee_exit(__attribute__((unused))pid_t trapped_tracee_tid) {
//    assert( g_cli_args_ptr && "Not inited yet" );

/* ... */
}


/* - Helpers - */
static void print_syscall(FILE *output,
                   pid_t tid, struct user_regs_struct *read_regs) {
    const char* scall_name = syscalls_get_name(USER_REGS_STRUCT_SC_NO( (*read_regs) ));
    fprintf(output, "[%d] ", tid);
    fprintf(output, "%s(", (scall_name) ? (scall_name) : ("UNKNOWN"));
    syscalls_print_args(tid, read_regs);
    fprintf(output, ")");

    const long syscall_rtn_val = USER_REGS_STRUCT_SC_RTNVAL( (*read_regs) );
    fprintf(output, " = %ld\n", syscall_rtn_val);
}
