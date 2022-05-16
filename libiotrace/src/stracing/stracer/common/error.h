/*
 * Derived version from libiotrace which DOESN'T USE any libiotrace facilities (such as macros)
 */
#ifndef STRACER_ERROR_H_
#define STRACER_ERROR_H_

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define __LOG_MODULE_NAME "stracer"


/* - Error handling marcos - */
#ifndef NDEBUG
#  define LOG_DEBUG(format, ...)                                                                         \
  do {                                                                                                   \
    fprintf(stdout, "<<"__LOG_MODULE_NAME">> [DEBUG] `%s` (%s:%d): " format ".\n", __func__, __FILE__, __LINE__, ##__VA_ARGS__); \
  } while(0)
#else
#  define LOG_DEBUG(format, ...) do { } while(0)
#endif /* NDEBUG */

#define LOG_WARN(format, ...)                                                                           \
  do {                                                                                                  \
    fprintf(stderr, "<<"__LOG_MODULE_NAME">> [WARN] `%s` (%s:%d): " format ".\n", __func__, __FILE__, __LINE__, ##__VA_ARGS__); \
  } while(0)

#define LOG_ERROR_AND_EXIT(format, ...)                                                                  \
  do {                                                                                                   \
    fprintf(stderr, "<<"__LOG_MODULE_NAME">> [ERROR] `%s` (%s:%d): " format ".\n", __func__, __FILE__, __LINE__, ##__VA_ARGS__); \
    exit(EXIT_FAILURE);                                                                                  \
  } while(0)


#define DIE_WHEN_ERRNO(FUNC) __extension__({ ({                                   \
    int __val = (FUNC);                                                           \
    (-1 == __val ? ({ LOG_ERROR_AND_EXIT("%s", strerror(errno)); -1; }) : __val); \
  }); })

#define DIE_WHEN_ERRNO_VPTR(FUNC) __extension__({ ({                                  \
    void* __val = (FUNC);                                                             \
    (NULL == __val ? ({ LOG_ERROR_AND_EXIT("%s", strerror(errno)); NULL; }) : __val); \
  }); })


/* -- Debugging macros -- */
#ifdef DEV_DEBUG_ENABLE_LOGS
#  include "error.h"
#  define DEV_DEBUG_PRINT_MSG(format, ...) LOG_DEBUG(format, ##__VA_ARGS__)
#else
#  define DEV_DEBUG_PRINT_MSG(format, ...) do {  } while(0)
#endif

#endif /* STRACER_ERROR_H_ */
