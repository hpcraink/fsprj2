#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "fnmap.h"
#include "../../libs/atomic_hash/atomic_hash.h"
#include "../../error.h"
#include "libiotrace_config.h"


/* -- Globals / Constants -- */
static hash_t *global_map = NULL;
#define TTL_DISABLE 0


/* -- Functions -- */
/* - Internal functions - */
/* ... hooks for hashmap */
/* Hook is necessary for destroying map (and removing values) */
static int del_hook(void *hash_data, void *caller_data ATTRIBUTE_UNUSED) {
    if (hash_data) {
        // LIBIOTRACE_DEBUG("Freeing string/filename '%s'", (char*)hash_data);
        free(hash_data);
    }

    return PLEASE_REMOVE_HASH_NODE;
}

/* ... debugging functions ... */
#ifndef NDEBUG
static int sprint_fnmap_key(const fnmap_key_t *key, char *str_buf, size_t str_buf_size) {
    #define SNPRINTF(id_type_str, id_format_specifier, value) \
        snprintf(str_buf, str_buf_size, "type=%s,id=" #id_format_specifier ",mmap_length=%zu", \
            id_type_str, value, key->mmap_length)

    if (F_DESCRIPTOR == key->type) {
        return SNPRINTF("F_DES", %d, key->id.fildes);
    } else if (F_STREAM == key->type) {
        return SNPRINTF("F_STR", %p, (void *)(key->id.stream));
    } else if (F_MEMORY == key->type) {
        return SNPRINTF("F_MEM", %p, key->id.mmap_start);
    } else if (F_MPI == key->type) {
        return SNPRINTF("F_MPI", %d, key->id.mpi_id);
    } else {
        return SNPRINTF("R_MPI", %d, key->id.mpi_req_id);
    }
}

static void log_fnmap_key(const fnmap_key_t *key) {
    char* key_str_buf = NULL;
    int key_str_buf_size = sprint_fnmap_key(key, NULL, 0) + 1;
    if (NULL != (key_str_buf = malloc(key_str_buf_size))) {
        sprint_fnmap_key(key, key_str_buf, key_str_buf_size);
        LIBIOTRACE_DEBUG("fnmap-key: %s", key_str_buf);

        free(key_str_buf);
    } else {
        LIBIOTRACE_ERROR("Failed printing key (`malloc` returned NULL)");
    }
}

#  define LOG_DEBUG_FNMAP_KEY(key) log_fnmap_key(key)
#else
#  define LOG_DEBUG_FNMAP_KEY(key) do { } while(0)
#endif



/* - Public functions - */
/**
 * Shall be called by `init_process` in event.c
 */
void fnmap_create(long max_size) {
    if (global_map) {
        LIBIOTRACE_ERROR("fnmap has been already init'ed");
    }

    if (!(global_map = atomic_hash_create(max_size, TTL_DISABLE))) {
        LIBIOTRACE_ERROR("Couldn't init fnmap");
    } else {
        atomic_hash_register_hooks(global_map,
                                   NULL, NULL, NULL, NULL, del_hook);
    }
}

void fnmap_destroy(void) {
    if (!global_map) {   /* Used to be assert */
        LIBIOTRACE_ERROR("fnmap hasn't been init'ed yet");
    }

    if ((atomic_hash_destroy(global_map))) {
        LIBIOTRACE_WARN("Couldn't uninit fnmap");
    } else {
        global_map = NULL;
    }
}

int fnmap_get(const fnmap_key_t *key, char **found_fname) {
    if (!global_map || !key) {   /* Used to be assert */
        LIBIOTRACE_ERROR("Invalid key or uninit fnmap");
    }

    int map_operation_result = atomic_hash_get(global_map, key, FNMAP_KEY_SIZE, NULL, found_fname);
    /*if (map_operation_result) {
        LIBIOTRACE_DEBUG("Couldn't find filename using key below (err_code=%d) ...", map_operation_result);
        LOG_DEBUG_FNMAP_KEY(key);
    }*/
    return map_operation_result;
}

void fnmap_add_or_update(const fnmap_key_t *key, const char *fname) {
    if (!global_map || !key || !fname) {   /* Used to be assert */
        LIBIOTRACE_ERROR("Invalid key / fname or uninit fnmap");
    }

    char *filename;
    const size_t fname_size = (strnlen(fname, FILENAME_MAX - 1) + 1 /* +1 for terminating '\0' */);
    if (NULL != (filename = malloc(fname_size))) {
        strncpy(filename, fname, fname_size);                    /* Make copy of filename (which will be stored in fnmap as value) */

        int map_operation_result; bool value_already_removed_for_update = false;
    update_value_after_removal:
        if ((map_operation_result = atomic_hash_add(global_map, key, FNMAP_KEY_SIZE, filename, TTL_DISABLE, NULL, NULL)) ) {

            if (1 == map_operation_result && !value_already_removed_for_update) {      /* UPDATE value under already used key */
                // LIBIOTRACE_DEBUG("Updating value under already existing key ...");
                fnmap_remove(key);
                value_already_removed_for_update = true;
                goto update_value_after_removal;
            }

            LOG_DEBUG_FNMAP_KEY(key);
            LIBIOTRACE_ERROR("Couldn't add value '%s' (err_code=%d [%s])", fname, map_operation_result, (
                    (-1 == map_operation_result) ? "max filenames in fnmap exceeded" : "unknown"));
        } /*else {
            LIBIOTRACE_DEBUG("Added '%s' using following key ...", filename);
            LOG_DEBUG_FNMAP_KEY(key);
        }*/
    } else {
        LIBIOTRACE_ERROR("`malloc` returned NULL for '%s'", fname);
    }
}

void fnmap_remove(const fnmap_key_t *key) {
    if (!global_map || !key) {   /* Used to be assert */
        LIBIOTRACE_ERROR("Invalid key or uninit fnmap");
    }

    int map_operation_result;
    if ((map_operation_result = atomic_hash_del(global_map, key, FNMAP_KEY_SIZE, NULL, NULL))) {
        LOG_DEBUG_FNMAP_KEY(key);
        LIBIOTRACE_ERROR("Couldn't delete value (filename) (err_code=%d)", map_operation_result);
    } /*else {
        LIBIOTRACE_DEBUG("Removed filename using following key ...");
        LOG_DEBUG_FNMAP_KEY(key);
    }*/
}
