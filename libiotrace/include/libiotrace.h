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

BEGIN_C_DECLS

/*********************** DATA STRUCTURES ***************************/


/*********************** FUNCTION DEFINITIONS ***************************/

/**
 * Start logging in actual thread.
 */
void libiotrace_start_log(void);

/**
 * End logging in actual thread.
 */
void libiotrace_end_log(void);

/**
 * Start sending log in actual thread.
 */
void libiotrace_start_send(void);

/**
 * End sending log in actual thread.
 */
void libiotrace_end_send(void);

/**
 * Start logging of stacktrace pointer in actual thread
 * (if logging is active and stacktrace depth is greater than 0).
 */
void libiotrace_start_stacktrace_ptr(void);

/**
 * End logging of stacktrace pointer in actual thread.
 */
void libiotrace_end_stacktrace_ptr(void);

/**
 * Start logging of stacktrace symbols in actual thread
 * (if logging is active and stacktrace depth is greater than 0).
 */
void libiotrace_start_stacktrace_symbol(void);

/**
 * End logging of stacktrace symbols in actual thread.
 */
void libiotrace_end_stacktrace_symbol(void);

/**
 * Set stacktrace depth for logging in actual thread.
 *
 * @param[in] depth The stacktrace depth (if set to 0 no stacktrace is logged)
 */
void libiotrace_set_stacktrace_depth(int depth);


/**
 * Get stacktrace depth.
 *
 * @return stacktrace depth
 */
int libiotrace_get_stacktrace_depth(void);

/**
 * Set wrapper for a function with name equals "wrapper" active.
 *
 * An active wrapper logs/sends collected data.
 * Status of a wrapper (active/inactive) is saved per process.
 * Changing the status affects all threads in the process.
 *
 * @param[in] wrapper "\0" terminated name of the wrapped function.
 */
void libiotrace_set_wrapper_active(const char *wrapper);

/**
 * Set wrapper for a function with name equals "wrapper" inactive.
 *
 * An inactive wrapper doesn't logs/sends collected data.
 * Status of a wrapper (active/inactive) is saved per process.
 * Changing the status affects all threads in the process.
 *
 * @param[in] wrapper "\0" terminated name of the wrapped function.
 */
void libiotrace_set_wrapper_inactive(const char *wrapper);

END_C_DECLS

#endif /* LIBIOTRACE_H */
