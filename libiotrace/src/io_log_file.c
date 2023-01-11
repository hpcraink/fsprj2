/**
 * Write buffer implementation.
 * Manages one write buffer per process.
 */

#include "libiotrace_config.h"

// TODO: put normal includes here
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#ifdef HAVE_PTHREAD_H
#include <pthread.h>
#endif

#include "io_log_file.h"

#include "common/error.h"

//#include "libiotrace_include_function.h"
#include "wrapper_defines.h"

#include "libiotrace_functions.h"

/* log file write Buffer */
#ifdef IOTRACE_ENABLE_LOGFILE

#ifndef BUFFER_SIZE_BYTES
#  define BUFFER_SIZE_BYTES 1048576 // 1 MB
#endif /* BUFFER_SIZE_BYTES */
static char data_buffer[BUFFER_SIZE_BYTES];
static const char *endpos = data_buffer + BUFFER_SIZE_BYTES;
static char *pos;
static int count_basic;

static pthread_mutex_t lock;

static char log_name[MAXFILENAME];

void io_log_file_buffer_flush(void);

/**
 * Initializes buffer.
 *
 * This function is not thread-safe and destroys an existing buffer.
 * This function should therefore only be called once from one thread.
 **/
void io_log_file_buffer_init_process(const char *logfile_name) {
	pos = data_buffer;
	count_basic = 0;

	strncpy(log_name, logfile_name, MAXFILENAME);
	log_name[MAXFILENAME - 1] = '\0'; // ensure string is terminated

	pthread_mutex_init(&lock, NULL);
}

/**
 * Initializes buffer.
 *
 * This function must be thread-safe and initializes the current buffer
 * (see io_file_buffer_init_process) for the actual thread.
 * This function should therefore be called from each thread.
 **/
void io_log_file_buffer_init_thread(void) {
	return;
}

/**
 * Writes Buffer to file.
 **/
void io_log_file_buffer_clear(void) {
	pthread_mutex_lock(&lock);
	io_log_file_buffer_flush();
	pthread_mutex_unlock(&lock);
}

/**
 * Writes Buffer to file and destroys the buffer.
 *
 * Is called on process exit.
 **/
void io_log_file_buffer_destroy(void) {
	pthread_mutex_lock(&lock);
	io_log_file_buffer_flush();
	pthread_mutex_unlock(&lock);

	pthread_mutex_destroy(&lock);
}

/**
 * Prints all structures from buffer to file.
 *
 * Each structure is serialized to json and written to
 * a file. After that the buffer is empty. Serialization
 * and clearing of the buffer is not synchronized with
 * other threads. So "print_buffer" should only be
 * called from a synchronized code.
 */
void io_log_file_buffer_flush(void) {
	struct basic *data;
	int ret;
	int count;
	char buf[libiotrace_struct_max_size_basic() + sizeof(LINE_BREAK)];
	int fd;
	pos = data_buffer;

	fd = CALL_REAL_POSIX_SYNC(open)(log_name, O_WRONLY | O_CREAT | O_APPEND,
	S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
	if (-1 == fd) {
		LOG_ERROR_AND_EXIT("open() of file %s returned %d with errno=%d",
                           log_name, fd, errno);
	}

	for (int i = 0; i < count_basic; i++) {
		data = (struct basic*) ((void*) pos);

		ret = libiotrace_struct_print_basic(buf, sizeof(buf), data); //Function is present at runtime, built with macros from libiotrace_defines.h
		strcpy(buf + ret, LINE_BREAK);
		count = ret + sizeof(LINE_BREAK) - 1;
		ret = CALL_REAL_POSIX_SYNC(write)(fd, buf, count); // TODO: buffer serialized structs and call write less often
		if (0 > ret) {
			LOG_ERROR_AND_EXIT("write() returned %d", ret);
		}
		if (ret < count) {
			LOG_ERROR_AND_EXIT("incomplete write() occurred");
		}
		ret = libiotrace_struct_sizeof_basic(data);

		pos += ret;
	}

	CALL_REAL_POSIX_SYNC(close)(fd);

	pos = data_buffer;
	count_basic = 0;
}

/**
 * Writes a struct basic to the buffer for this process.
 *
 * A deep copy of all values from "data" and all in "data"
 * referenced structures and arrays is synchronized written
 * to the central buffer.
 * If the buffer hasn't enough free space for the deep copy
 * the buffer is cleared with a call to "print_buffer"
 * first.
 *
 * @param[in] data Pointer to struct basic
 */
void io_log_file_buffer_write(struct basic *data) {
#ifdef LOG_WRAPPER_TIME
	static char *old_pos;
#endif

	/* write (synchronized) */
	pthread_mutex_lock(&lock);

	int length = libiotrace_struct_sizeof_basic(data);

	if (pos + length > endpos) {
		io_log_file_buffer_flush();
	}
	if (pos + length > endpos) {
		// ToDo: solve circular dependency of fprintf
		LOG_ERROR_AND_EXIT(
				"buffer (%lu bytes) not big enough for even one struct basic (%d bytes)",
				sizeof(data_buffer), length);
	}

#ifdef LOG_WRAPPER_TIME
	old_pos = pos;
#endif
	pos = (void*) libiotrace_struct_copy_basic((void*) pos, data);
	count_basic++;
	// insert end time for wrapper in buffer
	WRAPPER_TIME_END((*((struct basic *)((void *)old_pos))))

	pthread_mutex_unlock(&lock);
}

#endif
