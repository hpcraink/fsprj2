#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <time.h>
#include "atomic_buffer.h"
#include "omp.h"

#define COUNT_WAIT_POINTS 3

struct test {
	void *pointer;
	int value;
};

int main(void) {
	struct atomic_buffer buf;
	int *waits;
	int current_waits[COUNT_WAIT_POINTS];
	int ret;
	int tests;
	int max_threads;
	int thread_num;
	int max_mem;
	unsigned int seed;
	void *mem;
	int value;
	char testing;
	char loop;
	//static struct timespec wait_time = { 0, 0 };

	max_threads = omp_get_max_threads();
	// check if there are enough threads to control all
	// possible constellations of wait points
	assert(max_threads > COUNT_WAIT_POINTS);
	// start enough threads with next parallel region
	// (one thread to wait in each wait point and
	// one thread to control the wait points)
	omp_set_num_threads(COUNT_WAIT_POINTS + 1);
	max_threads = COUNT_WAIT_POINTS;

	max_mem = COUNT_WAIT_POINTS
			* (sizeof(int) + sizeof(struct atomic_buffer_prefix));

	tests = pow(COUNT_WAIT_POINTS, COUNT_WAIT_POINTS);

	waits = malloc(tests * COUNT_WAIT_POINTS * sizeof(int));
	assert(NULL != waits);

	// fill array with all possible combinations of wait points
	for (int i1 = 0; i1 < tests; i1++) {
		for (int i2 = 0; i2 < COUNT_WAIT_POINTS; i2++) {
			*(waits + (i1 * COUNT_WAIT_POINTS) + i2) = ((i1
					/ (int) pow(COUNT_WAIT_POINTS, i2)) % COUNT_WAIT_POINTS)
					+ 1;
			printf("%d ", *(waits + (i1 * COUNT_WAIT_POINTS) + i2));
		}
		printf("\n");
	}

	for (int i = 0; i < tests; i++) {

		// pick one combination of wait points
		for (int l = 0; l < COUNT_WAIT_POINTS; l++) {
			current_waits[l] = *(waits + (i * COUNT_WAIT_POINTS) + l);
			printf("worker %d waits for %d\n", l + 1, current_waits[l]);
		}

		testing = 0;
		loop = 1;

		ret = atomic_buffer_create(&buf, max_mem);
		assert(-1 != ret);

		// check if all memory is available for allocation
		assert(atomic_buffer_get_free_memory(&buf) == max_mem);
		assert(atomic_buffer_get_freed_memory(&buf) == 0);

#       pragma omp parallel private(seed, mem, thread_num, value)
		{
			thread_num = omp_get_thread_num();
			seed = thread_num;

			if (thread_num == 0) {

				printf("master started\n");

				// don't wait
				__atomic_store_n(&atomic_buffer_wait_pos, &thread_num,
				__ATOMIC_RELEASE);

			} else {

				printf("worker %d started\n", thread_num);

				// wait
				__atomic_store_n(&atomic_buffer_wait_pos,
						&(current_waits[thread_num - 1]), __ATOMIC_RELEASE);
			}

#           pragma omp barrier

			while (loop) {

				if (thread_num == 0) {

					if (!testing) {
						char test = 0;
						for (int l = 0; l < COUNT_WAIT_POINTS; l++) {
							int current_wait = __atomic_load_n(
									&current_waits[l], __ATOMIC_ACQUIRE);
							if (current_wait != 0) {
								test = 1;
								printf(
										"worker %d not waiting in wait point %d\n",
										l + 1, current_wait);
							}
						}
						if (!test) {
							printf("all workers waiting\n");
							// all wait points are reached (all worker threads are waiting)
							// => enable testing
							testing++;
							// and resume first wait point
							__atomic_store_n(&current_waits[testing - 1],
									*(waits + (i * COUNT_WAIT_POINTS) + testing
											- 1), __ATOMIC_RELEASE);
						}
					} else {
						// check resumed wait point
						if (__atomic_load_n(&current_waits[testing - 1],
						__ATOMIC_ACQUIRE) == 0) {
							printf("worker %d resumed\n", testing);
							// if resumed wait point has made progress
							if (testing < COUNT_WAIT_POINTS) {
								// resume next wait point
								testing++;
								__atomic_store_n(&current_waits[testing - 1],
										*(waits + (i * COUNT_WAIT_POINTS)
												+ testing - 1),
										__ATOMIC_RELEASE);
							} else {
								loop = 0;
							}
						}
					}
					getchar();

				} else {

					switch (current_waits[thread_num - 1]) {
					case 0:
						break;
					case 1:
					case 2:
						value = rand_r(&seed);

						mem = atomic_buffer_alloc(&buf, sizeof(int));
						assert(NULL != mem);

						*((int*) mem) = value;

						// check if allocated memory got not overwritten (each call to alloc must return a separate memory area)
						assert(*((int* )mem) == value);

						// free allocated memory in buffer
						atomic_buffer_free(&buf, mem);
						break;
					case 3:
						atomic_buffer_reuse(&buf, 0);
						break;
					case 4:
						//???
						break;
					}

				}

			}
		} // implicit barrier: all threads wait for each other

		// check if all memory is freed
		assert(
				atomic_buffer_get_free_memory(&buf)
						+ atomic_buffer_get_freed_memory(&buf) == max_mem);

		atomic_buffer_destroy(&buf, 0);

	}

	free(waits);

	return 0;
}
