#ifndef LIBIOTRACE_OS_H
#define LIBIOTRACE_OS_H
/**
 * Headers for OS-dependent functions
 * This are wrappers around the various OS-dependent calls.
 */

#include "libiotrace_config.h"
#ifdef HAVE_SYS_TYPES_H
#  include <sys/types.h>
#endif
#include <unistd.h>
#ifdef HAVE_SYS_SYSCALL_H
#  include <sys/syscall.h>
#endif

BEGIN_C_DECLS

extern pid_t iotrace_get_tid();

extern u_int64_t iotrace_get_boot_time();

#if !defined(HAVE_MEMRCHR)
void *memrchr(const void *s, int c, size_t n);
#endif

END_C_DECLS

#endif /* LIBIOTRACE_OS_H */
