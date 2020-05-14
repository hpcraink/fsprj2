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
	MPI_Info *info;
	MPI_Info_create(info);
	MPI_Info_set(*info, "access_style", "read_mostly,sequential");
	//snprintf(csize, 10, "%d", size);
	//MPI_Info_set(*info, "nb_proc", &csize[0]);
	MPI_Info_set(*info, "nb_proc", "4");

	//MPI_File_open(MPI_COMM_WORLD, "Makefile", amode, MPI_INFO_NULL, &fh);
	MPI_File_open(MPI_COMM_WORLD, "Makefile", amode, *info, &fh);
	MPI_File_close(&fh);

	MPI_Info_free(info);

	MPI_Finalize();
}
