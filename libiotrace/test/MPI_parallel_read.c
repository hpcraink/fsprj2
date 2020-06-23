/** Read a file into multiple processes **/

#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#define FILESIZE 1600

int main(int argc, char *argv[]){
    
    int *buf, rank, nprocs, nints, bufsize;
    MPI_File file;
    MPI_Status status;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    bufsize = FILESIZE / nprocs;
    buf = (int *) malloc(bufsize);
    nints = bufsize/sizeof(int);

    printf("%d", nints);

    MPI_File_open(MPI_COMM_WORLD, "testfile", MPI_MODE_RDONLY, MPI_INFO_NULL, &file);

    MPI_File_seek(file, rank*bufsize, MPI_SEEK_SET);

    MPI_File_read(file, buf, nints, MPI_INT, &status);
    MPI_File_close(&file);

    free(buf);
    MPI_Finalize();
    
    return 0;
}
