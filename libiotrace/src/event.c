#include "libiotrace_config.h"

#include <assert.h>

#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif

#ifdef HAVE_STDLIB_H
#  include <stdlib.h>
#endif

#include <sys/syscall.h>

#ifdef HAVE_PTHREAD_H
#  include <pthread.h>
#endif

#include "event.h"
#include "os.h"

#include "json_include_function.h"
#include "posix_io.h"

/* Buffer */
#define BUFFER_SIZE 500
static char data_buffer[BUFFER_SIZE];
static char* endpos;
static char* pos;
static int count_basic;

/* Mutex */
static pthread_mutex_t lock;

// ToDo: dependencies of __thread?
// ToDo: pid out of TLS, because all threads sharing one pid (threadgroup)
// once per process
static pid_t pid;
static char hostname[HOST_NAME_MAX];
// once per thread
static __thread pid_t tid = 0;

// ToDo: use macro ATTRIBUTE_CONSTRUCTOR
static void init()__attribute__((constructor));
// ToDo: test for destructor in cmake
static void cleanup()__attribute__((destructor));

static void init() {
	endpos = data_buffer + BUFFER_SIZE;
	pos = data_buffer;
	count_basic = 0;

	pid = getpid();
	gethostname(hostname, HOST_NAME_MAX);

	pthread_mutex_init(&lock, NULL);
}

void get_basic(struct basic *data) {
	if (tid == 0) {
		// ToDo: caching pid can be source of bugs, see man getpid()
		tid = iotrace_gettid();
	}
	// ToDo: are namespaces relevant (same pid for different processes in different namespaces)?
	data->process_id = pid;
	data->thread_id = tid;

	// ToDo: are hostnames relevant (multiple nodes)?
	data->hostname = hostname;
}

void printData() {
	struct basic *data;
	int ret;
	char buf[json_struct_max_size_basic() + 1]; /* +1 for trailing null character */
	pos = data_buffer;

	for (int i = 0; i < count_basic; i++) {
		data = (struct basic *) ((void *) pos);

		ret = json_struct_print_basic(buf, sizeof(buf), data);
		printf("%s\n", buf);
		//printf("json-length:   %d\n", ret);
		ret = json_struct_sizeof_basic(data);
		//printf("buffer-length: %d\n", ret);

		pos += ret;
	}
	pos = data_buffer;
	count_basic = 0;
}

void writeData(struct basic *data) {
	int length = json_struct_sizeof_basic(data);

	/* write (synchronized) */
	pthread_mutex_lock(&lock);

	if (pos + length > endpos) {
		printData();
	}
	if (pos + length > endpos) {
		__real_fprintf(stderr,
				"In function %s: buffer (%ld bytes) not big enough for even one struct basic (%d bytes).\n",
				__func__, sizeof(data_buffer), length);
		assert(0);
	}

	pos = (void*) json_struct_copy_basic((void*) pos, data);
	count_basic++;

	pthread_mutex_unlock(&lock);
}

void cleanup() {
	printData();

	pthread_mutex_destroy(&lock);
}
