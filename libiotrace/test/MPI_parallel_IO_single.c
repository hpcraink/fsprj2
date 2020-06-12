/** Parallel MPI write into one file **/

#include "mpi.h"
#include <stdio.h>
#define BUFSIZE 100

int main(int argc, char *argv[]){
    
    int i, rank, buf[BUFSIZE], err;
    char value [1024];
    MPI_File file;
    MPI_Info info_in;
    MPI_Info info_used;
    int flag;

    MPI_Init(&argc, &argv);
    MPI_Info_create( &info_in );

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    for (i=0; i<BUFSIZE; i++){
        buf[i] = rank * BUFSIZE + i;
    }

    MPI_Info_set( info_in, "access_style", "write_once" );

    MPI_File_open(MPI_COMM_WORLD, "testfile", MPI_MODE_CREATE | MPI_MODE_WRONLY, info_in, &file);

    MPI_Info_free(&info_in);

    //printf("%p", file); //Jeder Prozess bekommt eigenes File handle auf File
    
    MPI_File_set_view(file, rank * BUFSIZE * sizeof(int), MPI_INT, MPI_INT, "native", info_in);
        
    MPI_File_write(file, buf, BUFSIZE, MPI_INT, MPI_STATUS_IGNORE);

    err = MPI_File_get_info( file, &info_used );

    printf("%d\n", err);
    fflush(stdout);
    


    MPI_Info_get(info_used, "access_style", 1024, value, &flag);

    MPI_Info_free(&info_used);

    //printf("%s", value);

    

    MPI_File_close(&file);

    MPI_Finalize();
    
    return 0;
}