/**
 * @file Implementation of Posix-IO functions.
 */
#include "libiotrace_config.h"
#ifdef HAVE_STDLIB_H
//#  include <stdlib.h>
#endif
#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif

#include <assert.h>
#include <dlfcn.h>
#include <wchar.h>
#include <stdio.h>
#include <stdio_ext.h>
#include <fcntl.h>

#include <string.h>
#include <stdarg.h>
#include <time.h>
#include "json_include_struct.h"
#include "event.h"

#include "posix_io.h"

#define CALL_REAL(function_macro) __CALL_REAL(function_macro)
#define __CALL_REAL(function) __real_##function
#define WRAP(function_macro) __WRAP(function_macro)
#ifdef IO_LIB_STATIC
#  define __WRAP(function_name) __wrap_##function_name
// ToDo: __func__ dependencies
#  define POSIX_IO_SET_FUNCTION_NAME(data) strncpy(data, __func__ + 7, MAXFUNCTIONNAME) /* +7 removes beginning __wrap_ from __func__ */
#else
#  define __WRAP(function_name) function_name
#  define POSIX_IO_SET_FUNCTION_NAME(data) strncpy(data, __func__, MAXFUNCTIONNAME)
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
		return unknown_access_mode;
	}
}

enum lock_mode get_lock_mode(int type) {
	switch (type) {
	case FSETLOCKING_INTERNAL:
		return internal;
	case FSETLOCKING_BYCALLER:
		return bycaller;
	case FSETLOCKING_QUERY:
		return query_lock_mode;
	default:
		return unknown_lock_mode;
	}
}

enum lock_mode get_orientation_mode(int mode, char param) {
	if (mode > 0) {
		return wide;
	} else if (mode < 0) {
		return narrow;
	} else if (param) {
		return query_orientation_mode;
	} else {
		return not_set;
	}
}

enum read_write_state get_return_state_c(int ret) {
	if (ret == EOF) {
		return eof;
	} else {
		return ok;
	}
}

enum read_write_state get_return_state_wc(wint_t ret) {
	if (ret == WEOF) {
		return eof;
	} else {
		return ok;
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
		return unknown_access_mode;
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

	// ToDo: is clock_gettime correct for evaluation in different threads
	// ToDo: dependencies for clock_gettime
	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_start);
	ret = CALL_REAL(open)(pathname, flags);
	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_end);

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

	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_start);
	ret = CALL_REAL(close)(fd);
	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_end);

	close_data.return_value = ret;

	writeData(&data);

	return ret;
}

ssize_t WRAP(read)(int fd, void *buf, size_t count) {
	ssize_t ret;
	struct basic data;
	struct read_function read_data;

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, read_function, read_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_descriptor, fd)

	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_start);
	ret = CALL_REAL(read)(fd, buf, count);
	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_end);

	if (ret == -1) {
		read_data.return_state = error;
		read_data.read_bytes = 0;
	} else if (ret == 0 && count != 0) {
		read_data.return_state = eof;
		read_data.read_bytes = 0;
	} else {
		read_data.return_state = ok;
		read_data.read_bytes = ret;
	}

	writeData(&data);

	return ret;
}

FILE * WRAP(fopen)(const char *filename, const char *opentype) {
	FILE * file;
	struct basic data;
	struct open_function open_data;

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, open_function, open_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	open_data.file_name = filename;
	open_data.mode = check_mode(opentype, &open_data.creation,
			&open_data.status);

	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_start);
	file = CALL_REAL(fopen)(filename, opentype);
	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_end);

	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, file)

	writeData(&data);

	return file;
}

FILE * WRAP(fopen64)(const char *filename, const char *opentype) {
	FILE * file;
	struct basic data;
	struct open_function open_data;

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, open_function, open_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	open_data.file_name = filename;
	open_data.mode = check_mode(opentype, &open_data.creation,
			&open_data.status);

	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_start);
	file = CALL_REAL(fopen64)(filename, opentype);
	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_end);

	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, file)

	writeData(&data);

	return file;
}

FILE * WRAP(freopen)(const char *filename, const char *opentype, FILE *stream) {
	FILE * file;
	struct basic data;
	struct open_function open_data;

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, open_function, open_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	open_data.file_name = filename;
	open_data.mode = check_mode(opentype, &open_data.creation,
			&open_data.status);

	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_start);
	file = CALL_REAL(freopen)(filename, opentype, stream);
	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_end);

	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, file)

	writeData(&data);

	return file;
}

