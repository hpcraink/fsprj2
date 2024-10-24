/**
 * "Entrypoint" (for stracing in libiotrace):
 *   - Tasks:
 *     - Launch the stracer
 *     - Register tracee(s) (i.e., the process traced by libiotrace) w/ the running stracer
 */
#ifndef LIBIOTRACE_ENTRYPOINT_H
#define LIBIOTRACE_ENTRYPOINT_H


/* -- Function prototypes -- */
/**
 * @brief                        Creates all necessary IPC facilities, forks, and
 *                               launches the stracer (if not running yet)
 *                               May be called during process startup by `init_process`
 */
void stracing_init_stracer(void);


/**
 * @brief                        Cleans up scerb for tracee
 *                               May be called by `cleanup_thread`
 */
void stracing_tracee_fin(void);


/**
 * @brief                        Sends tracing request to stracer
 *                               Requires the stracer to be already running
 *                               May be called by `init_thread`
 */
void stracing_tracee_register_with_stracer(void);

#endif /* LIBIOTRACE_ENTRYPOINT_H */
