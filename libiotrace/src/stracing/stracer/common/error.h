/*
 * Derived version from libiotrace which DOESN'T USE any libiotrace facilities (such as macros)
 */
#ifndef STRACER_ERROR_H_
#define STRACER_ERROR_H_

#ifdef LIBIOTRACE_ERROR_H
#  error "Included libiotrace's version of header as well"
#endif

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "compiler.h"


#define __LOG_MODULE_NAME "stracer"


/* - Error handling marcos - */
#ifndef NDEBUG
#  define LOG_DEBUG(FMT, ...) \
  do { \
    fprintf(stdout, "<<"__LOG_MODULE_NAME">> [DEBUG] `%s` (%s:%d): " FMT ".\n", __func__, __FILE__, __LINE__, ##__VA_ARGS__); \
  } while(0)
#else
#  define LOG_DEBUG(FMT, ...) do { } while(0)
#endif /* NDEBUG */

#define LOG_WARN(FMT, ...) \
  do { \
    fprintf(stderr, "<<"__LOG_MODULE_NAME">> [WARN] `%s` (%s:%d): " FMT ".\n", __func__, __FILE__, __LINE__, ##__VA_ARGS__); \
  } while(0)

#define LOG_ERROR_AND_DIE(FMT, ...) \
  do { \
    fprintf(stderr, "<<"__LOG_MODULE_NAME">> [ERROR] `%s` (%s:%d): " FMT ".\n", __func__, __FILE__, __LINE__, ##__VA_ARGS__); \
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


/* -- Debugging macros -- */
#ifdef DEV_DEBUG_ENABLE_LOGS
#  define DEV_DEBUG_PRINT_MSG(FMT, ...) LOG_DEBUG(FMT, ##__VA_ARGS__)
#else
#  define DEV_DEBUG_PRINT_MSG(FMT, ...) do {  } while(0)
#endif

#endif /* STRACER_ERROR_H_ */