FILE * WRAP(freopen64)(const char *filename, const char *opentype, FILE *stream) {
	FILE * file;
	struct basic data;
	struct open_function open_data;

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, open_function, open_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	open_data.file_name = filename;
	open_data.mode = check_mode(opentype, &open_data.creation,
			&open_data.status);

	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_start);
	file = CALL_REAL(freopen64)(filename, opentype, stream);
	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_end);

	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, file)

	writeData(&data);

	return file;
}

FILE * WRAP(fdopen)(int fd, const char *opentype) {
	FILE * file;
	struct basic data;
	struct fdopen_function fdopen_data;

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, fdopen_function, fdopen_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	fdopen_data.descriptor = fd;
	fdopen_data.mode = check_mode(opentype, &fdopen_data.creation,
			&fdopen_data.status);

	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_start);
	file = CALL_REAL(fdopen)(fd, opentype);
	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_end);

	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, file)

	writeData(&data);

	return file;
}

int WRAP(fclose)(FILE *stream) {
	int ret;
	struct basic data;
	struct close_function close_data;

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, close_function, close_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_start);
	ret = CALL_REAL(fclose)(stream);
	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_end);

	close_data.return_value = ret;

	writeData(&data);

	return ret;
}

int WRAP(fcloseall)(void) {
	int ret;
	struct basic data;
	struct close_function close_data;
	FILE *file = NULL;

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, close_function, close_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, file)

	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_start);
	ret = CALL_REAL(fcloseall)();
	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_end);

	close_data.return_value = ret;

	writeData(&data);

	return ret;
}

void WRAP(flockfile)(FILE *stream) {
	struct basic data;
	struct lock_function lock_data;

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, lock_function, lock_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_start);
	CALL_REAL(flockfile)(stream);
	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_end);

	writeData(&data);

	return;
}

int WRAP(ftrylockfile)(FILE *stream) {
	int ret;
	struct basic data;
	struct trylock_function trylock_data;

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, trylock_function, trylock_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_start);
	ret = CALL_REAL(ftrylockfile)(stream);
	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_end);

	trylock_data.return_value = ret;

	writeData(&data);

	return ret;
}

void WRAP(funlockfile)(FILE *stream) {
	struct basic data;
	struct lock_function lock_data;

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, lock_function, lock_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_start);
	CALL_REAL(funlockfile)(stream);
	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_end);

	writeData(&data);

	return;
}

int WRAP(fwide)(FILE *stream, int mode) {
	int ret;
	struct basic data;
	struct orientation_mode_function orientation_mode_data;

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, orientation_mode_function,
			orientation_mode_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	orientation_mode_data.set_mode = get_orientation_mode(mode, 1);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_start);
	ret = CALL_REAL(fwide)(stream, mode);
	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_end);

	orientation_mode_data.return_mode = get_orientation_mode(ret, 0);

	writeData(&data);

	return ret;
}

int WRAP(fputc)(int c, FILE *stream) {
	int ret;
	struct basic data;
	struct write_function write_data;

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, write_function, write_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_start);
	ret = CALL_REAL(fputc)(c, stream);
	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_end);

	write_data.return_state = get_return_state_c(ret);
	if (write_data.return_state == ok) {
		write_data.written_bytes = 1;
	} else {
		write_data.written_bytes = 0;
	}

	writeData(&data);

	return ret;
}

wint_t WRAP(fputwc)(wchar_t wc, FILE *stream) {
	wint_t ret;
	struct basic data;
	struct write_function write_data;

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, write_function, write_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_start);
	ret = CALL_REAL(fputwc)(wc, stream);
	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_end);

	write_data.return_state = get_return_state_wc(ret);
	if (write_data.return_state == ok) {
		write_data.written_bytes = sizeof(wchar_t);
	} else {
		write_data.written_bytes = 0;
	}

	writeData(&data);

	return ret;
}

