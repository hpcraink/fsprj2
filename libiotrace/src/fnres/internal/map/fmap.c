#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

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
        // LOG_DEBUG(_LOG_MODULE_NAME": Freeing string/filename '%s'", (char*)hash_data)
        free(hash_data);
    }

    return PLEASE_REMOVE_HASH_NODE;
}

/* ... debugging ... */
#ifndef NDEBUG
static int __print_fmap_key_to_str(fmap_key* key, char* str_buf, size_t str_buf_size) {
    #define SNPRINTF(id_type_str, id_format_specifier, id_union_value) \
        snprintf(str_buf, str_buf_size, "type=%s,id=" #id_format_specifier ",mmap_length=%zu", \
        id_type_str, key->id.id_union_value, key->mmap_length);

    if (F_DESCRIPTOR == key->type) {
        return SNPRINTF("F_DES", %d, fildes)
    } else if (F_STREAM == key->type) {
        return SNPRINTF("F_STR", %p, stream)
    } else if (F_MEMORY == key->type) {
        return SNPRINTF("F_MEM", %p, mmap_start)
    } else if (F_MPI == key->type) {
        return SNPRINTF("F_MPI", %d, mpi_id)
    } else {
        return SNPRINTF("R_MPI", %d, mpi_req_id)
    }
}

static void __log_fmap_key(fmap_key* key) {
    char* str_buf = NULL;
    int str_buf_size = __print_fmap_key_to_str(key, NULL, 0) + ((int)sizeof((char)'\0'));
    if (NULL != (str_buf = malloc(str_buf_size))) {
        __print_fmap_key_to_str(key, str_buf, str_buf_size);
        LOG_DEBUG(_LOG_MODULE_NAME": fmap-key: %s", str_buf);

        free(str_buf);
    } else {
        LOG_ERROR_AND_EXIT(_LOG_MODULE_NAME": Failed printing key ('malloc' returned NULL)")
    }
}

#  define PRINT_FMAP_KEY_IN_DEBUG(key) __log_fmap_key(key);
#else
#  define PRINT_FMAP_KEY_IN_DEBUG(key)
#endif




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

void fmap_add_or_update(fmap_key* key, const char* fname) {
    if (NULL == global_map || NULL == key || NULL == fname) {   /* Used to be assert */
        LOG_ERROR_AND_EXIT(_LOG_MODULE_NAME": Invalid key / fname or uninit fmap")
    }

    char* filename;
    const size_t fname_size = (strlen(fname) * sizeof(char)) + sizeof((char)'\0');
    if (NULL != (filename = malloc(fname_size))) {
        strncpy(filename, fname, fname_size);                    /* Make copy of filename (which will be stored in fmap as value) */

        int map_operation_result; bool value_already_removed_for_update = false;
    update_value_after_removal:
        if ((map_operation_result = atomic_hash_add(global_map, key, FMAP_KEY_SIZE, filename, TTL_DISABLE, NULL, NULL)) ) {

            if (1 == map_operation_result && !value_already_removed_for_update) {      /* UPDATE value under already used key */
                LOG_DEBUG(_LOG_MODULE_NAME": Updating value under already existing key ...")        // DEBUGGING (TOO VERBOSE -> TODO: RMV LATER)
                fmap_remove(key);
                value_already_removed_for_update = true;
                goto update_value_after_removal;
            }

            PRINT_FMAP_KEY_IN_DEBUG(key)
            LOG_ERROR_AND_EXIT(_LOG_MODULE_NAME": Couldn't add value '%s' (code = %d [%s])", fname, map_operation_result, (
                    (-1 == map_operation_result) ? "max filenames in fmap exceeded" : "unknown"))
        } else {
            LOG_DEBUG(_LOG_MODULE_NAME": Added '%s' using following key ...", filename)        // DEBUGGING (TOO VERBOSE -> TODO: RMV LATER)
            PRINT_FMAP_KEY_IN_DEBUG(key)                               // DEBUGGING (TOO VERBOSE -> TODO: RMV LATER)
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
        PRINT_FMAP_KEY_IN_DEBUG(key)
        LOG_ERROR_AND_EXIT(_LOG_MODULE_NAME": Couldn't delete value (filename).");
    } else {
        LOG_DEBUG(_LOG_MODULE_NAME": Removed filename using following key ...")        // DEBUGGING (TOO VERBOSE -> TODO: RMV LATER)
        PRINT_FMAP_KEY_IN_DEBUG(key)                                                   // DEBUGGING (TOO VERBOSE -> TODO: RMV LATER)
    }
}
