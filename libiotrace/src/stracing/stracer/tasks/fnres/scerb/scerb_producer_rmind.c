/**
 * Implements the interface using the library 'rmind-ringbuf'
 *   Source:        https://github.com/rmind/ringbuf
 *   API reference: " "
 */
#include <assert.h>
#include <stdio.h>
#include <stddef.h>

#include "../../../../common/tasks/fnres/scerb/scerb_ipc_utils.h"
#include "../../../../common/tasks/fnres/scerb/scerb_types.h"
#include "../../../../common/tasks/fnres/scerb/scerb_types_rmind.h"

//#define DEV_DEBUG_ENABLE_LOGS
#include "../../../../../common/debug.h"
#include "common/error.h"


/* -- Functions -- */
/* - Init - */
int fnres_scerb_attach(sm_scerb_t** sm_scerb, char* smo_name) {
    assert( sm_scerb && smo_name && "params may not be `NULL`" );
    // TODO: CHECK INIT'ed

/* Get shared memory block + map it into caller's address space */
    unsigned long long shared_mem_len;
    DIE_WHEN_ERRNO( sm_ipc_attach_create_smo(smo_name, 0, (void**)sm_scerb, &shared_mem_len, 0) );

/* Register producer (!!!  THERE CAN ONLY BE 1  !!!) */
    (void)ringbuf_register(&(*sm_scerb)->ringbuf, 0);

    return 0;
}


int fnres_scerb_destory_detach(sm_scerb_t** sm_scerb, char* smo_name) {
    assert( sm_scerb && smo_name && "params may not be `NULL`" );
    // TODO: CHECK INIT'ed

    DIE_WHEN_ERRNO( sm_ipc_detach_smo((void**)sm_scerb, smo_name) );

    return (NULL == *sm_scerb) ? (0) : (-1);
}


int fnres_scerb_offer(sm_scerb_t* sm_scerb, scevent_t* event_buf_ptr) {
    assert(sm_scerb && event_buf_ptr && "params may not be `NULL`"  );
    // TODO: CHECK INIT'ed
    assert( event_buf_ptr->filename_len == strlen(event_buf_ptr->filename) + 1 && "`filename_len` may be set correctly!" );


    ringbuf_t* const sm_rb_ptr = &sm_scerb->ringbuf;
    uint8_t* const sm_buf_ptr = sm_scerb->buf;
    ringbuf_worker_t* const rb_worker_ptr = &sm_scerb->ringbuf.workers[0];


/* Acquire space in buffer */
    const size_t event_size = FNRES_SCEVENT_SIZEOF(event_buf_ptr);
    ssize_t cur_write_offset = ringbuf_acquire(sm_rb_ptr, rb_worker_ptr, event_size);
    if (-1 == cur_write_offset) {
        DEV_DEBUG_PRINT_MSG("Insert failed  --  Buffer had at t-1 not free enough space "
                            "to accommodate event (w/ %ld bytes)", event_size);
        return -2;
    }

/* Write data in buffer + hope 4 the best */
    memcpy(&sm_buf_ptr[cur_write_offset], event_buf_ptr, event_size);
    ringbuf_produce(sm_rb_ptr, rb_worker_ptr);

    return 0;
}
