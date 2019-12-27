/**
 * Library to control libiotrace function wrappers
 *
 * Copyright (c) 2018     Hochschule Esslingen, University of Applied Science
 *
 */

#ifndef LIBIOTRACE_H
#define LIBIOTRACE_H

#include "libiotrace_config.h"

#ifdef HAVE_SYS_TYPES_H
#  include <sys/types.h>
#endif

#define LIBIOTRACE_LIBRARY_VERSION "libIOtrace " # LIBIOTRACE_VERSION_MAJOR # "." # LIBIOTRACE_VERSION_MINOR

#ifndef FATAL_ERROR
#define FATAL_ERROR(func,errno) do { \
    __real_fprintf(stderr, "(FILE:%s:%d) func:%s with errno:%d\n", \
                   __FILE__, __LINE__, (func), (errno)); \
    exit(errno); \
} while(0)
#endif

BEGIN_C_DECLS

/*********************** DATA STRUCTURES ***************************/


/*********************** FUNCTION DEFINITIONS ***************************/

/**
 * Start logging in actual thread.
 */
void libiotrace_start_log();

/**
 * End logging in actual thread.
 */
void libiotrace_end_log();

/**
 * Start logging of stacktrace pointer in actual thread
 * (if logging is active and stacktrace depth is greater than 0).
 */
void libiotrace_start_stacktrace_ptr();

/**
 * End logging of stacktrace pointer in actual thread.
 */
void libiotrace_end_stacktrace_ptr();

/**
 * Start logging of stacktrace symbols in actual thread
 * (if logging is active and stacktrace depth is greater than 0).
 */
void libiotrace_start_stacktrace_symbol();

/**
 * End logging of stacktrace symbols in actual thread.
 */
void libiotrace_end_stacktrace_symbol();

/**
 * Set stacktrace depth for logging in actual thread.
 *
 * @param[in] depth           The stacktrace depth (if set to 0 no stacktrace is logged)
 */
void libiotrace_set_stacktrace_depth(int depth);


/**
 * Get stacktrace depth.
 *
 * @return stacktrace depth
 */
int libiotrace_get_stacktrace_depth();

END_C_DECLS

#endif /* LIBIOTRACE_H */
