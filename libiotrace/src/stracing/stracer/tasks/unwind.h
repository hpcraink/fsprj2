/**
 * Used for execution stack unwinding
 */
#ifndef UNWIND_H
#define UNWIND_H

#include <unistd.h>
#include <stdbool.h>


/* -- Function prototypes -- */
void unwind_init(void);
void unwind_fin(void);

bool unwind_ioevent_was_traced(pid_t tid,
                               char* stacktrace_module_name, char* stacktrace_fct_name);


#endif /* UNWIND_H */