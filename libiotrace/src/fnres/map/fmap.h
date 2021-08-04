//
// API of map impl. used by this module to track filenames
//

#ifndef LIBIOTRACE_FMAP_H
#define LIBIOTRACE_FMAP_H

#include <stdio.h>
#include <sys/types.h>


// - Data structures -
// TODO: Use enum 'file_type' from ../../libiotrace_structs.h @ 978
typedef union id {              // ?? UNION garbage values ??
    int fildes;
    FILE* stream;
    void* start;
} id;

typedef enum id_type {
    file_descriptor,
    file_stream,
    file_memory,
    file_mpi
} id_type;

typedef struct fmap_key {
    id id;
    id_type type;
    size_t length;                   // Only relevant 4 file_memory (0 indicates none)
} fmap_key;


// - Function prototypes -
void fmap_create(void);
void fmap_destroy(void);

char* fmap_get(fmap_key* key);
void fmap_set(fmap_key* key, char* fname);
void fmap_remove(fmap_key* key);

#endif //LIBIOTRACE_FMAP_H
