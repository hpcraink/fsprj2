#ifndef STRACER_FNRES_SCERB_H_
#define STRACER_FNRES_SCERB_H_

#include "../common/scerb.h"


/* -- Function prototypes -- */
/* - Init - */
/**
 * @brief                                 Attach to a ringbuffer which has been already init'ed by a different process
 *
 * @param[out] sm_scerb                   Handle (i.e., pointer) to shared memory segment containing scerb + buffer
 * @param[in] smo_name                    Identifier of shared memory object containing buffer
 * @return int                            Returns `-1` on failure and `0` on success
 */
int fnres_scerb_attach_register_producer(sm_scerb_t** sm_scerb, char* smo_name);

/**
 * @brief                                 Destroys ring buffer, detaches & destroys smo
 *
 * @param[out] sm_scerb                   Handle (i.e., pointer) to shared memory segment containing scerb + buffer
 * @param[in] smo_name                    Identifier of shared memory object containing buffer
 * @return int                            Returns `-1` on failure and `0` on success
 */
int fnres_scerb_destory_and_detach(sm_scerb_t** sm_scerb, char* smo_name);


/* - Operations - */
/**
 * @brief                                 Inserts item into ring buffer at next available slot
 *
 * @param[in] sm_scerb                    Handle (i.e., pointer) to shared memory segment containing scerb + buffer
 * @param[in] event_ptr                   Event to be added in next slot
 * @return int                            Returns `-2` if buffer full, `-1` on all other failures and `0` on success
 */
int fnres_scerb_offer(sm_scerb_t* sm_scerb, scevent_t* event_ptr);

#endif /* STRACER_FNRES_SCERB_H_ */
