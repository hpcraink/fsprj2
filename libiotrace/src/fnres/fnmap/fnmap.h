/**
 * Filename map (fnmap)
 *   Implements abstraction (API) for map implementation (which is used for tracking filenames)
 */
#ifndef LIBIOTRACE_FNMAP_H
#define LIBIOTRACE_FNMAP_H

#include <stdio.h>
#include <sys/types.h>
#include <stdint.h>
#include <stdbool.h>


/* -- Data structures -- */
typedef union {
    int fildes;
    FILE* stream;
    void* dir;              /* Use `void*` instead of `DIR*` (to avoid missing header `dirent.h`) */
    void* mmap_start;

    int mpi_id;
    int mpi_req_id;
} file_handle_t;

typedef enum {
    F_DESCRIPTOR,
    F_STREAM,
    F_DIR,
    F_MEMORY,

    F_MPI,
    R_MPI                                           /* Immediate mapping for MPI_Request struct */
} file_handle_type_t;

typedef struct {
    file_handle_t id;
    file_handle_type_t type;
    size_t mmap_length;                             /* Only relevant 4 file_memory (0 indicates none) */
} fnmap_key_t;

#define FNMAP_KEY_SIZE sizeof(fnmap_key_t)


/* -- Function prototypes -- */
/**
 * @brief                                 Returns whether fnmap has been already initialized
 *
 * @return bool                          `true` = already inited, otherwise `false`
 */
bool fnmap_is_inited(void);

/**
 * @brief                                 Creates the map w/ the specified capacity
 *                                        (Shall be called by `init_process` in event.c)
 *                                        NOTE: Will terminate entire process on failure
 *
 * @param[in] max_size                    Max # of file-handles which may be stored
 * @return void
 */
void fnmap_create(long max_size);

/**
 * @brief                                 Destroys map
 *
 * @return void
 */
void fnmap_destroy(void);

/**
 * @brief                                 Searches, given a key, for a filename
 *
 * @param[in] key                         Key, indicating file-handle type (which was initially used to store the associated filename)
 * @param[out] found_fname                Will contain (if found) the pointer to the found filename
 *
 * @return int                            `0` when found under specified key
 */
int fnmap_get(const fnmap_key_t *key, char **found_fname);

/**
 * @brief                                 Adds a new filename under the given key or updates an existing filename
 *                                        NOTE: Will terminate entire process on failure
 *
 * @param[in] key                         Key, indicating file-handle type
 * @param[in] fname                       Filename which shall be stored under specified key
 * @param[in] ts_in_ns                    Timestamp of associated io-event (i.e., wrapper_start)
 *
 * @return void
 */
void fnmap_add_or_update(const fnmap_key_t *key, const char *fname, uint64_t ts_in_ns);

/**
 * @brief                                 Removes filename under specified handle
 *                                        NOTE: Will terminate entire process on failure
 *
 * @param[in] key                         Key, indicating file-handle type
 *
 * @return void
 */
void fnmap_remove(const fnmap_key_t *key);

// void fnmap_clear(void);

#endif /* LIBIOTRACE_FNMAP_H */
