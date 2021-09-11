/**
* API of map impl. used for tracking filenames
*/

#ifndef LIBIOTRACE_FMAP_H
#define LIBIOTRACE_FMAP_H

#include <stdio.h>
#include <sys/types.h>


// - Data structures -
typedef union id {
    int fildes;
    FILE* stream;
    void* mmap_start;

    int mpi_id;
    int mpi_req_id;
} id;

typedef enum id_type {
    F_DESCRIPTOR,
    F_STREAM,
    F_MEMORY,
    F_MPI,
    R_MPI                                           /* Immediate mapping for MPI_Request struct */
} id_type;

typedef struct fmap_key {
    id id;
    id_type type;
    size_t mmap_length;                             /* Only relevant 4 file_memory (0 indicates none) */
} fmap_key;

#define FMAP_KEY_SIZE sizeof(fmap_key)


/* - Function prototypes - */
void fmap_create(size_t max_size);
void fmap_destroy(void);

int fmap_get(fmap_key* key, char** found_fname);
void fmap_set(fmap_key* key, const char* fname);    /* `const char*` -> avoid compiler warning */
void fmap_remove(fmap_key* key);
// void fmap_clear(void);

#endif /* LIBIOTRACE_FMAP_H */