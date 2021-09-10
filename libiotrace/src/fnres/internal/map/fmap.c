#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include "fmap.h"
#include "impl/atomic_hash.h"
#include "../logging.h"


/* - Constants - */
#define TTL_DISABLE 0

#define _LOG_MODULE_NAME "fmap"


/* - Globals - */
static hash_t* global_map = NULL;



/* - Internal functions - */
/* ... hooks for hashmap */
/* Hook is necessary for destroying map (and removing values) */
int __del_hook(void* hash_data, void* caller_data) {
    if (hash_data) {
        LOG_DEBUG("Freeing string/filename '%s'", (char*)hash_data)
        free(hash_data);
    }

    return PLEASE_REMOVE_HASH_NODE;
}


/* - Public functions - */
/**
 * Shall be called by init_process in event.c
 */
void fmap_create(size_t max_size) {
    if (NULL != global_map) {   /* Used to be assert */
        LOG_ERROR_AND_EXIT(_LOG_MODULE_NAME": fmap has been already init'ed")
    }

    if (NULL == (global_map = atomic_hash_create(max_size, TTL_DISABLE))) {
        LOG_ERROR_AND_EXIT(_LOG_MODULE_NAME": Couldn't init fmap")
    } else {
        global_map->on_del = __del_hook;
    }
}
void fmap_destroy(void) {
    if (NULL == global_map) {   /* Used to be assert */
        LOG_ERROR_AND_EXIT(_LOG_MODULE_NAME": fmap hasn't been init'ed yet")
    }

    if ((atomic_hash_destroy(global_map))) {
        LOG_WARN(_LOG_MODULE_NAME": Couldn't uninit fmap")
    } else {
        global_map = NULL;
    }
}

int fmap_get(fmap_key* key, char** found_fname) {
    if (NULL == global_map || NULL == key) {   /* Used to be assert */
        LOG_ERROR_AND_EXIT(_LOG_MODULE_NAME": Invalid key or uninit fmap")
    }

    return atomic_hash_get(global_map, key, FMAP_KEY_SIZE, NULL, found_fname);
}

void fmap_set(fmap_key* key, const char* fname) {
    if (NULL == global_map || NULL == key || NULL == fname) {   /* Used to be assert */
        LOG_ERROR_AND_EXIT(_LOG_MODULE_NAME": Invalid key / fname or uninit fmap")
    }

    char* filename;
    const size_t fname_size = (strlen(fname) * sizeof(char)) + sizeof((char)'\0');
    if (NULL != (filename = malloc(fname_size))) {
        strncpy(filename, fname, fname_size);                    /* Make copy of filename */

        if (atomic_hash_add(global_map, key, FMAP_KEY_SIZE, filename, TTL_DISABLE, NULL, NULL)) {
            LOG_ERROR_AND_EXIT(_LOG_MODULE_NAME": Couldn't add value '%s'", fname);
        } else {
            LOG_DEBUG(_LOG_MODULE_NAME": Added '%s'", filename)
        }
    } else {
        LOG_ERROR_AND_EXIT(_LOG_MODULE_NAME": malloc() returned NULL for '%s'", fname);
    }
}

void fmap_remove(fmap_key* key) {
    if (NULL == global_map || NULL == key) {   /* Used to be assert */
        LOG_ERROR_AND_EXIT(_LOG_MODULE_NAME": Invalid key or uninit fmap")
    }

    if (atomic_hash_del(global_map, key, FMAP_KEY_SIZE, NULL, NULL)) {
        LOG_ERROR_AND_EXIT(_LOG_MODULE_NAME": Couldn't delete value (filename)");
    }
}
