/**
 * Libiotrace Syscall Event Passing (lsep)
 *   Allows libiotrace to receive syscall events, which have been traced by the stracer
 *   by storing them in a ringbuffer, which is allocated in a shared memory segment
 */
#ifndef LIBIOTRACE_STRACING_LSEP_H_
#define LIBIOTRACE_STRACING_LSEP_H_


/* -- Forward type declarations -- */
struct basic;           /* Note: #include "../libiotrace_include_struct.h" causes "Declaration of 'struct basic' will not be visible outside of this function" when incl. in "wrapper_defines.h" */


/* -- Function prototypes -- */
/**
 * @brief                                 Creates scerb for current thread
 *
 * @return void
 */
void stracing_lsep_setup(void);

/**
 * @brief                                 Detaches scerb for current thread
 *
 * @return void
 */
void stracing_lsep_cleanup(void);

/**
 * @brief                                 Checks scerb for new scevent's and processes them (e.g., by adding them to the fnmap)
 *
 * @return void
 */
void stracing_lsep_process_new_scevents(void);


/**
 * @brief                                 Alias given stream (by looking it up using derived fd) in fnmap
 *                                        Necessary since syscalls use only fd's
 * @param[in] ioevent_ptr                 To be aliased ioevent, which is of type stream
 *
 * @return int                            Returns `0` if given stream was aliased, otherwise `-1` on error or `-2`
 */
int stracing_fnres_lookup_and_alias_stream(struct basic* ioevent_ptr);

#endif /* LIBIOTRACE_STRACING_LSEP_H_ */
