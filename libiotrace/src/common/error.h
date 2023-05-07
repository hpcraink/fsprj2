#ifndef LIBIOTRACE_ERROR_H
#define LIBIOTRACE_ERROR_H

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "../wrapper_defines.h"
#include "compiler.h"

#ifdef WITH_POSIX_IO
#  include "../posix_io.h"
#endif

//#ifdef HAVE_ERRNO_H
//#  include "errno.h"
//#endif


#define __LOG_UNIT_NAME "libiotrace"


/* Mapping of error values; 0 is no error. */
//#define LIBIOTRACE_EINVAL   1
//#define LIBIOTRACE_ENOMEM   2
//#define LIBIOTRACE_ENOSPC   3

extern pid_t pid;
extern ATTRIBUTE_THREAD pid_t tid;

#ifndef NDEBUG
#  define LOG_DEBUG(FMT, ...) \
    do { \
        CALL_REAL_POSIX_SYNC(fprintf)(stdout, "<<"__LOG_UNIT_NAME">> [DEBUG][%d][%d] `%s` (%s:%d): " FMT "." LINE_BREAK, pid, tid, __func__, __FILE__, __LINE__, ##__VA_ARGS__); \
    } while(0)
#else
#  define LOG_DEBUG(FMT, ...) do {  } while(0)
#endif


#define LOG_WARN(FMT, ...) \
    do { \
        CALL_REAL_POSIX_SYNC(fprintf)(stderr, "<<"__LOG_UNIT_NAME">> [WARN][%d][%d] `%s` (%s:%d): " FMT "." LINE_BREAK, pid, tid, __func__, __FILE__, __LINE__, ##__VA_ARGS__); \
    } while(0)


// TODO: GCC -fmacro-prefix-map=old=new to shorten path in __FILE__
// ToDo: __func__ dependencies (like in posix_io.c)
#define LOG_ERROR_AND_DIE(FMT, ...) \
    do { \
        CALL_REAL_POSIX_SYNC(fprintf)(stderr, "<<"__LOG_UNIT_NAME">> [ERROR][%d][%d] `%s` (%s:%d): " FMT "." LINE_BREAK, pid, tid, __func__, __FILE__, __LINE__, ##__VA_ARGS__); \
        exit(EXIT_FAILURE); \
    } while(0)


#define DIE_WHEN_ERRNO(FUNC) __extension__({ ({\
    __typeof__(FUNC) __val = (FUNC);\
    (BRANCH_UNLIKELY(-1 == __val) ? ({ LOG_ERROR_AND_DIE("%s", strerror(errno)); -1; }) : __val);\
  }); })

#define DIE_WHEN_ERRNO_VPTR(FUNC) __extension__({ ({\
    __typeof__(FUNC) __val = (FUNC);\
    (BRANCH_UNLIKELY(NULL == __val) ? ({ LOG_ERROR_AND_DIE("%s", strerror(errno)); (__typeof__(FUNC))NULL; }) : __val);\
  }); })



/* -- Debugging -- */
#ifdef DEV_DEBUG_ENABLE_LOGS
#  define DEV_DEBUG_PRINT_MSG(FMT, ...) LOG_DEBUG(FMT, ##__VA_ARGS__)
#else
#  define DEV_DEBUG_PRINT_MSG(FMT, ...) do {  } while(0)
#endif

#endif /* LIBIOTRACE_ERROR_H */
