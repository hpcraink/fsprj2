/**
 * Library to ease OpenCL Programming
 *
 * Copyright (c) 2018     Hochschule Esslingen, University of Applied Science
 *
 */

#ifndef LIBIOTRACE_H
#define LIBIOTRACE_H

#include "libiotrace_config.h"

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
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
 * Print....
 *
 * @param[in] string          The string to print
 *  
 * @return the error-status
 */
int libiotrace_printf(char * string);

END_C_DECLS

#endif /* HFTOPENCL_H */
