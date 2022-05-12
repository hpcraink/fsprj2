#ifndef STRACER_STRACING_FNRES_H_
#define STRACER_STRACING_FNRES_H_

#include <sys/types.h>
#include <stdbool.h>
#include "../../../common/stracer_types.h"


/* -- Function prototypes -- */
/**
 * @brief                                 Initializes this module
 *                                        NOTE: May terminate entire process if init fails
 *
 * @param[in] scerbmap_max_size           Max # of scerb-pointers which may be traced at any given point
 * @return void
 */
void stracing_fnres_init(long scerbmap_max_size);

/**
 * @brief                                 Finalizes this module
 *
 * @return void
 */
void stracing_fnres_cleanup(void);


/**
 * @brief                                 Attaches scerb, provided by tracee, and stores pointer to it for later usage
 *                                        NOTE: May terminate entire process if init fails
 *
 * @param[in] tid                         Tid of to be tracee
 * @return void
 */
void stracing_fnres_tracee_attach(pid_t tid);

/**
 * @brief                                 Detaches scerb of tracee corresponding to provided tid
 *                                        NOTE: May terminate entire process if init fails
 *
 * @param[in] tid                         Tid of to be tracee
 * @return void
 */
void stracing_fnres_tracee_detach(pid_t tid);

/**
 * @brief                                 Writes scevent into corresponding tracee's scerb
 *                                        NOTE: May terminate entire process if init fails
 *
 * @param[in] tid                         Tid of "affected" tracee
 * @param[in] scevent_buf_ptr             scevent which shall be written into tracee's scerb
 * @return void
 */
void stracing_fnres_tracee_add_scevent(pid_t tid, scevent_t* scevent_buf_ptr);

#endif /* STRACER_STRACING_FNRES_H_ */
