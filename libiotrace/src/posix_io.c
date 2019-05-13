/**
 * @file Implementation of Posix-IO functions.
 */
#include "libiotrace_config.h"
#ifdef HAVE_STDLIB_H
#  include <stdlib.h>
#endif
#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif

#include <fcntl.h>

#include <string.h>
#include <time.h>
#include "event.h"

// ToDo: __func__ dependencies
#ifdef IO_LIB_STATIC
#  define WRAP(function_name) __wrap_##function_name
#  define POSIX_IO_SET_FUNCTION_NAME(data) strncpy(data, __func__ + 7, MAXFUNCTIONNAME) /* +7 removes beginning __wrap_ from __func__ */
#else
#  define WRAP(function_name) function_name
#  define POSIX_IO_SET_FUNCTION_NAME(data) strncpy(data, __func__, MAXFUNCTIONNAME)
#endif

static void init() ATTRIBUTE_CONSTRUCTOR;

/* Function pointers for glibc functions */
#ifdef IO_LIB_STATIC

/* POSIX byte */
int __real_open (const char *pathname, int flags);
int __real_close (int fd);
/* POSIX stream */
FILE * __real_fopen (const char *filename, const char *mode);
FILE * __real_fdopen (int fd, const char *mode);
FILE * __real_freopen (const char *pathname, const char *mode, FILE *stream);
int __real_fclose (FILE *file);

#else

/* POSIX byte */
static int (*__real_open)(const char *pathname, int flags) = NULL;
static int (*__real_close)(int fd) = NULL;
/* POSIX stream */
static FILE * (*__real_fopen)(const char *filename, const char *mode) = NULL;
static FILE * (*__real_fdopen)(int fd, const char *mode) = NULL;
static FILE * (*__real_freopen)(const char *pathname, const char *mode,
		FILE *stream) = NULL;
static int (*__real_fclose)(FILE *file) = NULL;

/* initialize pointers for glibc functions */
static void init() {
#ifdef _GNU_SOURCE
	__real_open = dlsym(RTLD_NEXT, "open");
	__real_close = dlsym(RTLD_NEXT, "close");
	__real_fopen = dlsym(RTLD_NEXT, "fopen");
	__real_fdopen = dlsym(RTLD_NEXT, "fdopen");
	__real_freopen = dlsym(RTLD_NEXT, "freopen");
	__real_fclose = dlsym(RTLD_NEXT, "fclose");
#endif
}

#endif

enum access_mode get_access_mode(int flags) {
	int access_mode = flags & O_ACCMODE;

	if (access_mode == O_RDONLY) {
		return read_only;
	} else if (access_mode == O_WRONLY) {
		return write_only;
	} else if (access_mode == O_RDWR) {
		return read_and_write;
	} else {
		return unknown;
	}
}

void get_creation_flags(const int flags, struct creation_flags *cf) {
	cf->cloexec = flags & O_CLOEXEC ? 1 : 0;
	cf->creat = flags & O_CREAT ? 1 : 0;
	cf->directory = flags & O_DIRECTORY ? 1 : 0;
	cf->excl = flags & O_EXCL ? 1 : 0;
	cf->noctty = flags & O_NOCTTY ? 1 : 0;
	cf->nofollow = flags & O_NOFOLLOW ? 1 : 0;
	cf->tmpfile = flags & O_TMPFILE ? 1 : 0;
	cf->trunc = flags & O_TRUNC ? 1 : 0;
}

void get_status_flags(const int flags, struct status_flags *sf) {
	sf->append = flags & O_APPEND ? 1 : 0;
	sf->async = flags & O_ASYNC ? 1 : 0;
	sf->direct = flags & O_DIRECT ? 1 : 0;
	sf->dsync = flags & O_DSYNC ? 1 : 0;
	sf->largefile = flags & O_LARGEFILE ? 1 : 0;
	sf->noatime = flags & O_NOATIME ? 1 : 0;
	sf->nonblock = flags & O_NONBLOCK ? 1 : 0;
	sf->ndelay = flags & O_NDELAY ? 1 : 0;
	sf->path = flags & O_PATH ? 1 : 0;
	sf->sync = flags & O_SYNC ? 1 : 0;
}

