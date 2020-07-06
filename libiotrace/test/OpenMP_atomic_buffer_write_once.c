#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include "atomic_buffer_write_once.h"
#include "omp.h"
#ifdef ATOMIC_BUFFER_CACHE_ALIGNED
#  include "os.h"
#endif

#define BUFFER_REUSES 1
#define TESTS_PER_BUFFER_PER_THREAD 100000
#define BLOCK_SIZE 5000

struct test {
	void *pointer;
	char value;
};

int main(int argc, char *argv[]) {
	struct atomic_buffer_write_once buf;
	int ret;
	int max_threads;
	int iterations;
	void *mem;
	struct test *mems;
	int value;
#ifndef ATOMIC_BUFFER_MALLOC_TEST
	int max_mem;
#endif
	int block_size;
	int tests_per_buffer_per_thread;

	if (argc >= 2) {
		char *endptr;
		errno = 0;
		long l = strtol(argv[1], &endptr, 10);
		if (errno || *endptr != '\0' || argv[1] == endptr || l < INT_MIN
				|| l > INT_MAX) {
			block_size = BLOCK_SIZE;
		} else {
			block_size = l;
		}
	} else {
		block_size = BLOCK_SIZE;
	}
	if (argc >= 3) {
		char *endptr;
		errno = 0;
		long l = strtol(argv[2], &endptr, 10);
		if (errno || *endptr != '\0' || argv[1] == endptr || l < INT_MIN
				|| l > INT_MAX) {
			tests_per_buffer_per_thread = TESTS_PER_BUFFER_PER_THREAD;
		} else {
			tests_per_buffer_per_thread = l;
		}
	} else {
		tests_per_buffer_per_thread = TESTS_PER_BUFFER_PER_THREAD;
	}
	printf("block-size: %d, tests-per-buffer-per-thread: %d\n", block_size,
			tests_per_buffer_per_thread);

	max_threads = omp_get_max_threads();
	iterations = tests_per_buffer_per_thread * max_threads;
#ifndef ATOMIC_BUFFER_MALLOC_TEST
	max_mem = iterations
			* (block_size + sizeof(struct atomic_buffer_write_once_prefix));

	ret = atomic_buffer_write_once_create(&buf, max_mem);
	assert(-1 != ret);
#endif

	mems = malloc(iterations * sizeof(struct test));
	assert(NULL != mems);

	for (int l = 0; l < BUFFER_REUSES; l++) {

#   pragma omp parallel private(mem	)
		{
			// parallel allocate memory up to maximum buffer size and fill allocated memory with random values
#       	pragma omp for
			for (int i = 0; i < iterations; i++) {
				mems[i].value = '0' + (i % 10);

#ifndef ATOMIC_BUFFER_MALLOC_TEST
				mem = atomic_buffer_write_once_alloc(&buf, block_size);
#else
				mem = malloc(block_size);
#endif
				assert(NULL != mem);

				memset(mem, '0' + (i % 10), block_size);

				mems[i].pointer = mem;
			} // implicit barrier: all threads wait for each other

#if !defined(ATOMIC_BUFFER_MEASUREMENT) && !defined(ATOMIC_BUFFER_MALLOC_TEST)
			// parallel allocate more than maximum buffer size memory (must fail)
			mem = atomic_buffer_write_once_alloc(&buf, 0);
			assert(NULL == mem);
#endif

			// check if allocated memory got not overwritten (each call to alloc must return a separate memory area)
#      		pragma omp for
			for (int i = 0; i < iterations; i++) {
				for (int l = 0; l < block_size; l++) {
					assert(mems[i].value == *(((char* )mems[i].pointer) + l));
				}
			} // implicit barrier: all threads wait for each other

			// free allocated memory in buffer
#ifdef ATOMIC_BUFFER_MALLOC_TEST
#       	pragma omp for
			for (int i = 0; i < iterations; i++) {
				free(mems[i].pointer);
			} // implicit barrier: all threads wait for each other
#endif
		}

#if !defined(ATOMIC_BUFFER_MEASUREMENT) && !defined(ATOMIC_BUFFER_MALLOC_TEST)
		// sequential allocate more than maximum buffer size memory (must fail)
		mem = atomic_buffer_write_once_alloc(&buf, 1);
		assert(NULL == mem);
		mem = atomic_buffer_write_once_alloc(&buf, 0);
		assert(NULL == mem);
#endif

#ifndef ATOMIC_BUFFER_MALLOC_TEST
		atomic_buffer_write_once_destroy(&buf);
		ret = atomic_buffer_write_once_create(&buf, max_mem);
		assert(-1 != ret);
#endif
	}

#ifndef ATOMIC_BUFFER_MALLOC_TEST
	atomic_buffer_write_once_destroy(&buf);
#endif

	free(mems);

	return 0;
}
