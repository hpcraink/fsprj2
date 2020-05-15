/** Parallel MPI write into one file **/

#include "mpi.h"
#include <stdio.h>
#define BUFSIZE 100

int main(int argc, char *argv[]){
    
    int i, rank, buf[BUFSIZE];
    MPI_File *file;

    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    for (i=0; i<BUFSIZE; i++){
        buf[i] = rank * BUFSIZE + i;
    }

    MPI_File_open(MPI_COMM_WORLD, "testfile", MPI_MODE_CREATE | MPI_MODE_WRONLY, MPI_INFO_NULL, &file);
    
    MPI_File_set_view(file, rank * BUFSIZE * sizeof(int), MPI_INT, MPI_INT, "native", MPI_INFO_NULL);
    
    MPI_File_write(file, buf, BUFSIZE, MPI_INT, MPI_STATUS_IGNORE);

    MPI_File_close(&file);

    MPI_Finalize();
    
    return 0;
}