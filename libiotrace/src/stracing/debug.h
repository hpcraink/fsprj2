/*
 * Usage: `#define DEV_DEBUG_ENABLE_LOGS` (or not) BEFORE `#include debug.h`
 */
#ifndef LIBIOTRACE_DEV_DEBUG_H
#define LIBIOTRACE_DEV_DEBUG_H


#ifdef DEV_DEBUG_ENABLE_LOGS
#  include "../error.h"
#  define DEV_DEBUG_PRINT_MSG(format, ...) LIBIOTRACE_DEBUG(format, ##__VA_ARGS__)
#else
#  define DEV_DEBUG_PRINT_MSG(format, ...) do {  } while(0)
#endif


#endif /* LIBIOTRACE_DEV_DEBUG_H */
