/**
 * Implements the interface using the library 'lwrb'
 *   Source:        https://github.com/MaJerle/lwrb
 *   API reference: https://docs.majerle.eu/projects/lwrb/en/latest/api-reference/lwrb.html
 */
#include <assert.h>
#include <stdio.h>
#include <stddef.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "scerb.h"
#include "lwrb/lwrb.h"

#include "../../../../../common/include/error.h"


/* -- Globals -- */
static lwrb_t *lwrb;
static fnres_scevent *lwrb_buffer;


/* -- Macros -- */
#define LWRB_STRUCT_SIZE ( sizeof(*lwrb) )



/* -- Functions -- */
/* - Internal helpers - */
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

static void detach_smo(char* smo_name) {
    int smo_fd = DIE_WHEN_ERRNO( shm_open(smo_name, O_RDONLY, 0) );     // NOTE: `mode` flags are required (not a variadic fct on Linux)
    struct stat stat_info;
    DIE_WHEN_ERRNO( fstat(smo_fd, &stat_info) );

    DIE_WHEN_ERRNO( munmap(lwrb, stat_info.st_size) );
    close(smo_fd);
}


/* - Init - */
int fnres_scerb_create_and_attach(char* smo_name, unsigned int min_buf_capacity_bytes) {
    assert( smo_name && "`smo_name` may not be `NULL`"  );
    assert( !lwrb_is_ready(lwrb) && "Ring buffer is already inited." );

/* Cleanup old stuff    (NOTE: Don't check for errors since smo may not exist) */
    shm_unlink(smo_name);

/* Create new shared mem block + map it into caller's address space */
    const size_t min_lwrb_buf_size = min_buf_capacity_bytes + 1;         /* buffer    (+1 due to the lib) ? */
    void* shared_mem_addr; unsigned long long shared_mem_len;
    attach_create_map_smo(
            smo_name, LWRB_STRUCT_SIZE + min_lwrb_buf_size,
            &shared_mem_addr, &shared_mem_len,
            1);

    LOG_DEBUG("Created smo \"%s\" w/ requested `min_buf_capacity_bytes`=%lu (got from OS=%llu)", smo_name,
              LWRB_STRUCT_SIZE + min_lwrb_buf_size, shared_mem_len);

/* Init ringbuffer */
    lwrb = shared_mem_addr;
    lwrb_buffer = shared_mem_addr + LWRB_STRUCT_SIZE;

    return lwrb_init(lwrb, lwrb_buffer, shared_mem_len - LWRB_STRUCT_SIZE) ? 0 : -1;
}

int fnres_scerb_attach(char* smo_name) {
    assert( smo_name && "`smo_name` may not be `NULL`"  );
    assert( !lwrb_is_ready(lwrb) && "Ring buffer is already init'ed." );

/* Get shared memory block + map it into caller's address space */
    void* shared_mem_addr; unsigned long long shared_mem_len;
    attach_create_map_smo(
            smo_name, 0,
            &shared_mem_addr, &shared_mem_len,
            0);

/* 'Init' ringbuffer */
    lwrb = shared_mem_addr;
    lwrb_buffer = shared_mem_addr + LWRB_STRUCT_SIZE;

    LOG_DEBUG("Mapping info: `lwrb`@%p, `lwrb_buffer`@%p < -- > `lwrb.buff`=%p", lwrb, lwrb_buffer, lwrb->buff);
    assert( (void*)lwrb_buffer == (void*)lwrb->buff && "Addresses don't match -> lwrb fcts will cause SEGFAULT" );

    return lwrb_is_ready(lwrb) ? 0 : -1;
}


int fnres_scerb_destory_and_detach(char* smo_name) {
    assert( smo_name && "`smo_name` may not be `NULL`"  );
    assert( lwrb_is_ready(lwrb) && "Ring buffer may be inited prior detaching." );

    lwrb_free(lwrb);

    detach_smo(smo_name);
    lwrb = NULL;
    lwrb_buffer = NULL;

    DIE_WHEN_ERRNO( shm_unlink(smo_name) );

    return !lwrb_is_ready(lwrb) ? 0 : -1;
}

int fnres_scerb_detach(char* smo_name) {
    assert( smo_name && "`smo_name` may not be `NULL`"  );
    assert( lwrb_is_ready(lwrb) && "Ring buffer may be inited prior detaching." );

    detach_smo(smo_name);
    lwrb = NULL;
    lwrb_buffer = NULL;

    return !lwrb_is_ready(lwrb) ? 0 : -1;
}


