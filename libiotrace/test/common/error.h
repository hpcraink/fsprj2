/**
 * Macros for error handling in tests
 */
#ifndef TESTING_ERROR_H
#define TESTING_ERROR_H

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* - Error handling marcos - */
#ifndef NDEBUG
#  define LOG_DEBUG(format, ...)                                                                         \
  do {                                                                                                   \
    fprintf(stdout, "[DEBUG] `%s` (%s:%d): " format ".\n", __func__, __FILE__, __LINE__, ##__VA_ARGS__); \
  } while(0)
#else
#  define LOG_DEBUG(format, ...) do { } while(0)
#endif /* NDEBUG */

#define LOG_WARN(format, ...)                                                                           \
  do {                                                                                                  \
    fprintf(stderr, "[WARN] `%s` (%s:%d): " format ".\n", __func__, __FILE__, __LINE__, ##__VA_ARGS__); \
  } while(0)

#define LOG_ERROR_AND_EXIT(format, ...)                                                                  \
  do {                                                                                                   \
    fprintf(stderr, "[ERROR] `%s` (%s:%d): " format ".\n", __func__, __FILE__, __LINE__, ##__VA_ARGS__); \
    exit(EXIT_FAILURE);                                                                                  \
  } while(0)


/**
 * To be used for library functions which make use of `errno` and
 * return an int, where -1 indicates an error condition
 */
#define DIE_WHEN_ERRNO(FUNC) __extension__({ ({                                   \
    int __val = (FUNC);                                                           \
    (-1 == __val ? ({ LOG_ERROR_AND_EXIT("%s", strerror(errno)); -1; }) : __val); \
  }); })

/**
 * To be used for non-library functions which return an int, where
 * -1 indicates an error condition
 */
#define DIE_WHEN_ERR(FUNC) __extension__({ ({                     \
    int __val = (FUNC);                                           \
    (-1 == __val ? ({ LOG_ERROR_AND_EXIT(#FUNC); -1; }) : __val); \
  }); })

/**
 * To be used for library functions which make use of `errno` and
 * return an void pointer, where `NULL` indicates an error condition
 */
#define DIE_WHEN_ERRNO_VPTR(FUNC) __extension__({ ({                                  \
    void* __val = (FUNC);                                                             \
    (NULL == __val ? ({ LOG_ERROR_AND_EXIT("%s", strerror(errno)); NULL; }) : __val); \
  }); })

/**
 * To be used for non-library functions which return an void pointer,
 *  where `NULL` indicates an error condition
 */
#define DIE_WHEN_ERR_VPTR(FUNC, MSG) __extension__({ ({                                  \
    void* __val = (FUNC);                                                             \
    (NULL == __val ? ({ LOG_ERROR_AND_EXIT(MSG); NULL; }) : __val); \
  }); })

#endif /* TESTING_ERROR_H */
