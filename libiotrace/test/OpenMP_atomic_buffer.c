#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "atomic_buffer.h"
#include "omp.h"

#define BUFFER_REUSES 1000
#define TESTS_PER_BUFFER_PER_THREAD 100

struct test {
	void *pointer;
	int value;
};

int main(void) {
	struct atomic_buffer buf;
	int ret;
	int max_threads;
	int thread_num;
	int iterations;
	int max_mem;
	unsigned int seed;
	void *mem;
	struct test *mems;
	int value;

	max_threads = omp_get_max_threads();
	iterations = TESTS_PER_BUFFER_PER_THREAD * max_threads;
	max_mem = iterations * (sizeof(int) + sizeof(struct atomic_buffer_prefix));

	/* 1. allocating and freeing memory in separate steps, so no allocation interferes with an free
	 * (tests reuse with full buffer) */

	mems = malloc(iterations * sizeof(struct test));
	assert(NULL != mems);

	ret = atomic_buffer_create(&buf, max_mem);
	assert(-1 != ret);

	for (int l = 0; l < BUFFER_REUSES; l++) {

		// check if all memory is available for allocation
		assert(atomic_buffer_get_free_memory(&buf) == max_mem);
		assert(atomic_buffer_get_freed_memory(&buf) == 0);

#   pragma omp parallel private(seed, mem, thread_num)
		{
			thread_num = omp_get_thread_num();
			seed = thread_num;

			// parallel allocate memory up to maximum buffer size and fill allocated memory with random values
#       	pragma omp for
			for (int i = 0; i < iterations; i++) {
				mems[i].value = rand_r(&seed);

				mem = atomic_buffer_alloc(&buf, sizeof(int));
				assert(NULL != mem);

				// save random values in separate array for later evaluation
				*((int*) mem) = mems[i].value;

				mems[i].pointer = mem;
			} // implicit barrier: all threads wait for each other

			// check if all memory is used
			assert(atomic_buffer_get_free_memory(&buf) == 0);
			assert(atomic_buffer_get_freed_memory(&buf) == 0);

			// parallel allocate more than maximum buffer size memory (must fail)
			mem = atomic_buffer_alloc(&buf, 0);
			assert(NULL == mem);

			// check if allocated memory got not overwritten (each call to alloc must return a separate memory area)
#      		pragma omp for
			for (int i = 0; i < iterations; i++) {
				assert(*((int* )(mems[i].pointer)) == mems[i].value);
			} // implicit barrier: all threads wait for each other

			// free allocated memory in buffer
#       	pragma omp for
			for (int i = 0; i < iterations; i++) {
				atomic_buffer_free(&buf, mems[i].pointer);
			} // implicit barrier: all threads wait for each other

			// check if all memory is freed
			assert(atomic_buffer_get_free_memory(&buf) == 0);
			assert(atomic_buffer_get_freed_memory(&buf) == max_mem);
		}

		// sequential allocate more than maximum buffer size memory (must fail)
		mem = atomic_buffer_alloc(&buf, 1);
		assert(NULL == mem);
		mem = atomic_buffer_alloc(&buf, 0);
		assert(NULL == mem);

		atomic_buffer_reuse(&buf, 0);
	}

	atomic_buffer_destroy(&buf, 0);

	/* 2. interfering allocation, freeing and reuse */

	ret = atomic_buffer_create(&buf, max_mem);
	assert(-1 != ret);

	// check if all memory is available for allocation
	assert(atomic_buffer_get_free_memory(&buf) == max_mem);
	assert(atomic_buffer_get_freed_memory(&buf) == 0);

#   pragma omp parallel private(seed, mem, thread_num, value)
	{
		thread_num = omp_get_thread_num();
		seed = thread_num;

		// parallel allocate memory up to maximum buffer size and fill allocated memory with random values
#       pragma omp for
		for (int i = 0; i < iterations * BUFFER_REUSES; i++) {
			value = rand_r(&seed);

			mem = atomic_buffer_alloc(&buf, sizeof(int));
			while (NULL == mem) {
#               pragma omp critical
				{
					atomic_buffer_reuse(&buf, 0);
				}
				mem = atomic_buffer_alloc(&buf, sizeof(int));
			}

			*((int*) mem) = value;

			// check if allocated memory got not overwritten (each call to alloc must return a separate memory area)
			assert(*((int* )mem) == value);

			// free allocated memory in buffer
			atomic_buffer_free(&buf, mem);
		} // implicit barrier: all threads wait for each other
	}

	atomic_buffer_destroy(&buf, 0);

	free(mems);

	return 0;
}
