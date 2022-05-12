/**
 * Implements the interface using the library 'rmind-ringbuf'
 *   Source:        https://github.com/rmind/ringbuf
 *   API reference: " "
 */
#include <assert.h>
#include <stdio.h>
#include <stddef.h>

#include <sys/mman.h>

#include "../../../../common/tasks/fnres/scerb/scerb_ipc_utils.h"
#include "scerb_consumer.h"
#include "../../../../common/tasks/fnres/scerb/scerb_types_rmind.h"

//#define DEV_DEBUG_ENABLE_LOGS
#include "../../../../../common/debug.h"
#include "../../../../../common/error.h"


/* -- Functions -- */
/* - Init - */
int scerb_create_attach(sm_scerb_t** sm_scerb, char* smo_name) {
    assert( sm_scerb && smo_name && "params may not be `NULL`" );
    // TODO: CHECK INIT'ed

    const unsigned long sm_min_size = sizeof(sm_scerb_t);    /* NOTE: Shared memory segment is atm FIXed in size; KEEP IN MIND: OS may allocate more (multiple of page size -> thus 'min') */

/* Cleanup old stuff    (NOTE: Don't check for errors since smo may not exist) */
    shm_unlink(smo_name);

/* Create new shared mem block + map it into caller's address space */
    unsigned long long sm_len;
    DIE_WHEN_ERRNO( sm_ipc_attach_create_smo(smo_name, sm_min_size, (void**)sm_scerb, &sm_len, true) );

    DEV_DEBUG_PRINT_MSG("Created smo \"%s\" w/ requested `min_buf_capacity_bytes`=%lu (got from OS=%llu)", smo_name,
              sm_min_size, sm_len);

/* Init ringbuffer */
    return (0 == ringbuf_setup(&(*sm_scerb)->ringbuf, STRACING_FNRES_RB_SIZE)) ? 0 : -1;
}


int scerb_detach(sm_scerb_t** sm_scerb, char* smo_name) {
    assert( sm_scerb && smo_name && "params may not be `NULL`" );
    // TODO: CHECK INIT'ed

    DIE_WHEN_ERRNO( sm_ipc_detach_smo((void**)sm_scerb, smo_name) );

    return (NULL == *sm_scerb) ? (0) : (-1);
}


int scerb_poll(sm_scerb_t* sm_scerb, scevent_t* scevent_buf_ptr) {
    assert( sm_scerb && scevent_buf_ptr && "params may not be `NULL`" );
    // TODO: CHECK INIT'ed


    ringbuf_t* const sm_rb_ptr = &sm_scerb->ringbuf;
    uint8_t* const sm_buf_ptr = sm_scerb->buf;


    size_t cur_consumable_bytes,
           cur_read_offset;
    if (0 == (cur_consumable_bytes = ringbuf_consume(sm_rb_ptr, &cur_read_offset))) {
        DEV_DEBUG_PRINT_MSG("Read failed  --  Buffer was at t-1 empty");
        return -2;
    }

/* Read = 2 step process  -> 1st step (until struct member field `filename_len`, so we know # bytes we must read)  -> then 2nd step */
#define POLL_LEN_OFFSET_STRUCT offsetof(scevent_t, filename)
    memcpy(scevent_buf_ptr, &sm_buf_ptr[cur_read_offset], POLL_LEN_OFFSET_STRUCT);
    assert(cur_consumable_bytes >= (scevent_buf_ptr->filename_len + POLL_LEN_OFFSET_STRUCT) );
    memcpy(scevent_buf_ptr->filename, &sm_buf_ptr[cur_read_offset + POLL_LEN_OFFSET_STRUCT], scevent_buf_ptr->filename_len);

    ringbuf_release(sm_rb_ptr, SCEVENT_SIZEOF(scevent_buf_ptr));

    return 0;
}
