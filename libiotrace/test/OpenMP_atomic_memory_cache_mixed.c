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
		size_t tmp_memory = sizeof(struct atomic_memory_block)
				+ atomic_memory_sizes[l];
		max_memory += (tmp_memory + ATOMIC_MEMORY_ALIGNMENT - 1)
				& ~(ATOMIC_MEMORY_ALIGNMENT - 1);
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
				assert(NULL != tmp._tag_ptr._ptr);
				printf("Test: %d/%d, Thread: %d, Pointer: %p:%ld, Bytes: %ld\n",
						i, l, thread_num, (void*) (tmp._tag_ptr._ptr),
						tmp._tag_ptr._tag, atomic_memory_sizes[l]);
				fflush(stdout);
				memset(tmp._tag_ptr._ptr->memory, tmp._tag_ptr._ptr->_size,
						atomic_memory_sizes[l]);

				atomic_memory_push(tmp);

				tmp = atomic_memory_pop();
				assert(NULL != tmp._tag_ptr._ptr);
				for (int j = 0;
						j < atomic_memory_sizes[tmp._tag_ptr._ptr->_size];
						j++) {
					assert(
							tmp._tag_ptr._ptr->memory[j]
									== tmp._tag_ptr._ptr->_size);
				}

				atomic_memory_free(tmp);
			}
		}
	}

	printf("finish\n");
}