/* - Operations - */
int fnres_scerb_offer(fnres_scevent* event_ptr) {
    assert( event_ptr && "`event_ptr` may not be `NULL`"  );
    assert(event_ptr->filename_len == strlen(event_ptr->filename) + 1 && "`filename_len` may be set correctly!" );
    assert( lwrb_is_ready(lwrb) && "Ring buffer may be inited prior usage." );

/* Check if there's sufficient space in buffer */
    const size_t buf_free_bytes = lwrb_get_free(lwrb),
                 event_size = FNRES_SCEVENT_SIZE_OF_INST(event_ptr);
    if (event_size > buf_free_bytes) {
        LOG_DEBUG("Insert failed  --  Buffer had at t-1 not free enough space (%zu bytes) "
                  "to accommodate event (w/ %ld bytes)", buf_free_bytes, event_size);
        return -2;
    }

/* Write data in buffer + check for error */
    const size_t bytes_written = lwrb_write(lwrb, event_ptr, event_size);
    if (event_size != bytes_written) {
        // TODO: Try to "reverse" write pointer
        LOG_WARN("Wrote incomplete data (%zu of %zu bytes) .. "
                 "buffer may be in inconsistent state", bytes_written, event_size);
        return -1;
    }

    return 0;
}

int fnres_scerb_poll(fnres_scevent* event_ptr) {
    assert( event_ptr && "`event_ptr` may not be `NULL`"  );
    assert( lwrb_is_ready(lwrb) && "Ring buffer may be inited prior usage." );

/* 'Peek' until struct member field `filename_len`, so we now how many bytes we must read */
#define POLL_LEN_OFFSET_STRUCT offsetof(fnres_scevent, filename)
    fnres_scevent tmp_scevent;
    const size_t bytes_peeked = lwrb_peek(lwrb, 0, &tmp_scevent, POLL_LEN_OFFSET_STRUCT);
    if (bytes_peeked < POLL_LEN_OFFSET_STRUCT) {
        LOG_DEBUG("Read failed  --  Buffer was at t-1 empty (or is in a corrupted state)"
                  "(CURRENT allocation=%zu bytes)", lwrb_get_full(lwrb));
        return -2;
    }

/* Read the next scevent + check for error */
    const size_t peeked_event_size = FNRES_SCEVENT_SIZE_OF_INST(&tmp_scevent),
                 read_bytes        = lwrb_read(lwrb, event_ptr, peeked_event_size);
    if (read_bytes != peeked_event_size) {
        // TODO: Try to "reverse" read pointer
        LOG_WARN("Read incomplete data (%zu of expected ('peeked') %zu bytes) .. "
                 "buffer may be in inconsistent state", read_bytes, peeked_event_size);
        return -1;
    }

    return 0;
}


/* - Misc. - */
int fnres_scerb_is_inited(void) {
    return lwrb_is_ready(lwrb);
}

int fnres_scerb_get_buf_stats(unsigned int *buf_used_bytes_ptr,
                              unsigned int *buf_free_bytes_ptr,
                              unsigned int *buf_capacity_bytes_ptr) {
    assert( lwrb_is_ready(lwrb) && "Ring buffer may be inited prior usage." );
    if (!lwrb_is_ready(lwrb)) {
        return -1;
    }

    if (buf_used_bytes_ptr) {
        *buf_used_bytes_ptr = lwrb_get_full(lwrb);
    }
    if (buf_free_bytes_ptr) {
        *buf_free_bytes_ptr = lwrb_get_free(lwrb);
    }
    if (buf_capacity_bytes_ptr) {
        *buf_capacity_bytes_ptr = (lwrb->size - 1);
    }

    return 0;
}


/* - Debugging - */
void fnres_scerb_debug_print_status(FILE* output_stream) {
    unsigned int buf_used_bytes,
                 buf_free_bytes,
                 buf_capacity_bytes;
    fnres_scerb_get_buf_stats(&buf_used_bytes,
                              &buf_free_bytes,
                              &buf_capacity_bytes);

    fprintf((output_stream) ? (output_stream) : (stdout),
            "RB status: init=%s, total size=%u, usage (in bytes): used=%u, free=%u\n",
            lwrb_is_ready(lwrb) ? ("yes") : ("no"),
            buf_capacity_bytes,
            buf_used_bytes, buf_free_bytes);
}