int WRAP(fputc_unlocked)(int c, FILE *stream) {
	int ret;
	struct basic data;
	struct write_function write_data;

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, write_function, write_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_start);
	ret = CALL_REAL(fputc_unlocked)(c, stream);
	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_end);

	write_data.return_state = get_return_state_c(ret);
	if (write_data.return_state == ok) {
		write_data.written_bytes = 1;
	} else {
		write_data.written_bytes = 0;
	}

	writeData(&data);

	return ret;
}

wint_t WRAP(fputwc_unlocked)(wchar_t wc, FILE *stream) {
	wint_t ret;
	struct basic data;
	struct write_function write_data;

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, write_function, write_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_start);
	ret = CALL_REAL(fputwc_unlocked)(wc, stream);
	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_end);

	write_data.return_state = get_return_state_wc(ret);
	if (write_data.return_state == ok) {
		write_data.written_bytes = sizeof(wchar_t);
	} else {
		write_data.written_bytes = 0;
	}

	writeData(&data);

	return ret;
}

int WRAP(putc_MACRO)(int c, FILE *stream) {
	int ret;
	struct basic data;
	struct write_function write_data;

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, write_function, write_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_start);
	ret = CALL_REAL(putc_MACRO)(c, stream);
	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_end);

	write_data.return_state = get_return_state_c(ret);
	if (write_data.return_state == ok) {
		write_data.written_bytes = 1;
	} else {
		write_data.written_bytes = 0;
	}

	writeData(&data);

	return ret;
}

wint_t WRAP(putwc_MACRO)(wchar_t wc, FILE *stream) {
	wint_t ret;
	struct basic data;
	struct write_function write_data;

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, write_function, write_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_start);
	ret = CALL_REAL(putwc_MACRO)(wc, stream);
	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_end);

	write_data.return_state = get_return_state_wc(ret);
	if (write_data.return_state == ok) {
		write_data.written_bytes = sizeof(wchar_t);
	} else {
		write_data.written_bytes = 0;
	}

	writeData(&data);

	return ret;
}

int WRAP(putc_unlocked_MACRO)(int c, FILE *stream) {
	int ret;
	struct basic data;
	struct write_function write_data;

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, write_function, write_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_start);
	ret = CALL_REAL(putc_unlocked_MACRO)(c, stream);
	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_end);

	write_data.return_state = get_return_state_c(ret);
	if (write_data.return_state == ok) {
		write_data.written_bytes = 1;
	} else {
		write_data.written_bytes = 0;
	}

	writeData(&data);

	return ret;
}

wint_t WRAP(putwc_unlocked_MACRO)(wchar_t wc, FILE *stream) {
	wint_t ret;
	struct basic data;
	struct write_function write_data;

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, write_function, write_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_start);
	ret = CALL_REAL(putwc_unlocked_MACRO)(wc, stream);
	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_end);

	write_data.return_state = get_return_state_wc(ret);
	if (write_data.return_state == ok) {
		write_data.written_bytes = sizeof(wchar_t);
	} else {
		write_data.written_bytes = 0;
	}

	writeData(&data);

	return ret;
}

int WRAP(fputs)(const char *s, FILE *stream) {
	int ret;
	struct basic data;
	struct write_function write_data;

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, write_function, write_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_start);
	ret = CALL_REAL(fputs)(s, stream);
	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_end);

	write_data.return_state = get_return_state_c(ret);
	if (write_data.return_state == ok) {
		write_data.written_bytes = strlen(s);
	} else {
		write_data.written_bytes = 0;
	}

	writeData(&data);

	return ret;
}

int WRAP(fputws)(const wchar_t *ws, FILE *stream) {
	int ret;
	struct basic data;
	struct write_function write_data;

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, write_function, write_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_start);
	ret = CALL_REAL(fputws)(ws, stream);
	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_end);

	// ToDo: wchar.h says WEOF is error, man pages says -1 is error, what if WEOF isn't -1 ??? ???
	write_data.return_state = get_return_state_wc(ret);
	if (write_data.return_state == ok) {
		write_data.written_bytes = wcslen(ws) * sizeof(wchar_t);
	} else {
		write_data.written_bytes = 0;
	}

	writeData(&data);

	return ret;
}

int WRAP(fputs_unlocked)(const char *s, FILE *stream) {
	int ret;
	struct basic data;
	struct write_function write_data;

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, write_function, write_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_start);
	ret = CALL_REAL(fputs_unlocked)(s, stream);
	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_end);

	write_data.return_state = get_return_state_c(ret);
	if (write_data.return_state == ok) {
		write_data.written_bytes = strlen(s);
	} else {
		write_data.written_bytes = 0;
	}

	writeData(&data);

	return ret;
}

