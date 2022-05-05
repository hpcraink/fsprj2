/**
 * Implements the interface using the library 'rmind-ringbuf'
 *   Source:        https://github.com/rmind/ringbuf
 *   API reference: " "
 */
#include <assert.h>
#include <stdio.h>
#include <stddef.h>

#include <sys/mman.h>

#include "../common/sm_ipc_utils.h"
#include "scerb_consumer.h"
#include "../common/scerb_rmind.h"

#include "../../../stracer/common/error.h"


/* -- Functions -- */
/* - Init - */
int fnres_scerb_create(sm_scerb_t** sm_scerb, char* smo_name) {
    assert( sm_scerb && smo_name && "params may not be `NULL`" );
    // TODO: CHECK INIT'ed

    const unsigned long sm_min_size = sizeof(sm_scerb_t);    /* NOTE: Buffer-size is currently fixed; may be in reality larger (multiple of page size) */

/* Cleanup old stuff    (NOTE: Don't check for errors since smo may not exist) */
    shm_unlink(smo_name);

/* Create new shared mem block + map it into caller's address space */
    unsigned long long sm_len;
    DIE_WHEN_ERRNO( sm_ipc_attach_create_map_smo(smo_name, sm_min_size, (void**)sm_scerb, &sm_len, 1) );

    LOG_DEBUG("Created smo \"%s\" w/ requested `min_buf_capacity_bytes`=%lu (got from OS=%llu)", smo_name,
              sm_min_size, sm_len);

/* Init ringbuffer */
    return ringbuf_setup(&(*sm_scerb)->ringbuf, 1, STRACE_FNRES_RB_SIZE);
}


int fnres_scerb_detach(sm_scerb_t** sm_scerb, char* smo_name) {
    assert( sm_scerb && smo_name && "params may not be `NULL`"  );
    // TODO: CHECK INIT'ed

    DIE_WHEN_ERRNO( sm_ipc_detach_smo((void**)sm_scerb, smo_name) );

    return NULL == *sm_scerb;
}


int fnres_scerb_poll(sm_scerb_t* sm_scerb, scevent_t* event_ptr) {
    assert( sm_scerb && event_ptr && "params may not be `NULL`"  );
    // TODO: CHECK INIT'ed


    ringbuf_t* const sm_rb_ptr = &sm_scerb->ringbuf;
    uint8_t* const sm_buf_ptr = sm_scerb->buf;


    size_t rb_cur_len,
           rb_cur_off;
    if (0 == (rb_cur_len = ringbuf_consume(sm_rb_ptr, &rb_cur_off))) {
        LOG_DEBUG("Read failed  --  Buffer was at t-1 empty");
        return -2;
    }

/* Read  -> 1st part (until struct member field `filename_len`, so we know # bytes we must read)  -> then 2nd part */
#define POLL_LEN_OFFSET_STRUCT offsetof(scevent_t, filename)
    memcpy(event_ptr, &sm_buf_ptr[rb_cur_off], POLL_LEN_OFFSET_STRUCT);
    assert( rb_cur_len >= (event_ptr->filename_len + POLL_LEN_OFFSET_STRUCT) );
    memcpy(event_ptr->filename, &sm_buf_ptr[rb_cur_off +POLL_LEN_OFFSET_STRUCT], event_ptr->filename_len);

    ringbuf_release(sm_rb_ptr, FNRES_SCEVENT_SIZE_OF_INST(event_ptr));

    return 0;
}
