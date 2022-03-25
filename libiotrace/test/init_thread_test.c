#include <pthread.h>
#include <stdio.h>

#include "libiotrace_config.h"

static void* thread_start(ATTRIBUTE_UNUSED void *arg) {
	printf("in thread\n");

	return NULL;
}

int main(void) {
	pthread_t recv_thread;

	int ret = pthread_create(&recv_thread, NULL, thread_start, NULL);
	if (0 != ret) {
		printf("pthread_create() failed. (%d)\n", ret);
		return 1;
	}

	return 0;
}
