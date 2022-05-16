#include <assert.h>
#include <stdio.h>

#include "task_hooks.h"
#include "../../common/stracer_types.h"
#include "../trace/syscalls.h"
#include "aux/unwind.h"
#include "lsep/stracing_lsep.h"

#include "../common/utils.h"

//#define DEV_DEBUG_ENABLE_LOGS
#include "../common/error.h"


/* -- Globals -- */
static cli_args_t* g_cli_args_ptr;


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
    if (g_cli_args_ptr->task_warn_not_traced_ioevents ||            // All tasks which require stack unwinding
        g_cli_args_ptr->task_lsep) {
        unwind_init();
    }

/* FNRES task */
    if (g_cli_args_ptr->task_lsep) {
        stracing_lsep_init(STRACING_FNRES_SCERBMAP_SIZE);
    }
}


void tasks_on_stracer_fin(void) {
    assert( g_cli_args_ptr && "Not inited yet" );

    DEV_DEBUG_PRINT_MSG(">>> Tasks: Fin");

/* Unwinding functionality */
    if (unwind_is_inited()) {
        unwind_fin();
    }

    if (g_cli_args_ptr->task_lsep) {
        stracing_lsep_cleanup();
    }

    g_cli_args_ptr = NULL;
}


void tasks_on_event_attached_tracee(pid_t new_tracee_tid, __attribute__((unused))uxd_sock_ipc_msg_t *new_tracee_ipc_request) {
    assert( g_cli_args_ptr && "Not inited yet" );

    if (g_cli_args_ptr->task_lsep) {
        stracing_lsep_tracee_attach(new_tracee_tid);
    }
}


void tasks_on_event_syscall(pid_t trapped_tracee_tid,
                            struct user_regs_struct *read_regs_ptr) {
    assert( g_cli_args_ptr && "Not inited yet" );

#define LIBIOTRACE_STATIC_WRAPPERS_PREFIX "__wrap_"
    const bool ioevent_was_traced = (unwind_is_inited()) ? ( unwind_ioevent_was_traced(trapped_tracee_tid,
                                                                                    g_cli_args_ptr->unwind_module_name,
                                                                                    g_cli_args_ptr->unwind_static_linkage ? (LIBIOTRACE_STATIC_WRAPPERS_PREFIX) : (NULL)) ) :
                                                           ( false );

    if (!ioevent_was_traced) {
/* -- TASK: WARN -- */
        if (g_cli_args_ptr->task_warn_not_traced_ioevents) {
            LOG_WARN("An ioevent wasn't traced by libiotrace, indicated by the following syscall (time=%lu):", gettime());
            print_syscall(stderr, trapped_tracee_tid, read_regs_ptr);
        }

/* -- TASK: LSEP -- */
        if (g_cli_args_ptr->task_lsep) {
            scevent_t *scevent_buf_ptr = (scevent_t*)alloca(SCEVENT_MAX_SIZE);
            if (0 != syscall_to_scevent(trapped_tracee_tid, read_regs_ptr, scevent_buf_ptr)) {
                LOG_ERROR_AND_EXIT("Couldn't 'translate' syscall to `scevent`");
            }

            stracing_lsep_tracee_add_scevent(trapped_tracee_tid, scevent_buf_ptr);
        }
    }
}


void tasks_on_event_tracee_exit(pid_t trapped_tracee_tid) {
    assert( g_cli_args_ptr && "Not inited yet" );

    if (g_cli_args_ptr->task_lsep) {
        stracing_lsep_tracee_detach(trapped_tracee_tid);
    }
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
