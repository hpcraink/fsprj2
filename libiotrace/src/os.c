#ifdef HAVE_SYS_SYSCALL_H
#  include <sys/syscall.h>
#endif

#ifdef HAVE_SYS_TYPES_H
#  include <sys/types.h>
#endif

#include "os.h"

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
