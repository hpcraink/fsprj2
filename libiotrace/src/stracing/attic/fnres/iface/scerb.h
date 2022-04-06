/**
 * Syscall Event RingBuffer (aka., `scerb`) storing syscall events (aka., `scevent`) for the filename resolution (aka., `fnres`)
 */
#ifndef FNRES_SCERB_H_
#define FNRES_SCERB_H_

#include <stdio.h>


/* -- Data types -- */
// - Will only be instantiated iff ...
//   (a) successful syscall
//   (b) not monitored by libiotrace
//   (c) syscall of type: open / openat, close
// - Must NOT use pointers (since no shared vm)
//
//    struct timespec t;
//    clock_gettime(CLOCK_REALTIME, &t);
//    (u_int64_t)t.tv_sec * 1000000000ll + (u_int64_t)t.tv_nsec;
typedef struct {
    int fd;
    enum { OPEN, CLOSE } type;
    size_t filename_len;
    char filename[];     // Flexible Array Member; max = FILENAME_MAX
} fnres_scevent;


/* -- Macros -- */
#define FNRES_SCEVENT_SIZE_OF_INST(STRUCT_PTR) ( sizeof(*(STRUCT_PTR)) + (STRUCT_PTR)->filename_len )
#define FNRES_SCEVENT_MAX_SIZE                 ( sizeof(fnres_scevent) + FILENAME_MAX )


/* -- Function prototypes -- */
/* - Init - */
/**
 * @brief                                 Creates & attaches smo, in which the ring buffer is inited
 *
 * @param[in] min_buf_capacity_bytes      Required minimum capacity of ring buffer in bytes
 *                                        NOTE: Will be in most cases larger (due to kernel rounding to page size)
 * @param[in] smo_name                    Identifier of shared memory object containing buffer
 * @return int                            Returns `-1` on failure and `0` on success
 */
int fnres_scerb_create_and_attach(
        char* smo_name,
        unsigned int min_buf_capacity_bytes);

/**
 * @brief                                 Attach to a ringbuffer which has been already init'ed by a different process
 *
 * @param[in] smo_name                    Identifier of shared memory object containing buffer
 * @return int                            Returns `-1` on failure and `0` on success
 */
int fnres_scerb_attach(char* smo_name);

/**
 * @brief                                 Destroys ring buffer, detaches & destroys smo
 *
 * @param[in] smo_name                    Identifier of shared memory object containing buffer
 * @return int                            Returns `-1` on failure and `0` on success
 */
int fnres_scerb_destory_and_detach(char* smo_name);

/**
 * @brief                                 Detach a previously attached ringbuffer (doesn't destroy the buffer)
 *
 * @param[in] smo_name                    Identifier of shared memory object containing buffer
 * @return int                            Returns `-1` on failure and `0` on success
 */
int fnres_scerb_detach(char* smo_name);

/* - Operations - */
/**
 * @brief                                 Inserts item into ring buffer at next available slot
 *
 * @param[in] event_ptr                   Event to be added in next slot
 * @return int                            Returns `-2` if buffer full, `-1` on all other failures and `0` on success
 */
int fnres_scerb_offer(fnres_scevent* event_ptr);

/**
 * @brief                                 Retrieves & removes the next unread element
 *
 * @param[out] event_ptr                  Pointer to output memory to copy buffer data to
 * @return int                            Returns `-2` if buffer empty, `-1` on all other failures and `0` on success
 */
int fnres_scerb_poll(fnres_scevent* event_ptr);

/* - Misc. - */
/**
 * @brief                                 Allows checking whether init has been performed
 *
 * @return                                Returns `1` if inited, `0` otherwise
 */
int fnres_scerb_is_inited(void);

/**
 * @brief                                 Returns buffer usage (used / free space)
 *
 * @warning                               This function's results aren't accurate in multi-threaded environments,
 *                                        since bytes used in buffer is evaluated @ moment t and bytes free at t+1
 * @param[out] buf_used_bytes_ptr         Pointer to uint which will hold used buffer space (in bytes)
 * @param[out] buf_free_bytes_ptr         Pointer to uint which will hold free (hence, available) buffer space (in bytes)
 * @param[out] buf_capacity_bytes_ptr     Pointer to uint which will hold total capacity of buffer (in bytes)
 * @return int                            Returns `-1` on failure and `0` on success
 */
int fnres_scerb_get_buf_stats(unsigned int *buf_used_bytes_ptr,
                              unsigned int *buf_free_bytes_ptr,
                              unsigned int *buf_capacity_bytes_ptr);

/* - Debugging - */
void fnres_scerb_debug_print_status(FILE* output_stream);
void fnres_scerb_debug_print_scevent(fnres_scevent* event,
                                     FILE* output_stream);

#endif /* FNRES_SCERB_H_ */
