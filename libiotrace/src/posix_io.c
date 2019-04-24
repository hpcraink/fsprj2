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


// ToDo: other solution for this dirty hack
// include of <bits/fcntl-linux.h> instead of <fcntl.h> because <fcntl.h> has functions who will be wrapped in this file
#include <fcntl.h>
//#ifndef _FCNTL_H
//#define _FCNTL_H
//#include <bits/fcntl-linux.h>
//#undef _FCNTL_H
//#endif

#include <string.h>
#include "event.h"

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
static int (*__real_open) (const char *pathname, int flags) = NULL;
static int (*__real_close) (int fd) = NULL;
/* POSIX stream */
static FILE * (*__real_fopen) (const char *filename, const char *mode) = NULL;
static FILE * (*__real_fdopen) (int fd, const char *mode) = NULL;
static FILE * (*__real_freopen) (const char *pathname, const char *mode, FILE *stream) = NULL;
static int (*__real_fclose) (FILE *file) = NULL;

/* initialize pointers for glibc functions */
static void init( ){
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
	cf->cloexec = flags & O_CLOEXEC;
	cf->creat = flags & O_CREAT;
	cf->directory = flags & O_DIRECTORY;
	cf->excl = flags & O_EXCL;
	cf->noctty = flags & O_NOCTTY;
	cf->nofollow = flags & O_NOFOLLOW;
	cf->tmpfile = flags & O_TMPFILE;
	cf->trunc = flags & O_TRUNC;
}

void get_status_flags(const int flags, struct status_flags *sf) {
	sf->append = flags & O_APPEND;
	sf->async = flags & O_ASYNC;
	sf->direct = flags & O_DIRECT;
	sf->dsync = flags & O_DSYNC;
	sf->largefile = flags & O_LARGEFILE;
	sf->noatime = flags & O_NOATIME;
	sf->nonblock = flags & O_NONBLOCK;
	sf->ndelay = flags & O_NDELAY;
	sf->path = flags & O_PATH;
	sf->sync = flags & O_SYNC;
}

enum access_mode check_mode(const char *mode, struct creation_flags *cf, struct status_flags *sf) {
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

#ifdef IO_LIB_STATIC
int __wrap_open(const char *pathname, int flags) {
#else
	//ToDo: #pragma weak and ...
//#pragma weak __wrap_open = open
#ifdef HAVE_OPEN_ELLIPSES
int open(const char *pathname, int flags, ...)
#else
int open(const char *pathname, int flags, ...) /*... entfernen*/
#endif
{
#endif
	int ret;
	struct basic data;

#ifdef HAVE_OPEN_ELLIPSES
	// vastart vaend
#else
	// mode_t mode = os_getmode();
#endif

	get_basic(data);
	data.func_type = open_function;
	// ToDo: __func__ dependencies
	strncpy(data.function_name, __func__, MAXFUNCTIONNAME);
	data.type = descriptor;
	strncpy(data.open_data.file_name, pathname, MAXFILENAME);
	data.open_data.mode = get_access_mode(flags);
	get_creation_flags(flags, &data.open_data.creation);
	get_status_flags(flags, &data.open_data.status);

	// ToDo: is clock correct for evaluation in different threads
	data.time_start = clock();
	ret = __real_open(pathname, flags);
	data.time_end = clock();

	data.file_descriptor = ret;

	writeData(data);

	return ret;
}

#ifdef IO_LIB_STATIC
int __wrap_close(int fd) {
#else
int close(int fd) {
#endif
	int ret;
	struct basic data;

	get_basic(data);
	data.func_type = close_function;
	strncpy(data.function_name, __func__, MAXFUNCTIONNAME);
	data.type = descriptor;
	data.file_descriptor = fd;

	data.time_start = clock();
	ret = __real_close(fd);
	data.time_end = clock();

	data.close_data.return_value = ret;

	writeData(data);

	return ret;
}

#ifdef IO_LIB_STATIC
FILE * __wrap_fopen(const char *filename, const char *mode) {
#else
FILE * fopen(const char *filename, const char *mode) {
#endif
	FILE * file;
	struct basic data;

	get_basic(data);
	data.func_type = open_function;
	strncpy(data.function_name, __func__, MAXFUNCTIONNAME);
	data.type = stream;
	strncpy(data.open_data.file_name, filename, MAXFILENAME);
	data.open_data.mode = check_mode(mode, &data.open_data.creation, &data.open_data.status);

	data.time_start = clock();
	file = __real_fopen(filename, mode);
	data.time_end = clock();

	data.file_stream = file;

	writeData(data);

	return file;
}

#ifdef IO_LIB_STATIC
FILE *__wrap_fdopen(int fd, const char *mode) {
#else
FILE *fdopen(int fd, const char *mode) {
#endif

}

#ifdef IO_LIB_STATIC
FILE *__wrap_freopen(const char *pathname, const char *mode, FILE *stream) {
#else
FILE *freopen(const char *pathname, const char *mode, FILE *stream) {
#endif

}

#ifdef IO_LIB_STATIC
int __wrap_fclose(FILE *file) {
#else
int fclose(FILE *file) {
#endif
	int ret;
	struct basic data;

	get_basic(data);
	data.func_type = close_function;
	strncpy(data.function_name, __func__, MAXFUNCTIONNAME);
	data.type = stream;
	data.file_stream = file;

	data.time_start = clock();
	ret = __real_fclose(file);
	data.time_end = clock();

	data.close_data.return_value = ret;

	writeData(data);

	return ret;
}
