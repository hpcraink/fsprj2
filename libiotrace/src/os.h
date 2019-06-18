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


/**
 * Get the Thread ID.
 * @return Returns the thread ID of the calling Thread.
 */
inline pid_t iotrace_gettid() {
    pid_t tmp = -1;
#ifdef __linux__
    tmp = syscall(SYS_gettid);
#elif defined(__APPLE__) || defined(__OSX__)
    // call gettid() as syscall because there is no implementation in glibc
    tmp = syscall(SYS_thread_selfid);
#else
#   warning "iotrace_gettid has not been defined for this OS."
#endif
    return tmp;
}
