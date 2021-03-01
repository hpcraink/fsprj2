#ifndef LIBIOTRACE_ERROR_H
#define LIBIOTRACE_ERROR_H

#include <stdlib.h>

#include "wrapper_defines.h"

#ifdef WITH_POSIX_IO
#include "posix_io.h"
#endif

//#ifdef HAVE_ERRNO_H
//#  include "errno.h"
//#endif

/* Mapping of error values; 0 is no error. */
//#define LIBIOTRACE_EINVAL   1
//#define LIBIOTRACE_ENOMEM   2
//#define LIBIOTRACE_ENOSPC   3

// TODO: GCC -fmacro-prefix-map=old=new to shorten path in __FILE__
// ToDo: __func__ dependencies (like in posix_io.c)
#define LIBIOTRACE_ERROR(format, ...) \
	do { \
		CALL_REAL_POSIX_SYNC(fprintf)(stderr, "In function %s (file %s, line %d): " format ".\n", __func__, __FILE__, __LINE__, ##__VA_ARGS__); \
		exit(EXIT_FAILURE); \
	} while(0)

#endif /* LIBIOTRACE_ERROR_H */
