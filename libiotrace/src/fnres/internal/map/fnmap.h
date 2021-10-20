/**
* Filename map (fnmap)
*   Implements abstraction (API) for map implementation (which is used for tracking filenames)
*/

#ifndef LIBIOTRACE_FNMAP_H
#define LIBIOTRACE_FNMAP_H

#include <stdio.h>          /* FILE struct */
#include <sys/types.h>      /* sizt_t */


// - Data structures -
typedef union id {
    int fildes;
    FILE* stream;
    void* dir;              /* Use `void*` instead of `DIR*` (to avoid missing header `dirent.h`) */
    void* mmap_start;

    int mpi_id;
    int mpi_req_id;
} id;

typedef enum id_type {
    F_DESCRIPTOR,
    F_STREAM,
    F_DIR,
    F_MEMORY,
    F_MPI,
    R_MPI                                           /* Immediate mapping for MPI_Request struct */
} id_type;

typedef struct fnmap_key {
    id id;
    id_type type;
    size_t mmap_length;                             /* Only relevant 4 file_memory (0 indicates none) */
} fnmap_key;

#define FNMAP_KEY_SIZE sizeof(fnmap_key)


/* - Function prototypes - */
void fnmap_create(size_t max_size);
void fnmap_destroy(void);

int fnmap_get(fnmap_key* key, char** found_fname);
void fnmap_add_or_update(fnmap_key* key, const char* fname);    /* `const char*` -> avoid compiler warning */
void fnmap_remove(fnmap_key* key);
// void fnmap_clear(void);

#endif /* LIBIOTRACE_FNMAP_H */