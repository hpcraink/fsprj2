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
	union atomic_memory_block_tag_ptr *tests;
	int pop_counts[atomic_memory_size_count] = { 0 };

	for (int l = 0; l < atomic_memory_size_count; l++) {
		max_memory += atomic_memory_sizes[l];
	}

	iterations = ATOMIC_MEMORY_BUFFER_SIZE / max_memory;

	tests = malloc(
			iterations * atomic_memory_size_count
					* sizeof(union atomic_memory_block_tag_ptr));
	assert(NULL != tests);

//	for (int i = 0; i < atomic_memory_size_count; i++) {
//		printf("%p, %ld\n", atomic_memory_cache[i]._tag_ptr._ptr, atomic_memory_cache[i]._tag_ptr._tag);
//	}

	printf("start alloc and push\n");
#pragma omp parallel
	{
		int thread_num = omp_get_thread_num();
#pragma omp for
		for (int i = 0; i < iterations; i++) {
			for (int l = 0; l < atomic_memory_size_count; l++) {
				union atomic_memory_block_tag_ptr tmp = atomic_memory_alloc(
						(enum atomic_memory_size) l);
				assert(NULL != tmp.tag_ptr.ptr);
				printf("Test: %d/%d, Thread: %d, Pointer: %p:%ld, Bytes: %ld\n",
						i, l, thread_num, (void*) (tmp.tag_ptr.ptr),
						tmp.tag_ptr._tag, atomic_memory_sizes[l]);
				fflush(stdout);
				memset(tmp.tag_ptr.ptr->memory, tmp.tag_ptr.ptr->_size,
						atomic_memory_sizes[l]
								- sizeof(struct atomic_memory_block));
				tests[i * atomic_memory_size_count + l] = tmp;
#ifdef ATOMIC_MEMORY_CACHE_PER_NODE
				atomic_memory_push(tmp);
#endif
			}
		}
	}

	printf("start check alloc\n");
#pragma omp parallel for
	for (int i = 0; i < iterations; i++) {
		for (int l = 0; l < atomic_memory_size_count; l++) {
			union atomic_memory_block_tag_ptr tmp = tests[i
					* atomic_memory_size_count + l];
			for (int j = 0;
					j
							< atomic_memory_sizes[tmp.tag_ptr.ptr->_size]
									- sizeof(struct atomic_memory_block); j++) {
				assert(
						tmp.tag_ptr.ptr->memory[j]
								== tmp.tag_ptr.ptr->_size);
			}
		}
	}

#ifdef ATOMIC_MEMORY_CACHE_PER_NODE
	printf("start pop and free\n");
#pragma omp parallel for reduction(+:pop_counts[:atomic_memory_size_count])
	for (int i = 0; i < iterations; i++) {
		for (int l = 0; l < atomic_memory_size_count; l++) {
			union atomic_memory_block_tag_ptr tmp = atomic_memory_pop();
			assert(NULL != tmp.tag_ptr.ptr);
			for (int j = 0;
					j
							< atomic_memory_sizes[tmp.tag_ptr.ptr->_size]
									- sizeof(struct atomic_memory_block); j++) {
				assert(
						tmp.tag_ptr.ptr->memory[j]
								== tmp.tag_ptr.ptr->_size);
			}
			pop_counts[l]++;
			atomic_memory_free(tmp);
		}
	}
	for (int l = 0; l < atomic_memory_size_count; l++) {
		assert(pop_counts[l] == iterations);
	}

	printf("start alloc from cache\n");
#pragma omp parallel
	{
		int thread_num = omp_get_thread_num();
#pragma omp for
		for (int i = 0; i < iterations; i++) {
			for (int l = 0; l < atomic_memory_size_count; l++) {
				union atomic_memory_block_tag_ptr tmp = atomic_memory_alloc(
						(enum atomic_memory_size) l);
				assert(NULL != tmp.tag_ptr.ptr);
				printf("Test: %d/%d, Thread: %d, Pointer: %p:%ld, Bytes: %ld\n",
						i, l, thread_num, (void*) (tmp.tag_ptr.ptr),
						tmp.tag_ptr._tag, atomic_memory_sizes[l]);
				fflush(stdout);
				memset(tmp.tag_ptr.ptr->memory,
						tmp.tag_ptr.ptr->_size + atomic_memory_size_count,
						atomic_memory_sizes[l]
								- sizeof(struct atomic_memory_block));
				tests[i * atomic_memory_size_count + l] = tmp;
			}
		}
	}
#endif

	printf("start check alloc\n");
#pragma omp parallel for
	for (int i = 0; i < iterations; i++) {
		for (int l = 0; l < atomic_memory_size_count; l++) {
			union atomic_memory_block_tag_ptr tmp = tests[i
					* atomic_memory_size_count + l];
			for (int j = 0;
					j
							< atomic_memory_sizes[tmp.tag_ptr.ptr->_size]
									- sizeof(struct atomic_memory_block); j++) {
				assert(
						tmp.tag_ptr.ptr->memory[j]
								== tmp.tag_ptr.ptr->_size
										+ atomic_memory_size_count);
			}
		}
	}

	printf("finish\n");
}