int WRAP(fputws_unlocked)(const wchar_t *ws, FILE *stream) {
	int ret;
	struct basic data;
	struct write_function write_data;

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, write_function, write_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_start);
	ret = CALL_REAL(fputws_unlocked)(ws, stream);
	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_end);

	// ToDo: wchar.h says WEOF is error, man pages says -1 is error, what if WEOF isn't -1 ???
	write_data.return_state = get_return_state_wc(ret);
	if (write_data.return_state == ok) {
		write_data.written_bytes = wcslen(ws) * sizeof(wchar_t);
	} else {
		write_data.written_bytes = 0;
	}

	writeData(&data);

	return ret;
}

int WRAP(putw)(int w, FILE *stream) {
	int ret;
	struct basic data;
	struct write_function write_data;

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, write_function, write_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_start);
	ret = CALL_REAL(putw)(w, stream);
	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_end);

	// ToDo: behavior as described in man pages because header file says nothing about errors
	write_data.return_state = get_return_state_c(ret);
	if (write_data.return_state == ok) {
		write_data.written_bytes = sizeof(int);
	} else {
		write_data.written_bytes = 0;
	}

	writeData(&data);

	return ret;
}

int WRAP(fgetc)(FILE *stream) {
	int ret;
	struct basic data;
	struct read_function read_data;

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, read_function, read_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_start);
	ret = CALL_REAL(fgetc)(stream);
	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_end);

	read_data.return_state = get_return_state_c(ret);
	if (read_data.return_state == ok) {
		read_data.read_bytes = 1;
	} else {
		read_data.read_bytes = 0;
	}

	writeData(&data);

	return ret;
}

wint_t WRAP(fgetwc)(FILE *stream) {
	wint_t ret;
	struct basic data;
	struct read_function read_data;

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, read_function, read_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_start);
	ret = CALL_REAL(fgetwc)(stream);
	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_end);

	read_data.return_state = get_return_state_wc(ret);
	if (read_data.return_state == ok) {
		read_data.read_bytes = sizeof(wchar_t);
	} else {
		read_data.read_bytes = 0;
	}

	writeData(&data);

	return ret;
}

int WRAP(fgetc_unlocked)(FILE *stream) {
	int ret;
	struct basic data;
	struct read_function read_data;

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, read_function, read_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_start);
	ret = CALL_REAL(fgetc_unlocked)(stream);
	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_end);

	read_data.return_state = get_return_state_c(ret);
	if (read_data.return_state == ok) {
		read_data.read_bytes = 1;
	} else {
		read_data.read_bytes = 0;
	}

	writeData(&data);

	return ret;
}

wint_t WRAP(fgetwc_unlocked)(FILE *stream) {
	wint_t ret;
	struct basic data;
	struct read_function read_data;

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, read_function, read_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_start);
	ret = CALL_REAL(fgetwc_unlocked)(stream);
	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_end);

	read_data.return_state = get_return_state_wc(ret);
	if (read_data.return_state == ok) {
		read_data.read_bytes = sizeof(wchar_t);
	} else {
		read_data.read_bytes = 0;
	}

	writeData(&data);

	return ret;
}

int WRAP(getc_MACRO)(FILE *stream) {
	int ret;
	struct basic data;
	struct read_function read_data;

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, read_function, read_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_start);
	ret = CALL_REAL(getc_MACRO)(stream);
	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_end);

	read_data.return_state = get_return_state_c(ret);
	if (read_data.return_state == ok) {
		read_data.read_bytes = 1;
	} else {
		read_data.read_bytes = 0;
	}

	writeData(&data);

	return ret;
}

wint_t WRAP(getwc_MACRO)(FILE *stream) {
	wint_t ret;
	struct basic data;
	struct read_function read_data;

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, read_function, read_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_start);
	ret = CALL_REAL(getwc_MACRO)(stream);
	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_end);

	read_data.return_state = get_return_state_wc(ret);
	if (read_data.return_state == ok) {
		read_data.read_bytes = sizeof(wchar_t);
	} else {
		read_data.read_bytes = 0;
	}

	writeData(&data);

	return ret;
}

