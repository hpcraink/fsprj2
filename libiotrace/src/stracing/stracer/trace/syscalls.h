/**
 * Functions which make use of `syscallents` type
 *   May also make use of "ptrace_utils"
 */
#ifndef TRACE_SYSCALLS_H
#define TRACE_SYSCALLS_H

#include <unistd.h>
#include "../../common/stracer_types.h"


/* -- Type declarations -- */
struct user_regs_struct_full;


/* -- Function prototypes -- */
const char *syscalls_get_name(long syscall_nr);
long syscalls_get_nr(char* syscall_name);

void syscalls_print_args(pid_t tid, struct user_regs_struct_full *regs);

int syscall_to_scevent(pid_t tid, struct user_regs_struct *read_regs_ptr, scevent_t* event_buf_ptr);


#endif /* TRACE_SYSCALLS_H */
