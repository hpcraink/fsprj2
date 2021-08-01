//
// API of map impl. used by this module to track filenames
//

#ifndef LIBIOTRACE_FMAP_H
#define LIBIOTRACE_FMAP_H

#include <sys/types.h>


// - Data structures -
// TODO: Use enum 'file_type' from ../../libiotrace_structs.h @ 978
typedef enum id_type {
    file_descriptor,
    file_stream,
    file_memory,
    file_mpi
} id_type;

typedef struct fmap_key {
    unsigned long long id;           // FILE* (stream) / int (fildes) / void* (mmap)
    id_type type;
    ssize_t length;                  // Only relevant 4 file_memory (-1 indicates none)
} fmap_key;


// - Function prototypes -
void fmap_create(void);
void fmap_destroy(void);

char* fmap_get(fmap_key* key);
void fmap_set(fmap_key* key, char* filename);
void fmap_remove(fmap_key* key);

#endif //LIBIOTRACE_FMAP_H
