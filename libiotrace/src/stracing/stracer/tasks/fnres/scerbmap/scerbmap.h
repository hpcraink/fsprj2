/**
 * Scerb map (scerb_map)
 *   Implements abstraction (API) for map implementation (which is used for tracking pointers to scerb's)
 */
#ifndef STRACER_FNRES_STRACING_SCERBMAP_H_
#define STRACER_FNRES_STRACING_SCERBMAP_H_

#include <sys/types.h>
#include <stdbool.h>
#include "../../../../common/tasks/fnres/scerb/scerb_types.h"



/* -- Function prototypes -- */
/**
 * @brief                                 Returns whether map has been already initialized
 *
 * @return bool                          `true` = already inited, otherwise `false`
 */
bool scerbmap_is_inited(void);

/**
 * @brief                                 Creates the map w/ the specified capacity
 *                                        NOTE: Will terminate entire process on failure
 *
 * @param[in] max_size                    Max # of entries which may be stored
 * @return void
 */
void scerbmap_create(long max_size);

/**
 * @brief                                 Destroys map
 *
 * @return void
 */
void scerbmap_destroy(void);

/**
 * @brief                                 Searches, given a tid, for a scerb pointer
 *
 * @param[in] tid_ptr                     Pointer to tid of tracee
 * @param[out] found_sm_scerb             Will contain (if found) the pointer to the found scerb
 *
 * @return int                            `0` when found under specified key
 */
int scerbmap_get(pid_t* tid_ptr, sm_scerb_t** found_sm_scerb);

/**
 * @brief                                 Adds a new scerb under the given tid
 *                                        NOTE: Will terminate entire process on failure
 *
 * @param[in] tid_ptr                     Pointer to tid of tracee
 * @param[in] sm_scerb                    Pointer to scerb which shall be stored under specified tid
 *
 * @return void
 */
void scerbmap_add(pid_t* tid_ptr, sm_scerb_t* sm_scerb);

/**
 * @brief                                 Removes scerb entry of specified tid
 *                                        NOTE: Will terminate entire process on failure
 *
 * @param[in] tid_ptr                     Pointer to tid of tracee
 *
 * @return void
 */
void scerbmap_remove(pid_t* tid_ptr);

#endif /* STRACER_FNRES_STRACING_SCERBMAP_H_ */
