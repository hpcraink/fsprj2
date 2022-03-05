#ifndef LIBIOTRACE_ERROR_H
#define LIBIOTRACE_ERROR_H

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "wrapper_defines.h"

#ifdef WITH_POSIX_IO
#  include "posix_io.h"
#endif

//#ifdef HAVE_ERRNO_H
//#  include "errno.h"
//#endif


#define __LIB_NAME "libiotrace"


/* Mapping of error values; 0 is no error. */
//#define LIBIOTRACE_EINVAL   1
//#define LIBIOTRACE_ENOMEM   2
//#define LIBIOTRACE_ENOSPC   3


#if !defined(NDEBUG)
#  define LIBIOTRACE_DEBUG(format, ...)                                                                                                                                           \
	do {                                                                                                                                                                          \
		CALL_REAL_POSIX_SYNC(fprintf)(stdout, "<<"__LIB_NAME">> [DEBUG] In function %s (file %s, line %d): " format "." LINE_BREAK, __func__, __FILE__, __LINE__, ##__VA_ARGS__); \
	} while(0)
#else
#  define LIBIOTRACE_DEBUG(format, ...) do {  } while(0)
#endif


#define LIBIOTRACE_WARN(format, ...)                                                                                                                                             \
	do {                                                                                                                                                                         \
		CALL_REAL_POSIX_SYNC(fprintf)(stderr, "<<"__LIB_NAME">> [WARN] In function %s (file %s, line %d): " format "." LINE_BREAK, __func__, __FILE__, __LINE__, ##__VA_ARGS__); \
	} while(0)


// TODO: GCC -fmacro-prefix-map=old=new to shorten path in __FILE__
// ToDo: __func__ dependencies (like in posix_io.c)
#define LIBIOTRACE_ERROR(format, ...) \
	do {                                                                                                                                                                          \
		CALL_REAL_POSIX_SYNC(fprintf)(stderr, "<<"__LIB_NAME">> [ERROR] In function %s (file %s, line %d): " format "." LINE_BREAK, __func__, __FILE__, __LINE__, ##__VA_ARGS__); \
		exit(EXIT_FAILURE);                                                                                                                                                       \
	} while(0)


#define DIE_WHEN_ERRNO(FUNC) __extension__({ ({                                   \
    int __val = (FUNC);                                                           \
    (-1 == __val ? ({ LIBIOTRACE_ERROR("%s", strerror(errno)); -1; }) : __val);   \
  }); })

#endif /* LIBIOTRACE_ERROR_H */
