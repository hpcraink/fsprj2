#ifndef STRACER_TRACING_H_
#define STRACER_TRACING_H_

#include "../cli.h"


/* -- Function prototypes -- */
pid_t tracing_attach_tracee(pid_t tid);
int tracing_set_bp_and_check_trap(pid_t next_bp_tid);



#endif /* STRACER_TRACING_H_ */
