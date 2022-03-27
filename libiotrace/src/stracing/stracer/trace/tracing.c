#include <sys/ptrace.h>
#include <sys/wait.h>

#include "tracing.h"
#include "../common/error.h"


/* -- Functions -- */
void tracing_attach_tracee(pid_t tid) {
    DIE_WHEN_ERRNO( ptrace(PTRACE_ATTACH, tid) );

    int tracee_status;
    do {
        DIE_WHEN_ERRNO( waitpid(tid, &tracee_status, 0) );
    } while (!WIFSTOPPED(tracee_status));

    DIE_WHEN_ERRNO( ptrace(PTRACE_SETOPTIONS,
                           tid,
                           0,
                           PTRACE_O_TRACESYSGOOD) );
}
