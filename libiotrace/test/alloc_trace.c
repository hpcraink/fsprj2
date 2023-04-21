/**
 * Generates an alloc trace (which can be later validated in log-file)
 */
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

#include "common/error.h"



int main(int argc, char** argv) {

    // ---  (0.) Parse CLI args  ---
    unsigned int num_threads = 1;
    bool fork_once = false;
    for (int i = 1; i < argc; ++i) {
#define CLI_ARG_MULTI_PROCESSES "--multi-process"
#define CLI_ARG_MULTI_THREADED "--multi-threaded"
        if (!strcmp(CLI_ARG_MULTI_PROCESSES, argv[i])) {
            fork_once = true;
        } else if (!strcmp(CLI_ARG_MULTI_THREADED, argv[i])) {
            num_threads = 2;
        } else {
            fprintf(stderr, "Usage: %s ["CLI_ARG_MULTI_PROCESSES"]["CLI_ARG_MULTI_THREADED"]", argv[0]);
            return(1);
        }
    }


    if (fork_once) {
        (void)DIE_WHEN_ERRNO( fork() );
    }


#pragma omp parallel for default(none) shared(num_threads,    stderr)
    for (unsigned int i = 0; i < num_threads; ++i) {
        // --  Test `malloc`  --
        void* ptr = DIE_WHEN_ERRNO_VPTR( malloc(666) );
        free(ptr);
        ptr = NULL;

        // --  Test `calloc`  --
        ptr = DIE_WHEN_ERRNO_VPTR( calloc(1, 666) );

        // --  Test `realloc` & `reallocarray`  --
        ptr = DIE_WHEN_ERRNO_VPTR( realloc(ptr, 333) );
#ifdef __linux__
        ptr = DIE_WHEN_ERRNO_VPTR( reallocarray(ptr, 2, 333) );
#endif /* __linux__ */
        free(ptr);
        ptr = NULL;

        // --  Test `posix_memalign`  --
        DIE_WHEN_ERRNO( posix_memalign(&ptr, sizeof(void*), 666) );
        free(ptr);
        ptr = NULL;
    }


//    sleep(1);

    return 0;
}
