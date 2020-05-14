/** Example in which process 0 collects all the data
 *  and writes it via POSIX I/O **/

#include "mpi.h"
#include <stdio.h>
#define BUFSIZE 100

int main(int argc, char *argv[]){
    
    int i, rank, procs, buf[BUFSIZE];
    MPI_Status status;
    FILE *file;

    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &procs);

    // Each process fills its buffer
    for (i=0; i<BUFSIZE; i++) {
        buf[i] = rank * BUFSIZE + i;
    }
    
    if (rank != 0)
        MPI_Send(buf, BUFSIZE, MPI_INT, 0, 99, MPI_COMM_WORLD);
    
    else {
        file = fopen("testfile", "w");
        fwrite(buf, sizeof(int), BUFSIZE, file);

        for (i=1; i<procs; i++) {
            //Auswerten von Status???
            MPI_Recv(buf, BUFSIZE, MPI_INT, i, 99, MPI_COMM_WORLD, &status);
            fwrite(buf, sizeof(int), BUFSIZE, file);
        }

        fclose(file);
    }

    MPI_Finalize();
    
    return 0;
}