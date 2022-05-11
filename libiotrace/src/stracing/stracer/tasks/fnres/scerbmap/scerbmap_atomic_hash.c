/**
 * Implements the interface using the library 'atomic-hash'
 *   Source:        https://github.com/divfor/atomic_hash
 *   API reference: " "
 */
#include <string.h>
#include <stdlib.h>

#include "scerbmap.h"
#include "atomic_hash.h"

#include <assert.h>
#include "../../common/error.h"
//#define DEV_DEBUG_ENABLE_LOGS
#include "../../common/debug.h"


/* -- Globals / Constants -- */
static hmap_t *g_scerbmap;      /* TODO:  in BSS -> inited w/ 0  ?? */
#define HMAP_TTL_DISABLE 0


/* -- Functions -- */
/* - Public functions - */
bool scerbmap_is_inited(void) {
    return !! g_scerbmap;
}

void scerbmap_create(long max_size) {
    assert( !g_scerbmap && "scerbmap has been already init'ed" );

    if (!(g_scerbmap = atomic_hash_create(max_size, HMAP_TTL_DISABLE))) {
        LOG_ERROR_AND_EXIT("Couldn't init scerbmap");
    }
}

void scerbmap_destroy(void) {
    assert( g_scerbmap && "scerbmap hasn't been init'ed yet" );

    if ((atomic_hash_destroy(g_scerbmap))) {
        LOG_WARN("Couldn't uninit scerbmap");
    } else {
        g_scerbmap = NULL;
    }
}

int scerbmap_get(const pid_t* tid_ptr, sm_scerb_t** found_sm_scerb) {
    assert( g_scerbmap && "scerbmap hasn't been init'ed yet" );
    assert( tid_ptr && found_sm_scerb && "params may not be `NULL`" );

    sm_scerb_t *found_entry;
    const int hmap_rtnval = atomic_hash_get(g_scerbmap, *tid_ptr, sizeof(*tid_ptr), NULL, &found_entry);
    if (!hmap_rtnval) {
        *found_sm_scerb = found_entry;
    } else {
        DEV_DEBUG_PRINT_MSG("Couldn't find scerb-pointer of tid=%ld (err_code=%d) ...", *tid_ptr, hmap_rtnval);
    }

    return hmap_rtnval;
}

void scerbmap_add(const pid_t* tid_ptr, sm_scerb_t* sm_scerb) {
    assert( g_scerbmap && "scerbmap hasn't been init'ed yet" );
    assert( tid_ptr && sm_scerb && "params may not be `NULL`" );

    const int hmap_rtnval = atomic_hash_add(g_scerbmap, *tid_ptr, sizeof(*tid_ptr), sm_scerb, HMAP_TTL_DISABLE, NULL, NULL));
    if (!hmap_rtnval) {
        DEV_DEBUG_PRINT_MSG("Added scerb-pointer of tid=%ld", *tid_ptr);
        return 0;
    } else {
        LOG_WARN("Couldn't add scerb-pointer of tid=%ld", *tid_ptr);
        return -1;
    }
}

void scerbmap_remove(const pid_t* tid_ptr) {
    assert(g_scerbmap && "scerbmap hasn't been init'ed yet" );
    assert( tid_ptr && "param may not be `NULL`" );

    const int hmap_rtnval = atomic_hash_del(g_scerbmap, *tid_ptr, sizeof(*tid_ptr), NULL, NULL);
    if (hmap_rtnval) {
        LOG_ERROR_AND_EXIT("Couldn't delete scerb-pointer  of tid=%d (err_code=%d)", *tid_ptr, hmap_rtnval);
    } else {
        DEV_DEBUG_PRINT_MSG("Removed scerb-pointer  of tid=%lu", tid_ptr);
    }
}
