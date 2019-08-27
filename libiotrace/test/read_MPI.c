#include <stdio.h>
#include "mpi.h"

int main(int argc, char * argv[]) {
	int val, rank, size;
	//MPI_Status status;
	MPI_Init(&argc, &argv);
	MPI_Comm_size (MPI_COMM_WORLD, &size);
	MPI_Comm_rank (MPI_COMM_WORLD, &rank);

	int amode = MPI_MODE_RDONLY | MPI_MODE_SEQUENTIAL;
	MPI_File fh;
	MPI_File_open(MPI_COMM_WORLD, "Makefile", amode, MPI_INFO_NULL, &fh);
	MPI_File_close(&fh);

	MPI_Finalize();
}
