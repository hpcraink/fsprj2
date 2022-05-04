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
void fnmap_create(long max_size);
void fnmap_destroy(void);
bool fnmap_is_inited(void);

int fnmap_get(const fnmap_key_t *key, char **found_fname);
void fnmap_add_or_update(const fnmap_key_t *key, const char *fname, uint64_t ts_in_ns);
void fnmap_remove(const fnmap_key_t *key);
// void fnmap_clear(void);

#endif /* LIBIOTRACE_FNMAP_H */
