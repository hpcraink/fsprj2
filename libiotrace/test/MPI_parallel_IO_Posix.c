/** Example in which every process writes its own data
 *  via POSIX I/O **/

#include "mpi.h"
#include <stdio.h>
#define BUFSIZE 100

int main(int argc, char *argv[]){
    
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