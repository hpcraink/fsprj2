/**
 * Implements the interface using the library 'rmind-ringbuf'
 *   Source:        https://github.com/rmind/ringbuf
 *   API reference: " "
 */
#include <assert.h>
#include <stdio.h>
#include <stddef.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "scerb.h"
#include "../rmind-ringbuf/ringbuf.h"

#include "../../../stracer/common/error.h"


/* -- Consts -- */
#define BUFFER_SIZE (10000)

/* -- Data types -- */
struct sm_scerb {
    ringbuf_t ringbuf;
    uint8_t buf[BUFFER_SIZE];
};



/* -- Functions -- */
/* - Internal - */
static void attach_create_map_smo(
                char* smo_name, off_t smo_min_len,
                void** shared_mem_addr_ptr, unsigned long long* shared_mem_len_ptr,
                char o_creat) {
/* Create smo */
    int smo_fd = DIE_WHEN_ERRNO( shm_open(smo_name, O_RDWR | ((o_creat) ? (O_CREAT) : (0)), S_IRUSR | S_IWUSR) );
    if (o_creat) {
        DIE_WHEN_ERRNO( ftruncate(smo_fd, smo_min_len) );
    }

/* Get actual length */
    struct stat stat_info;
    DIE_WHEN_ERRNO( fstat(smo_fd, &stat_info) );
    *shared_mem_len_ptr = stat_info.st_size;         // NOTE: What we get depends on the page size (which may be retrieved via `getconf PAGE_SIZE` or `pagesize`)

/* Map it in address space of caller */
    if (MAP_FAILED == (*shared_mem_addr_ptr = mmap(NULL, *shared_mem_len_ptr, PROT_READ | PROT_WRITE, MAP_SHARED, smo_fd, 0))) {
        LOG_ERROR_AND_EXIT("`mmap`(2): %s", strerror(errno));
    }
    close(smo_fd);
}

static void detach_smo(void** shared_mem_addr_ptr, char* smo_name) {
    int smo_fd = DIE_WHEN_ERRNO( shm_open(smo_name, O_RDONLY, 0) );     // NOTE: `mode` flags are required (not a variadic fct on Linux)
    struct stat stat_info;
    DIE_WHEN_ERRNO( fstat(smo_fd, &stat_info) );

    DIE_WHEN_ERRNO( munmap(*shared_mem_addr_ptr, stat_info.st_size) );
    *shared_mem_addr_ptr = NULL;

    close(smo_fd);
}



/* - Init - */
int fnres_scerb_create(sm_scerb_t** sm_scerb, char* smo_name) {
    assert( sm_scerb && smo_name && "params may not be `NULL`" );
    // TODO: CHECK INIT'ed

    const unsigned long sm_min_size = sizeof(sm_scerb_t);    /* NOTE: Buffer-size is currently fixed; may be in reality larger (multiple of page size) */

/* Cleanup old stuff    (NOTE: Don't check for errors since smo may not exist) */
    shm_unlink(smo_name);

/* Create new shared mem block + map it into caller's address space */
    unsigned long long shared_mem_len;
    attach_create_map_smo(
            smo_name, sm_min_size,
            (void**)sm_scerb, &shared_mem_len,
            1);

    LOG_DEBUG("Created smo \"%s\" w/ requested `min_buf_capacity_bytes`=%lu (got from OS=%llu)", smo_name,
              sm_min_size, shared_mem_len);

/* Init ringbuffer */
    return ringbuf_setup(&(*sm_scerb)->ringbuf, 1, BUFFER_SIZE);
}


int fnres_scerb_attach_register_producer(sm_scerb_t** sm_scerb, char* smo_name) {
    assert( sm_scerb && smo_name && "params may not be `NULL`"  );
    // TODO: CHECK INIT'ed

/* Get shared memory block + map it into caller's address space */
    unsigned long long shared_mem_len;
    attach_create_map_smo(
            smo_name, 0,
            (void**)sm_scerb, &shared_mem_len,
            0);

/* Register producer (!!!  THERE CAN ONLY BE 1  !!!) */
    (void)ringbuf_register(&(*sm_scerb)->ringbuf, 0);

    return 0;
}


int fnres_scerb_destory_and_detach(sm_scerb_t** sm_scerb, char* smo_name) {
    return fnres_scerb_detach(sm_scerb, smo_name);
}


int fnres_scerb_detach(sm_scerb_t** sm_scerb, char* smo_name) {
    assert( sm_scerb && smo_name && "params may not be `NULL`"  );
    // TODO: CHECK INIT'ed

    detach_smo((void**)sm_scerb, smo_name);

    return NULL == *sm_scerb;
}


int fnres_scerb_offer(sm_scerb_t* sm_scerb, scevent_t* event_ptr) {
    assert( sm_scerb && event_ptr && "params may not be `NULL`"  );
    assert(event_ptr->filename_len == strlen(event_ptr->filename) + 1 && "`filename_len` may be set correctly!" );
    // TODO: CHECK INIT'ed


    ringbuf_t* const sm_rb_ptr = &sm_scerb->ringbuf;
    uint8_t* const sm_buf_ptr = sm_scerb->buf;
    ringbuf_worker_t* const worker_ptr = &sm_scerb->ringbuf.workers[0];

/* Acquire space in buffer */
    const size_t event_size = FNRES_SCEVENT_SIZE_OF_INST(event_ptr);
    ssize_t rb_cur_off = ringbuf_acquire(sm_rb_ptr, worker_ptr, event_size);
    if (-1 == rb_cur_off) {
        LOG_WARN("Insert failed  --  Buffer had at t-1 not free enough space "
                  "to accommodate event (w/ %ld bytes)", event_size);
        return -2;
    }

/* Write data in buffer + check for error */
    memcpy(event_ptr, &sm_buf_ptr[rb_cur_off], event_size);
    ringbuf_produce(sm_rb_ptr, worker_ptr);

    return 0;
}

int fnres_scerb_poll(sm_scerb_t* sm_scerb, scevent_t* event_ptr) {
    assert( sm_scerb && event_ptr && "params may not be `NULL`"  );
    // TODO: CHECK INIT'ed


    ringbuf_t* const sm_rb_ptr = &sm_scerb->ringbuf;
    uint8_t* const sm_buf_ptr = sm_scerb->buf;


    size_t rb_cur_off;
    if (0 == ringbuf_consume(sm_rb_ptr, &rb_cur_off)) {
        LOG_DEBUG("Read failed  --  Buffer was at t-1 empty");
        return -2;
    }

/* Read  -> 1st part (until struct member field `filename_len`, so we know # bytes we must read)  -> then 2nd part */
#define POLL_LEN_OFFSET_STRUCT offsetof(scevent_t, filename)
    memcpy(event_ptr, &sm_buf_ptr[rb_cur_off], POLL_LEN_OFFSET_STRUCT);
    memcpy(event_ptr +POLL_LEN_OFFSET_STRUCT, &sm_buf_ptr[rb_cur_off +POLL_LEN_OFFSET_STRUCT], event_ptr->filename_len);

    ringbuf_release(sm_rb_ptr, FNRES_SCEVENT_SIZE_OF_INST(event_ptr));

    return 0;
}


/* - Misc. - */
bool fnres_scerb_is_inited(sm_scerb_t* sm_scerb) {
    assert( sm_scerb && "params may not be `NULL`" );
    return NULL != sm_scerb;
}

