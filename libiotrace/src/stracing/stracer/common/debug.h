/*
 * Derived version from libiotrace which DOESN'T USE any libiotrace facilities (such as macros)
 */
#ifndef STRACER_DEBUG_H_
#define STRACER_DEBUG_H_

#ifdef DEV_DEBUG_ENABLE_LOGS
#  include "error.h"
#  define DEV_DEBUG_PRINT_MSG(format, ...) LOG_DEBUG(format, ##__VA_ARGS__)
#else
#  define DEV_DEBUG_PRINT_MSG(format, ...) do {  } while(0)
#endif

#endif /* STRACER_DEBUG_H_ */
