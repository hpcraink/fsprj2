#include <unistd.h>
#include "scerb/scerb_consumer.h"
#include "../../../../fnres/fnmap/fnmap.h"

#include <assert.h>
#include "../../common/error.h"
#define DEV_DEBUG_ENABLE_LOGS
#include "../../common/debug.h"


/* -- Forward type declarations -- */
struct basic;           /* Note: #include "../libiotrace_include_struct.h" causes "Declaration of 'struct basic' will not be visible outside of this function" when incl. in "wrapper_defines.h" */


/* -- Globals -- */
static ATTRIBUTE_THREAD sm_scerb_t *g_scerb;


/* -- Functions -- */
void stracing_fnres_init_scerb(void) {
    assert( !g_scerb && "scerb has been already init'ed" );

    char *smo_name;
    DIE_WHEN_ERRNO( asprintf(&smo_name, "_STRACING__%ld", gettid()) );
    if (-1 == scerb_create_attach(&g_scerb, smo_name)) {
        LOG_ERROR_AND_EXIT("Couldn't create scerb");
    }

    CALL_REAL_ALLOC_SYNC(free)(smo_name);
}


void stracing_fnres_check_and_add_scevents(void) {
    assert( g_scerb && "scerb hasn't been init'ed yet" );

    scevent_t *read_scevent = (scevent_t*)alloca(FNRES_SCEVENT_MAX_SIZE);
    while (0 == scerb_poll(g_scerb, read_scevent)) {
        DEV_DEBUG_PRINT_MSG("Retrieved scevent");
        fnmap_key_t fn_key = {
            .id = { .fildes = read_scevent->fd },
            .type = F_DESCRIPTOR,
            .mmap_length = 0
        };

        if (OPEN == read_scevent->type) {
            fnmap_add_or_update(fn_key, read_scevent->filename);
        } else {
            fnmap_remove(fn_key);
        }
    }
}


int stracing_fnres_lookup_and_alias_stream(struct basic* ioevent_ptr) {
    assert( g_scerb && "scerb hasn't been init'ed yet" );
    assert( __void_p_enum_file_type_file_stream == ioevent_ptr->__void_p_enum_file_type && "ioevent may be only of type STREAM" );


    DEV_DEBUG_PRINT_MSG("stracing > Trying to perform lookup for STREAM based on derived fildes");

    const FILE* const ioevent_stream_ptr = ((struct file_stream *) ioevent_ptr->__file_type)->stream;
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
    if (!fnmap_get(&found_fname, &search_key)) {
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
