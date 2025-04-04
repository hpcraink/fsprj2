/**
 * Producer API for Syscall Event RingBuffer (aka., `scerb`), which stores syscall events (aka., `scevent`) for the filename resolution (aka., `fnres`)
 */
#ifndef STRACER_LSEP_SCERB_PRODUCER_H_
#define STRACER_LSEP_SCERB_PRODUCER_H_

#include "../../../../common/stracer_types.h"
#include "../../../../common/tasks/lsep/scerb/scerb_types.h"


/* -- Function prototypes -- */
/* - Init - */
/**
 * @brief                                 Attaches to a ringbuffer which has been already init'ed by a different process
 *
 * @param[out] sm_scerb                   Handle (i.e., pointer) to shared memory segment containing scerb + buffer
 * @param[in] smo_name                    Identifier of shared memory object containing buffer
 * @return int                            Returns `-1` on failure and `0` on success
 */
int scerb_attach(sm_scerb_t** sm_scerb, char* smo_name);

/**
 * @brief                                 Destroys ring buffer, detaches & destroys smo
 *
 * @param[out] sm_scerb                   Handle (i.e., pointer) to shared memory segment containing scerb + buffer
 * @param[in] smo_name                    Identifier of shared memory object containing buffer
 * @return int                            Returns `-1` on failure and `0` on success
 */
int scerb_destory_detach(sm_scerb_t** sm_scerb, char* smo_name);


/* - Operations - */
/**
 * @brief                                 Inserts item into ring buffer at next available slot
 *
 * @param[in] sm_scerb                    Handle (i.e., pointer) to shared memory segment containing scerb + buffer
 * @param[in] scevent_buf_ptr               Event to be added in next slot
 * @return int                            Returns `-2` if buffer is full, `-1` on all other failures and `0` on success
 */
int scerb_offer(sm_scerb_t* sm_scerb, scevent_t* scevent_buf_ptr);

#endif /* STRACER_LSEP_SCERB_PRODUCER_H_ */
