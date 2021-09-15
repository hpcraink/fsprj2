#ifndef LIBIOTRACE_LOGGING_H
#define LIBIOTRACE_LOGGING_H


#include "../../error.h"

#define __LIB_NAME "libiotrace"
#define __MODULE_NAME "fnres"


#if !defined(NDEBUG)
#  include <stdio.h>
#  define LOG_DEBUG(fmt, ...) printf("<<"__LIB_NAME">> [DEBUG] "__MODULE_NAME" - "fmt"\n", ##__VA_ARGS__);
#else
#  define LOG_DEBUG(fmt, ...)
#endif

#define LOG_WARN(fmt, ...) LIBIOTRACE_WARN("<<"__LIB_NAME">> [WARN] "__MODULE_NAME " - "fmt, ##__VA_ARGS__);

#define LOG_ERROR_AND_EXIT(fmt, ...) LIBIOTRACE_ERROR("<<"__LIB_NAME">> [ERROR] "__MODULE_NAME " - "fmt, ##__VA_ARGS__);


#endif /* LIBIOTRACE_LOGGING_H */