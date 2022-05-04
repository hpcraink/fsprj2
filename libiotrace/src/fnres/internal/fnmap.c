#include <string.h>
#include <stdlib.h>

#include "fnmap.h"
#include "../../libs/atomic_hash/atomic_hash.h"
#include "libiotrace_config.h"

#include <assert.h>
#include "../../error.h"
//#define DEV_DEBUG_ENABLE_LOGS
#include "../../debug.h"


/* -- Globals / Constants -- */
static hmap_t *g_hmap;      /* TODO:  in BSS -> inited w/ 0  ?? */
#define HMAP_TTL_DISABLE 0


/* -- Data types -- */
typedef struct {
    char* fname_ptr;
    uint64_t ts_in_ns;
} fnmap_entry_t;


/* -- Functions -- */
/* - Internal functions - */
/* ... debugging functions ... */
#ifdef DEV_DEBUG_PRINT_MSG
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
    const int key_str_buf_size = sprint_fnmap_key(key, NULL, 0) + 1;
    char* key_str_buf = DIE_WHEN_ERRNO_VPTR( malloc(key_str_buf_size) );

    sprint_fnmap_key(key, key_str_buf, key_str_buf_size);
    DEV_DEBUG_PRINT_MSG("fnmap-key: %s", key_str_buf);

    free(key_str_buf);
}

#  define DEV_DEBUG_PRINT_FNMAP_KEY(key) log_fnmap_key(key)
#else
#  define DEV_DEBUG_PRINT_FNMAP_KEY(key) do { } while(0)
#endif


/* ... hooks for hashmap */
/* Hook is necessary for destroying map (and removing values) */
static int hmap_del_hook(void *hash_data, void *caller_data ATTRIBUTE_UNUSED) {
    if (hash_data) {
        fnmap_entry_t* entry = (fnmap_entry_t*)hash_data;
        DEV_DEBUG_PRINT_MSG("Freeing entry (`ts_in_ns`=%lu,`fname_ptr`=\"%s\")", entry->ts_in_ns, entry->fname_ptr);
        free(entry->fname_ptr);
        free(entry);
    }

    return HOOK_NODE_REMOVE;
}



/* - Public functions - */
bool fnmap_is_inited(void) {
    return !! g_hmap;
}

/**
 * Shall be called by `init_process` in event.c
 */
void fnmap_create(long max_size) {
    assert( !g_hmap && "fnmap has been already init'ed" );

    if (!(g_hmap = atomic_hash_create(max_size, HMAP_TTL_DISABLE))) {
        LIBIOTRACE_ERROR("Couldn't init fnmap");
    } else {
        atomic_hash_register_hooks(g_hmap,
                                   NULL, NULL, NULL, NULL, hmap_del_hook);
    }
}

void fnmap_destroy(void) {
    assert( g_hmap && "fnmap hasn't been init'ed yet" );

    if ((atomic_hash_destroy(g_hmap))) {
        LIBIOTRACE_WARN("Couldn't uninit fnmap");
    } else {
        g_hmap = NULL;
    }
}

int fnmap_get(const fnmap_key_t *key, char **found_fname) {
    assert( g_hmap && "fnmap hasn't been init'ed yet" );
    assert( key && "params may not be `NULL`" );

    fnmap_entry_t *found_entry;
    const int map_operation_result = atomic_hash_get(g_hmap, key, FNMAP_KEY_SIZE, NULL, &found_entry);
    if (!map_operation_result) {
        *found_fname = found_entry->fname_ptr;
    } else {
        DEV_DEBUG_PRINT_MSG("Couldn't find filename using key below (err_code=%d) ...", map_operation_result);
        DEV_DEBUG_PRINT_FNMAP_KEY(key);
    }

    return map_operation_result;
}

void fnmap_add_or_update(const fnmap_key_t *key, const char *fname, const uint64_t ts_in_ns) {
    assert( g_hmap && "fnmap hasn't been init'ed yet" );
    assert( key && fname && "params may not be `NULL`" );

    fnmap_entry_t *new_entry_ptr = DIE_WHEN_ERRNO_VPTR( malloc(sizeof *new_entry_ptr) );
    new_entry_ptr->fname_ptr = DIE_WHEN_ERRNO_VPTR( strdup(fname) );
    new_entry_ptr->ts_in_ns = ts_in_ns;

    int map_operation_result; bool value_already_removed_for_update = false;
update_value_after_removal:
    if ((map_operation_result = atomic_hash_add(g_hmap, key, FNMAP_KEY_SIZE, new_entry_ptr, HMAP_TTL_DISABLE, NULL, NULL)) ) {
        if (1 == map_operation_result && !value_already_removed_for_update) {      /* UPDATE value under already used key */
            DEV_DEBUG_PRINT_MSG("Updating value under already existing key ...");
            fnmap_remove(key);
            value_already_removed_for_update = true;
            goto update_value_after_removal;
        }

        DEV_DEBUG_PRINT_FNMAP_KEY(key);
        LIBIOTRACE_ERROR("Couldn't add value '%s' (err_code=%d [%s])", fname, map_operation_result, (
                (-1 == map_operation_result) ? "max filenames in fnmap exceeded" : "unknown"));
    } else {
        DEV_DEBUG_PRINT_MSG("Added entry (`ts_in_ns`=%lu,`fname_ptr`=\"%s\") using following key ...", new_entry_ptr->ts_in_ns, new_entry_ptr->fname_ptr);
        DEV_DEBUG_PRINT_FNMAP_KEY(key);
    }
}

void fnmap_remove(const fnmap_key_t *key) {
    assert( g_hmap && "fnmap hasn't been init'ed yet" );
    assert( key && "params may not be `NULL`" );

    const int map_operation_result = atomic_hash_del(g_hmap, key, FNMAP_KEY_SIZE, NULL, NULL);
    if (map_operation_result) {
        DEV_DEBUG_PRINT_FNMAP_KEY(key);
        LIBIOTRACE_ERROR("Couldn't delete value (filename) (err_code=%d)", map_operation_result);
    } else {
        DEV_DEBUG_PRINT_MSG("Removed filename using following key ...");
        DEV_DEBUG_PRINT_FNMAP_KEY(key);
    }
}
