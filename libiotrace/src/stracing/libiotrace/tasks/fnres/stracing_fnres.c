/**
 * NOTE: This approach using a ringbuffer w/ per-tread view has a major drawback:
 *       - Not traced io-relevant events will only be added to fnmap if a wrapper
 *         for the affected thread gets triggered (which then in turn reads the scerb)
 *       - BETTER would be: Put fnmap in shared memory w/ a process-wide view
 *
 */
#include "libiotrace_config.h"

#include "../../../common/stracer_consts.h"
#include "scerb/scerb_consumer.h"
#include "../../../../fnres/fnmap/fnmap.h"

#include <unistd.h>

#include <assert.h>
#include "../../../../common/error.h"
#define DEV_DEBUG_ENABLE_LOGS
#include "../../../../common/debug.h"


/* -- Forward type declarations -- */
struct basic;           /* Note: #include "../libiotrace_include_struct.h" causes "Declaration of 'struct basic' will not be visible outside of this function" when incl. in "wrapper_defines.h" */


/* -- Globals -- */
static ATTRIBUTE_THREAD sm_scerb_t *g_scerb;        // NOTE: Must be in TLS (since per thread)


/* -- Functions -- */
static char* derive_smo_name(void) {
    char *smo_name;
    DIE_WHEN_ERRNO( asprintf(&smo_name, STRACING_FNRES_SMO_NAME_FORMAT, gettid()) );        // MUST BE `free`ED !!!
    return smo_name;
}


/* - Public functions - */
void stracing_fnres_setup(void) {
    assert( !g_scerb && "scerb has been already init'ed" );

    char *smo_name = derive_smo_name();
    if (-1 == scerb_create_attach(&g_scerb, smo_name)) {
        LOG_ERROR_AND_EXIT("Couldn't create scerb");
    }

    DEV_DEBUG_PRINT_MSG("Finished setup (i.e., created & attached smo w/ identifier=\"%s\")", smo_name);
    CALL_REAL_ALLOC_SYNC(free)(smo_name);
}

void stracing_fnres_fin(void) {
    assert( g_scerb && "scerb hasn't been init'ed yet" );

    char *smo_name = derive_smo_name();
    if (-1 == scerb_detach(&g_scerb, smo_name)) {
        LOG_WARN("Couldn't detach scerb");
    }

    DEV_DEBUG_PRINT_MSG("Finished cleanup (i.e., detached smo w/ identifier=\"%s\")", smo_name);
    CALL_REAL_ALLOC_SYNC(free)(smo_name);
}


void stracing_fnres_check_and_add_scevents(void) {
//    assert( g_scerb && "scerb hasn't been init'ed yet" );
    if (!g_scerb) {                                     // NOTE: May not be inited yet, but still already called by wrappers
        DEV_DEBUG_PRINT_MSG("NOP for tid=%d  (not inited yet)", gettid());
        return;
    }

    scevent_t *scevent_buf_ptr = (scevent_t*)alloca(SCEVENT_MAX_SIZE);
    while (0 == scerb_poll(g_scerb, scevent_buf_ptr)) {
        DEV_DEBUG_PRINT_MSG("Received scevent { .ts_in_ns=%lu, .succeeded=%s, "
                            ".fd=%d, .type=%s, .filename_len=%zu, .filename=%s } ",
                            scevent_buf_ptr->ts_in_ns, scevent_buf_ptr->succeeded ? "true" : "false",
                            scevent_buf_ptr->fd,
                            OPEN == scevent_buf_ptr->type ? "OPEN" : "CLOSE",
                            scevent_buf_ptr->filename_len, scevent_buf_ptr->filename_len ? scevent_buf_ptr->filename : "N/A");

        if (!scevent_buf_ptr->succeeded) { continue; }

        fnmap_key_t sce_key = {
            .id = { .fildes = scevent_buf_ptr->fd },
            .type = F_DESCRIPTOR,
            .mmap_length = 0
        };
        if (OPEN == scevent_buf_ptr->type) {
            fnmap_add_or_update(&sce_key, scevent_buf_ptr->filename, scevent_buf_ptr->ts_in_ns);
        } else {
            fnmap_remove(&sce_key);
        }
    }
}



int stracing_fnres_lookup_and_alias_stream(struct basic* ioevent_ptr) {
    assert( g_scerb && "scerb hasn't been init'ed yet" );
    assert( __void_p_enum_file_type_file_stream == ioevent_ptr->__void_p_enum_file_type && "ioevent may be only of type STREAM" );


    DEV_DEBUG_PRINT_MSG("stracing > Trying to perform lookup for STREAM based on derived fildes");

    FILE* const ioevent_stream_ptr = ((struct file_stream *) ioevent_ptr->__file_type)->stream;
    const uint64_t ioevent_ts_in_ns = ioevent_ptr->time_start;

    const int derived_fd = CALL_REAL_POSIX_SYNC( fileno(ioevent_stream_ptr) );
    if (-1 == derived_fd) {
        LOG_WARN("stracing > Couldn't derive fildes for lookup from stream (errno=%d)", errno);
        return -1;
    }

    fnmap_key_t search_key = {
            .type = F_DESCRIPTOR,
            .id = { .fildes = derived_fd },
            .mmap_length = 0
    };
    char *found_fname;
    if (!fnmap_get(&search_key, &found_fname)) {
        DEV_DEBUG_PRINT_MSG("stracing > Found associated STREAM, adding it to fnmap");

        fnmap_key_t insert_key = {
                .type = F_STREAM,
                .id = { .stream = ioevent_stream_ptr },
                .mmap_length = 0
        };
        fnmap_add_or_update(&insert_key, found_fname, ioevent_ts_in_ns);
        return 0;
    }

    return -2;
}
