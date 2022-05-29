#include <stdio.h>
#include <unistd.h>
#include "omp.h"


int main()
{
	#pragma omp parallel
	{
		printf("Hello Thread!\n");
		printf("Thread %d of %d\n", omp_get_thread_num(), omp_get_num_threads());
	}
	write(0, "Hello, Kernel!\n", 15);
	printf("Hello World!\n");

    return 0;
}
