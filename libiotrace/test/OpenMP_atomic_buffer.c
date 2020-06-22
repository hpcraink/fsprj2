#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "atomic_buffer.h"
#include "omp.h"
#ifdef ATOMIC_BUFFER_CACHE_ALIGNED
#  include "os.h"
#endif

#define BUFFER_REUSES 1
#define TESTS_PER_BUFFER_PER_THREAD 1000000

struct test {
	void *pointer;
	int value;
};

// TODO: reference test malloc (vs. atomic_buffer_alloc vs. atomic_buffer_alloc with ATOMIC_BUFFER_CACHE_ALIGNED)
int main(void) {
	struct atomic_buffer buf;
	int ret;
	int max_threads;
	int thread_num;
	int iterations;
	unsigned int seed;
	void *mem;
	struct test *mems;
	int value;
#ifndef ATOMIC_BUFFER_MALLOC_TEST
	int max_mem;
#endif

	max_threads = omp_get_max_threads();
	iterations = TESTS_PER_BUFFER_PER_THREAD * max_threads;
#ifndef ATOMIC_BUFFER_MALLOC_TEST
#  ifndef ATOMIC_BUFFER_CACHE_ALIGNED
	max_mem = iterations * (sizeof(int) + sizeof(struct atomic_buffer_prefix));
#  else
	size_t line_size = cache_line_size();
	max_mem = iterations * ((sizeof(int) + sizeof(struct atomic_buffer_prefix) + line_size - 1) & -line_size);
#  endif
#endif

	/* 1. allocating and freeing memory in separate steps, so no allocation interferes with an free
	 * (tests reuse of buffer with full buffer) */

	mems = malloc(iterations * sizeof(struct test));
	assert(NULL != mems);

#ifndef ATOMIC_BUFFER_MALLOC_TEST
	ret = atomic_buffer_create(&buf, max_mem);
	assert(-1 != ret);
#endif

	for (int l = 0; l < BUFFER_REUSES; l++) {

#if !defined(ATOMIC_BUFFER_MEASUREMENT) && !defined(ATOMIC_BUFFER_MALLOC_TEST)
		// check if all memory is available for allocation
		assert(atomic_buffer_get_free_memory(&buf) == max_mem);
		assert(atomic_buffer_get_freed_memory(&buf) == 0);
#endif

#   pragma omp parallel private(seed, mem, thread_num)
		{
			thread_num = omp_get_thread_num();
			seed = thread_num;

			// parallel allocate memory up to maximum buffer size and fill allocated memory with random values
#       	pragma omp for
			for (int i = 0; i < iterations; i++) {
				mems[i].value = rand_r(&seed);

#ifndef ATOMIC_BUFFER_MALLOC_TEST
				mem = atomic_buffer_alloc(&buf, sizeof(int));
#else
				mem = malloc(sizeof(int));
#endif
				assert(NULL != mem);

				// save random values in separate array for later evaluation
				*((int*) mem) = mems[i].value;

				mems[i].pointer = mem;
			} // implicit barrier: all threads wait for each other

#if !defined(ATOMIC_BUFFER_MEASUREMENT) && !defined(ATOMIC_BUFFER_MALLOC_TEST)
			// check if all memory is used
			assert(atomic_buffer_get_free_memory(&buf) == 0);
			assert(atomic_buffer_get_freed_memory(&buf) == 0);

			// parallel allocate more than maximum buffer size memory (must fail)
			mem = atomic_buffer_alloc(&buf, 0);
			assert(NULL == mem);
#endif

			// check if allocated memory got not overwritten (each call to alloc must return a separate memory area)
#      		pragma omp for
			for (int i = 0; i < iterations; i++) {
				assert(*((int* )(mems[i].pointer)) == mems[i].value);
			} // implicit barrier: all threads wait for each other

			// free allocated memory in buffer
#       	pragma omp for
			for (int i = 0; i < iterations; i++) {
#ifndef ATOMIC_BUFFER_MALLOC_TEST
				atomic_buffer_free(&buf, mems[i].pointer);
#else
				free(mems[i].pointer);
#endif
			} // implicit barrier: all threads wait for each other

#if !defined(ATOMIC_BUFFER_MEASUREMENT) && !defined(ATOMIC_BUFFER_MALLOC_TEST)
			// check if all memory is freed
			assert(atomic_buffer_get_free_memory(&buf) == 0);
			assert(atomic_buffer_get_freed_memory(&buf) == max_mem);
#endif
		}

#if !defined(ATOMIC_BUFFER_MEASUREMENT) && !defined(ATOMIC_BUFFER_MALLOC_TEST)
		// sequential allocate more than maximum buffer size memory (must fail)
		mem = atomic_buffer_alloc(&buf, 1);
		assert(NULL == mem);
		mem = atomic_buffer_alloc(&buf, 0);
		assert(NULL == mem);
#endif

#ifndef ATOMIC_BUFFER_MALLOC_TEST
		atomic_buffer_reuse(&buf, 0);
#endif
	}

#ifndef ATOMIC_BUFFER_MALLOC_TEST
	atomic_buffer_destroy(&buf, 0);

	/* 2. interfering allocation, freeing and reuse */

	ret = atomic_buffer_create(&buf, max_mem);
	assert(-1 != ret);
#endif

#if !defined(ATOMIC_BUFFER_MEASUREMENT) && !defined(ATOMIC_BUFFER_MALLOC_TEST)
	// check if all memory is available for allocation
	assert(atomic_buffer_get_free_memory(&buf) == max_mem);
	assert(atomic_buffer_get_freed_memory(&buf) == 0);
#endif

#   pragma omp parallel private(seed, mem, thread_num, value)
	{
		thread_num = omp_get_thread_num();
		seed = thread_num;

		// parallel allocate memory up to maximum buffer size and fill allocated memory with random values
#       pragma omp for
		for (int i = 0; i < iterations * BUFFER_REUSES; i++) {
			value = rand_r(&seed);

#ifndef ATOMIC_BUFFER_MALLOC_TEST
			mem = atomic_buffer_alloc(&buf, sizeof(int));
			while (NULL == mem) {
#               pragma omp critical
				{
					atomic_buffer_reuse(&buf, 0);
				}
				mem = atomic_buffer_alloc(&buf, sizeof(int));
			}
#else
			mem = malloc(sizeof(int));
#endif
			assert(NULL != mem);

			*((int*) mem) = value;

			// check if allocated memory got not overwritten (each call to alloc must return a separate memory area)
			assert(*((int* )mem) == value);

			// free allocated memory in buffer
#ifndef ATOMIC_BUFFER_MALLOC_TEST
			atomic_buffer_free(&buf, mem);
#else
			free(mem);
#endif
		} // implicit barrier: all threads wait for each other
	}

#ifndef ATOMIC_BUFFER_MALLOC_TEST
	atomic_buffer_destroy(&buf, 0);
#endif

	free(mems);

	return 0;
}