int WRAP(getc_unlocked_MACRO)(FILE *stream) {
	int ret;
	struct basic data;
	struct read_function read_data;

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, read_function, read_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_start);
	ret = CALL_REAL(getc_unlocked_MACRO)(stream);
	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_end);

	read_data.return_state = get_return_state_c(ret);
	if (read_data.return_state == ok) {
		read_data.read_bytes = 1;
	} else {
		read_data.read_bytes = 0;
	}

	writeData(&data);

	return ret;
}

wint_t WRAP(getwc_unlocked_MACRO)(FILE *stream) {
	wint_t ret;
	struct basic data;
	struct read_function read_data;

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, read_function, read_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_start);
	ret = CALL_REAL(getwc_unlocked_MACRO)(stream);
	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_end);

	read_data.return_state = get_return_state_wc(ret);
	if (read_data.return_state == ok) {
		read_data.read_bytes = sizeof(wchar_t);
	} else {
		read_data.read_bytes = 0;
	}

	writeData(&data);

	return ret;
}

int WRAP(getw)(FILE *stream) {
	int ret;
	struct basic data;
	struct read_function read_data;

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, read_function, read_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_start);
	ret = CALL_REAL(getw)(stream);
	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_end);

	read_data.return_state = get_return_state_c(ret);
	if (read_data.return_state == ok) {
		read_data.read_bytes = sizeof(int);
	} else {
		read_data.read_bytes = 0;
	}

	writeData(&data);

	return ret;
}

ssize_t WRAP(getline)(char **lineptr, size_t *n, FILE *stream) {
	ssize_t ret;
	struct basic data;
	struct read_function read_data;

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, read_function, read_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_start);
	ret = CALL_REAL(getline)(lineptr, n, stream);
	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_end);

	if (ret == -1) {
		read_data.return_state = eof;
		read_data.read_bytes = 0;
	} else {
		read_data.return_state = ok;
		read_data.read_bytes = ret;
	}

	writeData(&data);

	return ret;
}

ssize_t WRAP(getdelim)(char **lineptr, size_t *n, int delimiter, FILE *stream) {
	ssize_t ret;
	struct basic data;
	struct read_function read_data;

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, read_function, read_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_start);
	ret = CALL_REAL(getdelim)(lineptr, n, delimiter, stream);
	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_end);

	if (ret == -1) {
		read_data.return_state = eof;
		read_data.read_bytes = 0;
	} else {
		read_data.return_state = ok;
		read_data.read_bytes = ret;
	}

	writeData(&data);

	return ret;
}

char * WRAP(fgets)(char *s, int count, FILE *stream) {
	char * ret;
	struct basic data;
	struct read_function read_data;

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, read_function, read_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_start);
	ret = CALL_REAL(fgets)(s, count, stream);
	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_end);

	if (NULL == ret) {
		read_data.return_state = eof;
		read_data.read_bytes = 0;
	} else {
		read_data.return_state = ok;
		read_data.read_bytes = strlen(s);
	}

	writeData(&data);

	return ret;
}

wchar_t * WRAP(fgetws)(wchar_t *ws, int count, FILE *stream) {
	wchar_t * ret;
	struct basic data;
	struct read_function read_data;

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, read_function, read_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_start);
	ret = CALL_REAL(fgetws)(ws, count, stream);
	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_end);

	if (NULL == ret) {
		read_data.return_state = eof;
		read_data.read_bytes = 0;
	} else {
		read_data.return_state = ok;
		read_data.read_bytes = wcslen(ws) * sizeof(wchar_t);
	}

	writeData(&data);

	return ret;
}

char * WRAP(fgets_unlocked)(char *s, int count, FILE *stream) {
	char * ret;
	struct basic data;
	struct read_function read_data;

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, read_function, read_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_start);
	ret = CALL_REAL(fgets_unlocked)(s, count, stream);
	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_end);

	if (NULL == ret) {
		read_data.return_state = eof;
		read_data.read_bytes = 0;
	} else {
		read_data.return_state = ok;
		read_data.read_bytes = strlen(s);
	}

	writeData(&data);

	return ret;
}

