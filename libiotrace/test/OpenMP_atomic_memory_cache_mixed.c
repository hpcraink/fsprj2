#include "atomic_memory_cache.h"

#include <stddef.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "omp.h"

int main(int argc, char *argv[]) {
	//int max_threads = omp_get_max_threads();
	int iterations;
	size_t max_memory = 0;

	for (int l = 0; l < atomic_memory_size_count; l++) {
		max_memory += atomic_memory_sizes[l];
	}

	iterations = ATOMIC_MEMORY_BUFFER_SIZE / max_memory;

	printf("start mixing all function calls\n");
#pragma omp parallel
	{
		int thread_num = omp_get_thread_num();
#pragma omp for
		for (int i = 0; i < iterations; i++) {
			for (int l = 0; l < atomic_memory_size_count; l++) {
				union atomic_memory_block_tag_ptr tmp = atomic_memory_alloc(
						(enum atomic_memory_size) l);
				assert(NULL != tmp.tag_ptr.ptr);
				memset(tmp.tag_ptr.ptr->memory, tmp.tag_ptr.ptr->_size,
						atomic_memory_sizes[l]
								- sizeof(struct atomic_memory_block));

				atomic_memory_push(tmp);
			}
		}
	}

	printf("finish\n");
}
