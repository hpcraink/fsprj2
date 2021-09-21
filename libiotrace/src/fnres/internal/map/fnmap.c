#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "fnmap.h"
#include "impl/atomic_hash.h"
#include "../../../error.h"


/* - Constants - */
#define TTL_DISABLE 0


/* - Globals - */
static hash_t* global_map = NULL;



/* - Internal functions - */
/* ... hooks for hashmap */
/* Hook is necessary for destroying map (and removing values) */
int __del_hook(void* hash_data, void* caller_data) {
    if (hash_data) {
        LIBIOTRACE_DEBUG("Freeing string/filename '%s'", (char*)hash_data);        // DEBUGGING (TOO VERBOSE -> TODO: RMV LATER)
        free(hash_data);
    }

    return PLEASE_REMOVE_HASH_NODE;
}

/* ... debugging ... */
#ifndef NDEBUG
static int __sprint_fnmap_key(fnmap_key* key, char* str_buf, size_t str_buf_size) {
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

static void __log_fnmap_key(fnmap_key* key) {
    char* key_str_buf = NULL;
    int key_str_buf_size = __sprint_fnmap_key(key, NULL, 0) + ((int)sizeof((char)'\0'));
    if (NULL != (key_str_buf = malloc(key_str_buf_size))) {
        __sprint_fnmap_key(key, key_str_buf, key_str_buf_size);
        LIBIOTRACE_DEBUG("fnmap-key: %s", key_str_buf);

        free(key_str_buf);
    } else {
        LIBIOTRACE_ERROR("Failed printing key (`malloc` returned NULL)");
    }
}

#  define LOG_DEBUG_FNMAP_KEY(key) __log_fnmap_key(key)
#else
#  define LOG_DEBUG_FNMAP_KEY(key) do {} while(0)
#endif



/* - Public functions - */
/**
 * Shall be called by init_process in event.c
 */
void fnmap_create(size_t max_size) {
    if (NULL != global_map) {   /* Used to be assert */
        LIBIOTRACE_ERROR("fnmap has been already init'ed");
    }

    if (NULL == (global_map = atomic_hash_create(max_size, TTL_DISABLE))) {
        LIBIOTRACE_ERROR("Couldn't init fnmap");
    } else {
        global_map->on_del = __del_hook;
    }
}

void fnmap_destroy(void) {
    if (NULL == global_map) {   /* Used to be assert */
        LIBIOTRACE_ERROR("fnmap hasn't been init'ed yet");
    }

    if ((atomic_hash_destroy(global_map))) {
        LIBIOTRACE_WARN("Couldn't uninit fnmap");
    } else {
        global_map = NULL;
    }
}

int fnmap_get(fnmap_key* key, char** found_fname) {
    if (NULL == global_map || NULL == key) {   /* Used to be assert */
        LIBIOTRACE_ERROR("Invalid key or uninit fnmap");
    }

    int map_operation_result;
    if ((map_operation_result = atomic_hash_get(global_map, key, FNMAP_KEY_SIZE, NULL, found_fname))) {
        LIBIOTRACE_DEBUG("Didn't find filename using key below (err_code=%d) ...", map_operation_result);
        LOG_DEBUG_FNMAP_KEY(key);                               // DEBUGGING (TOO VERBOSE -> TODO: RMV LATER)
    }
    return map_operation_result;
}

void fnmap_add_or_update(fnmap_key* key, const char* fname) {
    if (NULL == global_map || NULL == key || NULL == fname) {   /* Used to be assert */
        LIBIOTRACE_ERROR("Invalid key / fname or uninit fnmap");
    }

    char* filename;
    const size_t fname_size = (strlen(fname) * sizeof(char)) + sizeof((char)'\0');
    if (NULL != (filename = malloc(fname_size))) {
        strncpy(filename, fname, fname_size);                    /* Make copy of filename (which will be stored in fnmap as value) */

        int map_operation_result; bool value_already_removed_for_update = false;
    update_value_after_removal:
        if ((map_operation_result = atomic_hash_add(global_map, key, FNMAP_KEY_SIZE, filename, TTL_DISABLE, NULL, NULL)) ) {

            if (1 == map_operation_result && !value_already_removed_for_update) {      /* UPDATE value under already used key */
                LIBIOTRACE_DEBUG("Updating value under already existing key ...");        // DEBUGGING (TOO VERBOSE -> TODO: RMV LATER)
                fnmap_remove(key);
                value_already_removed_for_update = true;
                goto update_value_after_removal;
            }

            LOG_DEBUG_FNMAP_KEY(key);
            LIBIOTRACE_ERROR("Couldn't add value '%s' (err_code=%d [%s])", fname, map_operation_result, (
                    (-1 == map_operation_result) ? "max filenames in fnmap exceeded" : "unknown"));
        } else {
            LIBIOTRACE_DEBUG("Added '%s' using following key ...", filename);        // DEBUGGING (TOO VERBOSE -> TODO: RMV LATER)
            LOG_DEBUG_FNMAP_KEY(key);                               // DEBUGGING (TOO VERBOSE -> TODO: RMV LATER)
        }
    } else {
        LIBIOTRACE_ERROR("`malloc` returned NULL for '%s'", fname);
    }
}

void fnmap_remove(fnmap_key* key) {
    if (NULL == global_map || NULL == key) {   /* Used to be assert */
        LIBIOTRACE_ERROR("Invalid key or uninit fnmap");
    }

    int map_operation_result;
    if ((map_operation_result = atomic_hash_del(global_map, key, FNMAP_KEY_SIZE, NULL, NULL))) {
        LOG_DEBUG_FNMAP_KEY(key);
        LIBIOTRACE_ERROR("Couldn't delete value (filename) (err_code=%d)", map_operation_result);
    } else {
        LIBIOTRACE_DEBUG("Removed filename using following key ...");        // DEBUGGING (TOO VERBOSE -> TODO: RMV LATER)
        LOG_DEBUG_FNMAP_KEY(key);                                                   // DEBUGGING (TOO VERBOSE -> TODO: RMV LATER)
    }
}
