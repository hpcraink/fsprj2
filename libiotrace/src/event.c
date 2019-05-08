#include "libiotrace_config.h"

#include <assert.h>

#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif

#include <sys/syscall.h>

#ifdef HAVE_PTHREAD_H
#  include <pthread.h>
#endif

#include "event.h"
#include "os.h"

#include "json_include_function.h"

/* Buffer */
#define BUFFER_SIZE 500
static char data_buffer[BUFFER_SIZE];
static char* endpos;
static char* pos;
static int count_basic;

/* Mutex */
static pthread_mutex_t lock;

static void init()__attribute__((constructor));
// ToDo: test for destructor in cmake
static void cleanup()__attribute__((destructor));

static void init() {
	endpos = data_buffer + BUFFER_SIZE;
	pos = data_buffer;
	count_basic = 0;

	pthread_mutex_init(&lock, NULL);
}

void get_basic(struct basic data) {
	data.process_id = getpid();
	//data.pthread = pthread_self();
	//data.pthread = gettid();

	// call gettid() as syscall because there is no implementation in glibc
	data.thread_id = iotrace_gettid();
}

void printData() {
	struct basic data;
	int ret;
	char buf[max_size_basic() + 1]; /* +1 for trailing null character */
	pos = data_buffer;

	for (int i = 0; i < count_basic; i++) {
		data = *((struct basic *)((void *)pos));

		ret = print_basic(buf, sizeof(buf), data);
		printf("%s\n",buf);
		printf("json-length:   %d\n",ret);
		ret = sizeof_basic(data);
		printf("buffer-length: %d\n",ret);

		pos += ret;
	}
	pos = data_buffer;
	count_basic = 0;
}

void writeData(struct basic data) {
	int length = sizeof_basic(data);

	/* write (synchronized) */
	pthread_mutex_lock(&lock);

	if (pos + length > endpos) {
		printData();
	}
	assert(pos + length <= endpos); /* buffer not big enough for even one struct basic */
	/* ToDo: fail with error-message */

	pos = (void*) copy_basic((void*)pos, data);
	count_basic++;

	pthread_mutex_unlock(&lock);
}

void cleanup() {
	printData();

	pthread_mutex_destroy(&lock);
}