wchar_t * WRAP(fgetws_unlocked)(wchar_t *ws, int count, FILE *stream) {
	wchar_t * ret;
	struct basic data;
	struct read_function read_data;

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, read_function, read_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_start);
	ret = CALL_REAL(fgetws_unlocked)(ws, count, stream);
	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_end);

	if (NULL == ret) {
		read_data.return_state = eof;
		read_data.read_bytes = 0;
	} else {
		read_data.return_state = ok;
		read_data.read_bytes = wcslen(ws) * sizeof(wchar_t);
	}

	writeData(&data);

	return ret;
}

int WRAP(ungetc)(int c, FILE *stream) {
	int ret;
	struct basic data;
	struct unget_function unget_data;

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, unget_function, unget_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_start);
	ret = CALL_REAL(ungetc)(c, stream);
	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_end);

	unget_data.return_state = get_return_state_c(ret);
	if (unget_data.return_state == ok) {
		unget_data.buffer_bytes = -1;
	} else {
		unget_data.buffer_bytes = 0;
	}

	writeData(&data);

	return ret;
}

wint_t WRAP(ungetwc)(wint_t wc, FILE *stream) {
	wint_t ret;
	struct basic data;
	struct unget_function unget_data;

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, unget_function, unget_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_start);
	ret = CALL_REAL(ungetwc)(wc, stream);
	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_end);

	unget_data.return_state = get_return_state_wc(ret);
	if (unget_data.return_state == ok) {
		unget_data.buffer_bytes = -1;
	} else {
		unget_data.buffer_bytes = 0;
	}

	writeData(&data);

	return ret;
}

size_t WRAP(fread)(void *data, size_t size, size_t count, FILE *stream) {
	size_t ret;
	struct basic _data;
	struct read_function read_data;

	get_basic(&_data);
	JSON_STRUCT_SET_VOID_P(_data, function_data, read_function, read_data)
	POSIX_IO_SET_FUNCTION_NAME(_data.function_name);
	JSON_STRUCT_SET_VOID_P(_data, file_type, file_stream, stream)

	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &_data.time_start);
	ret = CALL_REAL(fread)(data, size, count, stream);
	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &_data.time_end);

	if (ret == 0) {
		read_data.return_state = eof;
		read_data.read_bytes = 0;
	} else {
		read_data.return_state = ok;
		read_data.read_bytes = ret * size;
	}

	writeData(&_data);

	return ret;
}

size_t WRAP(fread_unlocked)(void *data, size_t size, size_t count, FILE *stream) {
	size_t ret;
	struct basic _data;
	struct read_function read_data;

	get_basic(&_data);
	JSON_STRUCT_SET_VOID_P(_data, function_data, read_function, read_data)
	POSIX_IO_SET_FUNCTION_NAME(_data.function_name);
	JSON_STRUCT_SET_VOID_P(_data, file_type, file_stream, stream)

	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &_data.time_start);
	ret = CALL_REAL(fread_unlocked)(data, size, count, stream);
	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &_data.time_end);

	if (ret == 0) {
		read_data.return_state = eof;
		read_data.read_bytes = 0;
	} else {
		read_data.return_state = ok;
		read_data.read_bytes = ret * size;
	}

	writeData(&_data);

	return ret;
}

size_t WRAP(fwrite)(const void *data, size_t size, size_t count, FILE *stream) {
	size_t ret;
	struct basic _data;
	struct write_function write_data;

	get_basic(&_data);
	JSON_STRUCT_SET_VOID_P(_data, function_data, write_function, write_data)
	POSIX_IO_SET_FUNCTION_NAME(_data.function_name);
	JSON_STRUCT_SET_VOID_P(_data, file_type, file_stream, stream)

	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &_data.time_start);
	ret = CALL_REAL(fwrite)(data, size, count, stream);
	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &_data.time_end);

	if (ret != count) {
		write_data.return_state = error;
		write_data.written_bytes = 0;
	} else {
		write_data.return_state = ok;
		write_data.written_bytes = ret * size;
	}

	writeData(&_data);

	return ret;
}

