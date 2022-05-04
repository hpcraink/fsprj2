#ifndef LIBIOTRACE_FNRES_SCERB_H
#define LIBIOTRACE_FNRES_SCERB_H

#include "../common/scerb.h"


/* -- Function prototypes -- */
/* - Init - */
/**
 * @brief                                 Creates & attaches smo, in which the ring buffer is inited
 *
 * @param[out] sm_scerb                   Handle (i.e., pointer) to shared memory segment containing scerb + buffer
 *                                        NOTE: Will be in most cases larger (due to kernel rounding to page size)
 * @param[in] smo_name                    Identifier of shared memory object containing buffer
 * @return int                            Returns `-1` on failure and `0` on success
 */
int fnres_scerb_create(sm_scerb_t** sm_scerb, char* smo_name);

/**
 * @brief                                 Detach a previously attached ringbuffer (doesn't destroy the buffer)
 *
 * @param[out] sm_scerb                   Handle (i.e., pointer) to shared memory segment containing scerb + buffer
 * @param[in] smo_name                    Identifier of shared memory object containing buffer
 * @return int                            Returns `-1` on failure and `0` on success
 */
int fnres_scerb_detach(sm_scerb_t** sm_scerb, char* smo_name);


/* - Operations - */
/**
 * @brief                                 Retrieves & removes the next unread element
 *
 * @param[in] sm_scerb                    Handle (i.e., pointer) to shared memory segment containing scerb + buffer
 * @param[out] event_ptr                  Pointer to output memory to copy buffer data to
 * @return int                            Returns `-2` if buffer empty, `-1` on all other failures and `0` on success
 */
int fnres_scerb_poll(sm_scerb_t* sm_scerb, scevent_t* event_ptr);

#endif /* LIBIOTRACE_FNRES_SCERB_H */
