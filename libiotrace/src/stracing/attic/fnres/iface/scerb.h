/**
 * Syscall Event RingBuffer (aka., `scerb`) storing syscall events (aka., `scevent`) for the filename resolution (aka., `fnres`)
 */
#ifndef FNRES_SCERB_H_
#define FNRES_SCERB_H_

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>


/* -- Data types -- */
typedef struct sm_scerb sm_scerb_t;

typedef struct {
    uint64_t ts_in_ns;
    int fd;
    enum { OPEN, CLOSE } type;
    size_t filename_len;
    char filename[];     // Flexible Array Member; max = FILENAME_MAX
} scevent_t;


/* -- Macros -- */
#define FNRES_SCEVENT_SIZE_OF_INST(STRUCT_PTR) ( sizeof(*(STRUCT_PTR)) + (STRUCT_PTR)->filename_len )
#define FNRES_SCEVENT_MAX_SIZE                 ( sizeof(fnres_scevent) + FILENAME_MAX )


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
int fnres_scerb_create(
        sm_scerb_t** sm_scerb,
        char* smo_name);

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
 * @brief                                 Inserts item into ring buffer at next available slot
 *
 * @param[out] sm_scerb                   Handle (i.e., pointer) to shared memory segment containing scerb + buffer
 * @param[in] event_ptr                   Event to be added in next slot
 * @return int                            Returns `-2` if buffer full, `-1` on all other failures and `0` on success
 */
int fnres_scerb_offer(sm_scerb_t* sm_scerb, scevent_t* event_ptr);

/**
 * @brief                                 Retrieves & removes the next unread element
 *
 * @param[out] sm_scerb                   Handle (i.e., pointer) to shared memory segment containing scerb + buffer
 * @param[out] event_ptr                  Pointer to output memory to copy buffer data to
 * @return int                            Returns `-2` if buffer empty, `-1` on all other failures and `0` on success
 */
int fnres_scerb_poll(sm_scerb_t* sm_scerb, scevent_t* event_ptr);

/* - Misc. - */
/**
 * @brief                                 Allows checking whether init has been performed
 *
 * @param[out] sm_scerb                   Handle (i.e., pointer) to shared memory segment containing scerb + buffer
 * @return                                Returns `1` if inited, `0` otherwise
 */
bool fnres_scerb_is_inited(sm_scerb_t* sm_scerb);



/* -- Debugging -- */
void fnres_scerb_debug_print_scevent(scevent_t*, FILE*);

#endif /* FNRES_SCERB_H_ */
