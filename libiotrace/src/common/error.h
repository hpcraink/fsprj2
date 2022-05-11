#ifndef LIBIOTRACE_ERROR_H
#define LIBIOTRACE_ERROR_H

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "../wrapper_defines.h"

#ifdef WITH_POSIX_IO
#  include "../posix_io.h"
#endif

//#ifdef HAVE_ERRNO_H
//#  include "errno.h"
//#endif


#define __LIB_NAME "libiotrace"


/* Mapping of error values; 0 is no error. */
//#define LIBIOTRACE_EINVAL   1
//#define LIBIOTRACE_ENOMEM   2
//#define LIBIOTRACE_ENOSPC   3


#ifndef NDEBUG
#  define LOG_DEBUG(format, ...)                                                                                                                                           \
	do {                                                                                                                                                                          \
		CALL_REAL_POSIX_SYNC(fprintf)(stdout, "<<"__LIB_NAME">> [DEBUG] `%s` (%s:%d): " format "." LINE_BREAK, __func__, __FILE__, __LINE__, ##__VA_ARGS__); \
	} while(0)
#else
#  define LOG_DEBUG(format, ...) do {  } while(0)
#endif


#define LOG_WARN(format, ...)                                                                                                                                             \
	do {                                                                                                                                                                         \
		CALL_REAL_POSIX_SYNC(fprintf)(stderr, "<<"__LIB_NAME">> [WARN] `%s` (%s:%d): " format "." LINE_BREAK, __func__, __FILE__, __LINE__, ##__VA_ARGS__); \
	} while(0)


// TODO: GCC -fmacro-prefix-map=old=new to shorten path in __FILE__
// ToDo: __func__ dependencies (like in posix_io.c)
#define LOG_ERROR_AND_EXIT(format, ...) \
	do {                                                                                                                                                                          \
		CALL_REAL_POSIX_SYNC(fprintf)(stderr, "<<"__LIB_NAME">> [ERROR] `%s` (%s:%d): " format "." LINE_BREAK, __func__, __FILE__, __LINE__, ##__VA_ARGS__); \
		exit(EXIT_FAILURE);                                                                                                                                                       \
	} while(0)


#define DIE_WHEN_ERRNO(FUNC) __extension__({ ({                                   \
    int __val = (FUNC);                                                           \
    (-1 == __val ? ({ LOG_ERROR_AND_EXIT("%s", strerror(errno)); -1; }) : __val);   \
  }); })

#define DIE_WHEN_ERRNO_VPTR(FUNC) __extension__({ ({                                  \
    void* __val = (FUNC);                                                             \
    (NULL == __val ? ({ LOG_ERROR_AND_EXIT("%s", strerror(errno)); NULL; }) : __val); \
  }); })

#endif /* LIBIOTRACE_ERROR_H */
