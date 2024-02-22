/** Example in which every process writes its own data
 *  via POSIX I/O **/

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "omp.h"

#include "mpi.h"
#include <stdio.h>
#define BUFSIZE 100

void usage (void) {
    printf("USAGE: mpirun mpi_file_io [ITERATIONS]\n");
    printf(" \n");
    printf("Runs MPI File I/O tests on mpi_file_io. If the optional ITERATIONS\n");
    printf("is not specified, this will indefinitely write into file mpi_fil_io.txt\n");
    printf("filling up Your file-system.\n");
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]){
    int max_iterations = INT_MAX; // Iterate indefinitely

    if (argc > 1) {
        char * endptr;
        max_iterations = strtol(argv[1], &endptr, 10);
        if (*endptr != '\0' || max_iterations < 0)
            usage();
    }
    
    for (int j = 0; j < max_iterations; j++) {
        int i, rank, buf[BUFSIZE];
        char filename[128];
        FILE *file;

        MPI_Init(&argc, &argv);

        MPI_Comm_rank(MPI_COMM_WORLD, &rank);

        for (i=0; i<BUFSIZE; i++){
            buf[i] = rank * BUFSIZE + i;
        }

        sprintf(filename, "testfile.%d", rank);
        file = fopen(filename, "w");
        fwrite(buf, sizeof(int), BUFSIZE, file);
        fclose(file);

        MPI_Finalize();
        
        return 0;
    }
}