enum access_mode check_mode(const char *mode, struct creation_flags *cf,
		struct status_flags *sf) {
	cf->directory = 0;
	cf->noctty = 0;
	cf->nofollow = 0;
	cf->tmpfile = 0;
	sf->async = 0;
	sf->direct = 0;
	sf->dsync = 0;
	sf->largefile = 0;
	sf->noatime = 0;
	sf->nonblock = 0;
	sf->ndelay = 0;
	sf->path = 0;
	sf->sync = 0;

	// ToDo: c
	// ToDo: m
	// ToDo: ,ccs=<string>
	// ToDo: largefile from first write/read?
	if (strchr(mode, 'e') != NULL) {
		cf->cloexec = 1;
	} else {
		cf->cloexec = 0;
	}

	if (strchr(mode, 'x') != NULL) {
		cf->excl = 1;
	} else {
		cf->excl = 0;
	}

	if (strchr(mode, 'r') != NULL) {
		cf->creat = 0;
		cf->trunc = 0;
		sf->append = 0;
		if (strchr(mode, '+') == NULL) {
			return read_only;
		} else {
			return read_and_write;
		}
	} else if (strchr(mode, 'w') != NULL) {
		cf->creat = 1;
		cf->trunc = 1;
		sf->append = 0;
		if (strchr(mode, '+') == NULL) {
			return write_only;
		} else {
			return read_and_write;
		}
	} else if (strchr(mode, 'a') != NULL) {
		cf->creat = 1;
		cf->trunc = 0;
		sf->append = 1;
		if (strchr(mode, '+') == NULL) {
			return write_only;
		} else {
			return read_and_write;
		}
	} else {
		cf->creat = 0;
		cf->trunc = 0;
		sf->append = 0;
		return unknown;
	}
}

#ifdef HAVE_OPEN_ELLIPSES
int WRAP(open)(const char *pathname, int flags, ...) {
#else
int WRAP(open)(const char *pathname, int flags, ...) { /* "..." entfernen */
#endif
	int ret;
	struct basic data;
	struct open_function open_data;

#ifdef HAVE_OPEN_ELLIPSES
	// vastart vaend
#else
	// mode_t mode = os_getmode();
#endif

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, open_function, open_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	open_data.file_name = pathname;
	open_data.mode = get_access_mode(flags);
	get_creation_flags(flags, &open_data.creation);
	get_status_flags(flags, &open_data.status);

	// ToDo: is clock correct for evaluation in different threads
	data.time_start = clock();
	ret = __real_open(pathname, flags);
	data.time_end = clock();

	JSON_STRUCT_SET_VOID_P(data, file_type, file_descriptor, ret)

	writeData(&data);

	return ret;
}

int WRAP(close)(int fd) {
	int ret;
	struct basic data;
	struct close_function close_data;

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, close_function, close_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_descriptor, fd)

	data.time_start = clock();
	ret = __real_close(fd);
	data.time_end = clock();

	close_data.return_value = ret;

	writeData(&data);

	return ret;
}

FILE * WRAP(fopen)(const char *filename, const char *mode) {
	FILE * file;
	struct basic data;
	struct open_function open_data;

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, open_function, open_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	open_data.file_name = filename;
	open_data.mode = check_mode(mode, &open_data.creation, &open_data.status);

	data.time_start = clock();
	file = __real_fopen(filename, mode);
	data.time_end = clock();

	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, file)

	writeData(&data);

	return file;
}

FILE * WRAP(fdopen)(int fd, const char *mode) {

}

FILE * WRAP(freopen)(const char *pathname, const char *mode, FILE *stream) {

}

int WRAP(fclose)(FILE *file) {
	int ret;
	struct basic data;
	struct close_function close_data;

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, close_function, close_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, file)

	data.time_start = clock();
	ret = __real_fclose(file);
	data.time_end = clock();

	close_data.return_value = ret;

	writeData(&data);

	return ret;
}
