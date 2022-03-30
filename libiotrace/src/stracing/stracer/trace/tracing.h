#ifndef STRACER_TRACING_H_
#define STRACER_TRACING_H_

/* -- Function prototypes -- */
/**
 * @brief                                 Attaches process \p tid and sets first breakpoint
 *
 * @param[in] tid                         Process which shall be attached
 * @return int                            `-1` = attach failed; `>0` = pid of attached process
 */
pid_t tracing_attach_tracee(pid_t tid);

/**
 * @brief                                 Checks for processes which have been trapped, giving
 *                                        the caller the chance to
 *                                        a) check the trap and
 *                                        b) set the next breakpoint (aka. bp) for the process
 *
 * @param[in] next_bp_tid                 Process for which the next bp shall be set
 * @return int                            `0` = no pending traps;
 *                                        `>0` = pid of STOPPED process; `<0` = pid of TERMINATED process
 */
int tracing_set_next_bp_and_check_trap(pid_t next_bp_tid);

#endif /* STRACER_TRACING_H_ */