size_t WRAP(fwrite_unlocked)(const void *data, size_t size, size_t count,
		FILE *stream) {
	size_t ret;
	struct basic _data;
	struct write_function write_data;

	get_basic(&_data);
	JSON_STRUCT_SET_VOID_P(_data, function_data, write_function, write_data)
	POSIX_IO_SET_FUNCTION_NAME(_data.function_name);
	JSON_STRUCT_SET_VOID_P(_data, file_type, file_stream, stream)

	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &_data.time_start);
	ret = CALL_REAL(fwrite_unlocked)(data, size, count, stream);
	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &_data.time_end);

	if (ret != count) {
		write_data.return_state = error;
		write_data.written_bytes = 0;
	} else {
		write_data.return_state = ok;
		write_data.written_bytes = ret * size;
	}

	writeData(&_data);

	return ret;
}

int WRAP(fprintf)(FILE *stream, const char *template, ...) {
	int ret;
	va_list ap;
	struct basic data;
	struct write_function write_data;

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, write_function, write_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	va_start(ap, template);
	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_start);
	ret = CALL_REAL(vfprintf)(stream, template, ap);
	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_end);
	va_end(ap);

	if (ret < 0) {
		write_data.return_state = error;
		write_data.written_bytes = 0;
	} else {
		write_data.return_state = ok;
		write_data.written_bytes = ret;
	}

	writeData(&data);

	return ret;
}

int WRAP(fwprintf)(FILE *stream, const wchar_t *template, ...) {
	int ret;
	va_list ap;
	struct basic data;
	struct write_function write_data;

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, write_function, write_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	va_start(ap, template);
	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_start);
	ret = CALL_REAL(vfwprintf)(stream, template, ap);
	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_end);
	va_end(ap);

	if (ret < 0) {
		write_data.return_state = error;
		write_data.written_bytes = 0;
	} else {
		write_data.return_state = ok;
		write_data.written_bytes = ret * sizeof(wchar_t);
	}

	writeData(&data);

	return ret;
}

int WRAP(vfprintf)(FILE *stream, const char *template, va_list ap) {
	int ret;
	struct basic data;
	struct write_function write_data;

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, write_function, write_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_start);
	ret = CALL_REAL(vfprintf)(stream, template, ap);
	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_end);

	if (ret < 0) {
		write_data.return_state = error;
		write_data.written_bytes = 0;
	} else {
		write_data.return_state = ok;
		write_data.written_bytes = ret;
	}

	writeData(&data);

	return ret;
}

int WRAP(vfwprintf)(FILE *stream, const wchar_t *template, va_list ap) {
	int ret;
	struct basic data;
	struct write_function write_data;

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, write_function, write_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_start);
	ret = CALL_REAL(vfwprintf)(stream, template, ap);
	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_end);

	if (ret < 0) {
		write_data.return_state = error;
		write_data.written_bytes = 0;
	} else {
		write_data.return_state = ok;
		write_data.written_bytes = ret * sizeof(wchar_t);
	}

	writeData(&data);

	return ret;
}

int WRAP(fscanf)(FILE *stream, const char *template, ...) {
	int ret;
	va_list ap;
	struct basic data;
	struct read_function read_data;

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, read_function, read_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	va_start(ap, template);
	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_start);
	ret = CALL_REAL(vfscanf)(stream, template, ap);
	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_end);
	va_end(ap);

	read_data.return_state = get_return_state_c(ret);
	if (read_data.return_state == ok) {
		read_data.read_bytes = 0; // ToDo: read file pointer for bytes count
	} else {
		read_data.read_bytes = 0;
	}

	writeData(&data);

	return ret;
}

int WRAP(fwscanf)(FILE *stream, const wchar_t *template, ...) {
	int ret;
	va_list ap;
	struct basic data;
	struct read_function read_data;

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, read_function, read_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	va_start(ap, template);
	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_start);
	ret = CALL_REAL(vfwscanf)(stream, template, ap);
	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_end);
	va_end(ap);

	read_data.return_state = get_return_state_wc(ret);
	if (read_data.return_state == ok) {
		read_data.read_bytes = 0; // ToDo: read file pointer for bytes count
	} else {
		read_data.read_bytes = 0;
	}

	writeData(&data);

	return ret;
}

int WRAP(vfscanf)(FILE *stream, const char *template, va_list ap) {
	int ret;
	struct basic data;
	struct read_function read_data;

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, read_function, read_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_start);
	ret = CALL_REAL(vfscanf)(stream, template, ap);
	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_end);

	read_data.return_state = get_return_state_c(ret);
	if (read_data.return_state == ok) {
		read_data.read_bytes = 0; // ToDo: read file pointer for bytes count
	} else {
		read_data.read_bytes = 0;
	}

	writeData(&data);

	return ret;
}

