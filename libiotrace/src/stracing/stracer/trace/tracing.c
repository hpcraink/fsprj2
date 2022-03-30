#include <sys/ptrace.h>
#include <sys/wait.h>

#include "tracing.h"
#include "../common/error.h"
#define DEV_DEBUG_ENABLE_LOGS
#include "common/debug.h"


/* -- Consts -- */
#define PTRACE_TRAP_INDICATOR_BIT (1 << 7)


/* -- Functions -- */
pid_t tracing_attach_tracee(pid_t tid) {
    if (-1 == ptrace(PTRACE_ATTACH, tid) ) {
        if (ESRCH == errno) {
            LOG_WARN("`PTRACE_ATTACH` on tid=%d -- %s", tid, strerror(errno));
            return -1;              // In case process died shortly after sending request
        }
        LOG_ERROR_AND_EXIT("`PTRACE_ATTACH` on tid=%d -- %s", tid, strerror(errno));
    }

    int tracee_status;
    do {
        if ( -1 == waitpid(tid, &tracee_status, 0) ) {
            if (ECHILD == errno || ESRCH == errno) {
                LOG_WARN("`waitpid` on tid=%d -- %s", tid, strerror(errno));
                return -1;          // In case process died shortly after sending request
            }
            LOG_ERROR_AND_EXIT("`waitpid` on tid=%d -- %s", tid, strerror(errno));
        }
    } while (!WIFSTOPPED(tracee_status));

    DIE_WHEN_ERRNO( ptrace(PTRACE_SETOPTIONS,
                           tid,
                           0,
                           PTRACE_O_TRACESYSGOOD) );

    return tid;
}


int tracing_set_bp_and_check_trap(pid_t next_bp_tid) {  /* NOTEs: 'bp' = breakpoint; Reports only 'trap events' which are due to termination or stops caused by syscall's */

    for (int pending_signal = 0; ; ) {
    /* (0) Restart stopped tracee but set next breakpoint (on next syscall)   (AND "forward" received signal to tracee) */
        if (-1 != next_bp_tid) {        /* Check only (i.e., don't set breakpoint) */
            DIE_WHEN_ERRNO( ptrace(PTRACE_SYSCALL, next_bp_tid, 0, pending_signal) );
            DEV_DEBUG_PRINT_MSG(">>> Tracing: Set next bp for %d", next_bp_tid);
        }

        /* Reset signal (after it has been delivered) */
        pending_signal = 0;


    /* (1) Check for ANY tracee to change state (stops or terminates) */
        int trapped_tracee_status;
        const pid_t trapped_tracee_tid = DIE_WHEN_ERRNO( waitpid(-1, &trapped_tracee_status, WNOHANG) );
        if (0 == trapped_tracee_tid) {
            return 0;                           /* >>>   No traps */
        }


    /* (2) Check tracee's process status */
        /* (2.1) Possibility 1: Tracee was stopped */
        if (WIFSTOPPED(trapped_tracee_status)) {
            siginfo_t si;

            next_bp_tid = trapped_tracee_tid;
            const int stopsig = WSTOPSIG(trapped_tracee_status);

            /* (I) SYSCALL-ENTER-/-EXIT-stop */
            if ((SIGTRAP | PTRACE_TRAP_INDICATOR_BIT) == stopsig) {
                return trapped_tracee_tid;      /* >>>   Tracee was stopped (indicated by positive returned tid; only possible stop reason here: due to syscall breakpoint) */

            /* (II) `PTRACE_EVENT_xxx` stops */
            } else if (SIGTRAP == stopsig) {
                // ... Check for ptrace-events here ...

            /* (III) Group-stops */
            } else if (ptrace(PTRACE_GETSIGINFO, trapped_tracee_tid, 0, &si) < 0) {
                // ...

            /* (IV) Signal-delivery stops */
            } else {
                DEV_DEBUG_PRINT_MSG(">>> Tracing: +++ [%d] received (not delivered yet) signal \"%s\" +++", next_bp_tid, strsignal(stopsig));
                pending_signal = stopsig;
            }


        /* (2.2) Possibility 2: Tracee terminated */
        } else {
            return -(trapped_tracee_tid);       /* >>>   Tracee has terminated (indicated by negative returned tid; possible stop reasons: see above) */
        }
    }
}
