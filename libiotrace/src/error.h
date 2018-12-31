#ifndef LIBIOTRACE_ERROR_H
#define LIBIOTRACE_ERROR_H

#ifdef HAVE_ERRNO_H
#  include "errno.h"
#endif


/* Mapping of error values; 0 is no error. */
#define LIBIOTRACE_EINVAL   1
#define LIBIOTRACE_ENOMEM   2
#define LIBIOTRACE_ENOSPC   3

#endif /* LIBIOTRACE_ERROR_H */
