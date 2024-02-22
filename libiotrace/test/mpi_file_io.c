/**
 * Make sure tracing library traces all MPI File I/O calls in Threads.
 * 
 * Author: Rainer Keller, HS Esslingen, 2020
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "mpi.h"
#include "omp.h"

const char OUTPUTFILE[] = "mpi_file_io.txt";
//const char OUTPUTFILE[] = "$TMP/mpi_file_io.txt";
// const char OUTPUTFILE[] = "/run/user/968698/mpi_file_io.txt";

// #define WANT_OPENMP
// #define WANT_CLEAR_CACHES
#ifdef WANT_CLEAR_CACHES
#  define SIZE_L3_CACHE    (8 * 1024 * 1024) // 8 MB
#  define CLEAR_CACHES_PER_ITERATIONS 1
#endif

#ifdef WANT_OPENMP
#  ifndef WANT_THREADS
#    define WANT_THREADS
#  endif /* WANT_THREADS */
#endif /* WANT_OPENMP */

#ifdef WANT_THREADS
#  define WANT_MPI_THREAD_LEVEL  MPI_THREAD_MULTIPLE // Either one of MPI_THREAD_MULTIPLE (highest), MPI_THREAD_FUNNELED, MPI_THREAD_SINGLE
#endif

// For Debug, uncomment
// #define DEBUG(x) x
#define DEBUG(x)

#define ERROR_FATAL(s,val)   do { \
        char _mpi_error_string[MPI_MAX_ERROR_STRING]; \
        int _len = MPI_MAX_ERROR_STRING; \
        MPI_Error_string((val), _mpi_error_string, &_len); \
        fprintf(stderr, "ERROR (%s:%d) in %s errval:%d (in case of POSIX call:'%s', in case of MPI:'%s')\n", \
                __FILE__, __LINE__, (s), val, strerror(val), _mpi_error_string); \
        exit(val); \
    } while(0)

#define MPI_CHECK(x)  do { \
        int _ret = x; \
        if (_ret != MPI_SUCCESS) { \
            ERROR_FATAL("MPI", _ret);\
        } \
    } while(0)


#ifdef WANT_CLEAR_CACHES
/**
 * Clears the Data caches, aka it reads and modifies data as large the L3 cache
 */
void clear_dcache(void) {
    static char * buffer = NULL;
    if (NULL == buffer) {
        buffer = malloc(SIZE_L3_CACHE);
        if (NULL == buffer)
            ERROR_FATAL("malloc", ENOMEM);
    }
    buffer[0]++;
    for (int i = 1; i < SIZE_L3_CACHE; i++) {
        buffer[i] += buffer[i-1];
    }
}


#define NOP1      __asm__ __volatile__ ("nop\n\t");
#define NOP2     __asm__ __volatile__ ("mov %eax, %eax\n\t");
#define NOP3     __asm__ __volatile__ ("rolq $0x20, %rax\n\t");
#define NOP           NOP1 NOP2 NOP3
#define NOP10         NOP NOP NOP NOP NOP NOP NOP NOP NOP NOP
#define NOP100        NOP10 NOP10 NOP10 NOP10 NOP10 NOP10 NOP10 NOP10 NOP10 NOP10
#define NOP1000  do { NOP100 NOP100 NOP100 NOP100 NOP100 NOP100 NOP100 NOP100 NOP100 NOP100 } while(0)
/**
 * Clears the Instruction cache (icache), by jumping about some code.
 */
void clear_icache(void) {
    NOP1000;
}
#endif /* WANT_CLEAR_CACHES */

void usage (void) {
    printf("USAGE: mpirun mpi_file_io [ITERATIONS]\n");
    printf("\n");
    printf("Runs MPI File I/O tests on mpi_file_io. If the optional ITERATIONS\n");
    printf("is not specified, this will indefinitely write into file mpi_fil_io.txt\n");
    printf("filling up Your file-system.\n");
    exit(EXIT_FAILURE);
}