int WRAP(vfwscanf)(FILE *stream, const wchar_t *template, va_list ap) {
	int ret;
	struct basic data;
	struct read_function read_data;

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, read_function, read_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_start);
	ret = CALL_REAL(vfwscanf)(stream, template, ap);
	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_end);

	read_data.return_state = get_return_state_wc(ret);
	if (read_data.return_state == ok) {
		read_data.read_bytes = 0; // ToDo: read file pointer for bytes count
	} else {
		read_data.read_bytes = 0;
	}

	writeData(&data);

	return ret;
}

int WRAP(feof)(FILE *stream) {
	int ret;
	struct basic data;
	struct information_function information_data;

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, information_function,
			information_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_start);
	ret = CALL_REAL(feof)(stream);
	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_end);

	information_data.return_value = ret;

	writeData(&data);

	return ret;
}

int WRAP(feof_unlocked)(FILE *stream) {
	int ret;
	struct basic data;
	struct information_function information_data;

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, information_function,
			information_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_start);
	ret = CALL_REAL(feof_unlocked)(stream);
	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_end);

	information_data.return_value = ret;

	writeData(&data);

	return ret;
}

int WRAP(ferror)(FILE *stream) {
	int ret;
	struct basic data;
	struct information_function information_data;

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, information_function,
			information_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_start);
	ret = CALL_REAL(ferror)(stream);
	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_end);

	information_data.return_value = ret;

	writeData(&data);

	return ret;
}

int WRAP(ferror_unlocked)(FILE *stream) {
	int ret;
	struct basic data;
	struct information_function information_data;

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, information_function,
			information_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_start);
	ret = CALL_REAL(ferror_unlocked)(stream);
	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_end);

	information_data.return_value = ret;

	writeData(&data);

	return ret;
}

void WRAP(clearerr)(FILE *stream) {
	struct basic data;
	struct clearerr_function clearerr_data;

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, clearerr_function,
			clearerr_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_start);
	CALL_REAL(clearerr)(stream);
	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_end);

	writeData(&data);

	return;
}

void WRAP(clearerr_unlocked)(FILE *stream) {
	struct basic data;
	struct clearerr_function clearerr_data;

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, clearerr_function,
			clearerr_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_start);
	CALL_REAL(clearerr_unlocked)(stream);
	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_end);

	writeData(&data);

	return;
}

int WRAP(__freadable)(FILE *stream) {
	int ret;
	struct basic data;
	struct information_function information_data;

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, information_function,
			information_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_start);
	ret = CALL_REAL(__freadable)(stream);
	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_end);

	information_data.return_value = ret;

	writeData(&data);

	return ret;
}

int WRAP(__fwritable)(FILE *stream) {
	int ret;
	struct basic data;
	struct information_function information_data;

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, information_function,
			information_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_start);
	ret = CALL_REAL(__fwritable)(stream);
	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_end);

	information_data.return_value = ret;

	writeData(&data);

	return ret;
}

int WRAP(__freading)(FILE *stream) {
	int ret;
	struct basic data;
	struct information_function information_data;

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, information_function,
			information_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_start);
	ret = CALL_REAL(__freading)(stream);
	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_end);

	information_data.return_value = ret;

	writeData(&data);

	return ret;
}

int WRAP(__fwriting)(FILE *stream) {
	int ret;
	struct basic data;
	struct information_function information_data;

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, information_function,
			information_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_start);
	ret = CALL_REAL(__fwriting)(stream);
	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_end);

	information_data.return_value = ret;

	writeData(&data);

	return ret;
}

int WRAP(__fsetlocking)(FILE *stream, int type) {
	int ret;
	struct basic data;
	struct lock_mode_function lock_mode_data;

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, lock_mode_function,
			lock_mode_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	lock_mode_data.set_mode = get_lock_mode(type);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, stream)

	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_start);
	ret = CALL_REAL(__fsetlocking)(stream, type);
	clock_gettime(CLOCK_MONOTONIC_RAW, (void *) &data.time_end);

	lock_mode_data.return_mode = get_lock_mode(ret);

	writeData(&data);

	return ret;
}
