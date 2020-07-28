#include "atomic_memory_cache.h"

#include <stddef.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>
#include <errno.h>

#include "omp.h"

#define DEFAULT_ITERATIONS 1000;

int main(int argc, char *argv[]) {
	/* count threads
	 * count allocations
	 * count bytes
	 * count block sizes
	 * block sizes */
	int iterations;
	int sizes[] = {56, 110, 520, 1678};

	if (argc >= 2) {
		char *endptr;
		errno = 0;
		long l = strtol(argv[1], &endptr, 10);
		if (errno || *endptr != '\0' || argv[1] == endptr || l < INT_MIN
				|| l > INT_MAX) {
			iterations = DEFAULT_ITERATIONS
			;
		} else {
			iterations = l;
		}
	} else {
		iterations = DEFAULT_ITERATIONS
		;
	}

	//int max_threads = omp_get_max_threads();
//	size_t max_memory = 0;
//
//	for (int l = 0; l < atomic_memory_size_count; l++) {
//		max_memory += atomic_memory_sizes[l];
//	}

	//printf("start mixing all function calls\n");
#pragma omp parallel
	{
		//int thread_num = omp_get_thread_num();
#pragma omp for
		for (int i = 0; i < iterations; i++) {
			for (int l = 0; l < atomic_memory_size_count; l++) {
#ifndef ATOMIC_BUFFER_MALLOC_TEST
				union atomic_memory_block_tag_ptr tmp = atomic_memory_alloc(
						(enum atomic_memory_size) l);
				assert(NULL != tmp.tag_ptr.ptr);
				atomic_memory_free(tmp);
#else
				void *tmp = malloc(sizes[l]);
				assert(NULL != tmp);
				free(tmp);
#endif
			}
		}
	}

	//printf("finish\n");
}