int main (int argc, char * argv[]) {
    MPI_File fh;
    int max_iterations = INT_MAX; // Iterate indefinitely
    int comm_size;
    int comm_rank;
    int thread_num = 1;
    int thread_me = 0;

    if (argc > 1) {
        char * endptr;
        max_iterations = strtol(argv[1], &endptr, 10);
        if (*endptr != '\0' || max_iterations < 0)
            usage();
    }

#   ifdef WANT_THREADS
    int mpi_thread_level = WANT_MPI_THREAD_LEVEL;
    int mpi_thread_level_supported;
    MPI_CHECK(MPI_Init_thread(&argc, &argv, mpi_thread_level, &mpi_thread_level_supported));
    if (mpi_thread_level != mpi_thread_level_supported)
        ERROR_FATAL("MPI Thread-level not supported", EINVAL);
#   else
    MPI_CHECK(MPI_Init(&argc, &argv));
#   endif /* WANT_THREADS */

    MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &comm_rank);

#ifdef WANT_OPENMP
#pragma omp parallel
#endif
    {

#       ifdef WANT_OPENMP
        // In case of OpenMP, only the master thread of each process has to participate in opening the file.
#pragma omp master
        {
#       endif
            MPI_CHECK(MPI_File_open(MPI_COMM_WORLD, OUTPUTFILE, MPI_MODE_CREATE | MPI_MODE_RDWR, MPI_INFO_NULL, &fh));
            // MPI_CHECK(MPI_File_set_view(fh, 0, MPI_INT, MPI_INT, "native", MPI_INFO_NULL));
#       ifdef WANT_OPENMP
        }
        // Then all threads have to synchronize to be able to "see" the file, otherwise funny things will happen.
#pragma omp barrier
#       endif

#       ifdef WANT_OPENMP
        {
            int thread_min=1;
            int thread_max=1;
            thread_num = omp_get_num_threads();
            thread_me = omp_get_thread_num();

            // Make sure, all MPI ranks have the same number of threads, otherwise, this would lead to "holes in the file"
            MPI_CHECK(MPI_Allreduce (MPI_IN_PLACE, &thread_min, 1, MPI_INT, MPI_MIN, MPI_COMM_WORLD));
            MPI_CHECK(MPI_Allreduce (MPI_IN_PLACE, &thread_max, 1, MPI_INT, MPI_MAX, MPI_COMM_WORLD));

            if (thread_min != thread_max)
                ERROR_FATAL ("MPI ranks have varying number of threads (would lead to holes in file)", EINVAL);
        }
#       endif /* WANT_OPENMP */

        for (int i = 0; i < max_iterations; i++) {
            MPI_Status status;
            // The value encodes a "unique" id per thread, so
            // maximum of threads allowed is 1000 to be able to distinguish.
            int val = (comm_rank * 1000) + thread_me;
            
            MPI_Offset pos = (i * comm_size * thread_num + comm_rank * thread_num + thread_me) * sizeof(int);
            DEBUG(printf ("Rank:%d Thread:%d writing to pos:%lld\n", comm_rank, thread_num, pos));
            MPI_CHECK(MPI_File_seek(fh, pos, MPI_SEEK_SET));
            MPI_CHECK(MPI_File_write(fh, &val, 1, MPI_INT, &status));

            int count;
            MPI_CHECK(MPI_Get_count(&status, MPI_INT, &count));
            if (1 != count)
                ERROR_FATAL("count is wrong", EINVAL);

#           ifdef WANT_CLEAR_CACHES
            if ((i % CLEAR_CACHES_PER_ITERATIONS) == (CLEAR_CACHES_PER_ITERATIONS-1)) {
                clear_dcache();
                clear_icache();
            }
#           endif
        }


#       ifdef WANT_OPENMP
        // Just like above, only the master thread may close the file...
#pragma omp barrier
#pragma omp master
        {
#       endif
            // The MPI_CHECK(MPI_Barrier(MPI_COMM_WORLD)) is not necessary, as MPI_File_close() is collective (we don't do nonblocking and split collective I/O)
            // The MPI_CHECK(MPI_File_sync(fh)) is done in MPI_File_close()
            MPI_CHECK(MPI_File_close(&fh));
#       ifdef WANT_OPENMP
        }
#       endif
    }

    // Check the file size using POSIX calls.
    struct stat file_stat;
    stat (OUTPUTFILE, &file_stat);
    off_t expected_size = (off_t)max_iterations * comm_size * thread_num * sizeof(int);
    if (file_stat.st_size != expected_size)  {
        fprintf(stderr, "ERROR: expected file size to be %lld Bytes but instead file size is %lld Bytes.\n", (long long int) expected_size, (long long int) file_stat.st_size);
    }

    MPI_CHECK(MPI_Finalize());
    return EXIT_SUCCESS;
}
