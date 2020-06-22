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

extern pid_t iotrace_gettid();

extern size_t cache_line_size();

END_C_DECLS
