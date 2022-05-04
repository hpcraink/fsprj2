/**
 * Implements the interface using the library 'rmind-ringbuf'
 *   Source:        https://github.com/rmind/ringbuf
 *   API reference: " "
 */
#include <assert.h>
#include <stdio.h>
#include <stddef.h>

#include "../common/sm_ipc_utils.h"
#include "../common/scerb.h"
#include "../common/scerb_rmind.h"

#include "../../../../stracer/common/error.h"


/* -- Functions -- */
/* - Init - */
int fnres_scerb_attach_register_producer(sm_scerb_t** sm_scerb, char* smo_name) {
    assert( sm_scerb && smo_name && "params may not be `NULL`"  );
    // TODO: CHECK INIT'ed

/* Get shared memory block + map it into caller's address space */
    unsigned long long shared_mem_len;
    DIE_WHEN_ERRNO( sm_ipc_attach_create_map_smo(smo_name, 0,(void**)sm_scerb, &shared_mem_len, 0) );

/* Register producer (!!!  THERE CAN ONLY BE 1  !!!) */
    (void)ringbuf_register(&(*sm_scerb)->ringbuf, 0);

    return 0;
}


int fnres_scerb_destory_and_detach(sm_scerb_t** sm_scerb, char* smo_name) {
    assert( sm_scerb && smo_name && "params may not be `NULL`"  );
    // TODO: CHECK INIT'ed

    DIE_WHEN_ERRNO( sm_ipc_detach_smo((void**)sm_scerb, smo_name) );

    return NULL == *sm_scerb;
}


int fnres_scerb_offer(sm_scerb_t* sm_scerb, scevent_t* event_ptr) {
    assert( sm_scerb && event_ptr && "params may not be `NULL`"  );
    assert(event_ptr->filename_len == strlen(event_ptr->filename) + 1 && "`filename_len` may be set correctly!" );
    // TODO: CHECK INIT'ed


    const size_t event_size = FNRES_SCEVENT_SIZE_OF_INST(event_ptr);

    ringbuf_t* const sm_rb_ptr = &sm_scerb->ringbuf;
    uint8_t* const sm_buf_ptr = sm_scerb->buf;
    ringbuf_worker_t* const worker_ptr = &sm_scerb->ringbuf.workers[0];

/* Acquire space in buffer */
    ssize_t rb_cur_off = ringbuf_acquire(sm_rb_ptr, worker_ptr, event_size);
    if (-1 == rb_cur_off) {
        LOG_WARN("Insert failed  --  Buffer had at t-1 not free enough space "
                  "to accommodate event (w/ %ld bytes)", event_size);
        return -2;
    }

/* Write data in buffer + hope 4 the best */
    memcpy(&sm_buf_ptr[rb_cur_off], event_ptr, event_size);
    ringbuf_produce(sm_rb_ptr, worker_ptr);

    return 0;
}
