#ifdef HAVE_SYS_SYSCALL_H
#  include <sys/syscall.h>
#endif

#ifdef HAVE_SYS_TYPES_H
#  include <sys/types.h>
#endif

#if defined(__APPLE__) || defined(__OSX__)
#  include <sys/sysctl.h>
#endif

#ifdef _WIN32
#  include <stdlib.h>
#  include <windows.h>
#endif

#ifdef __linux__
#  include <unistd.h>
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

/**
 * Get the cache line size.
 * @return Returns the cache line size or 0 in case of an error.
 */
size_t cache_line_size() {
	size_t line_size = 0;
#if defined(__APPLE__) || defined(__OSX__)
	size_t len = sizeof(line_size);
	sysctlbyname("hw.cachelinesize", &line_size, &len, 0, 0);
#elif defined(_WIN32)
	DWORD buffer_size = 0;
	DWORD i = 0;
	SYSTEM_LOGICAL_PROCESSOR_INFORMATION * buffer = 0;

	GetLogicalProcessorInformation(0, &buffer_size);
	buffer = (SYSTEM_LOGICAL_PROCESSOR_INFORMATION *)malloc(buffer_size);
	GetLogicalProcessorInformation(&buffer[0], &buffer_size);

	for (i = 0; i != buffer_size / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION); ++i) {
	    if (buffer[i].Relationship == RelationCache && buffer[i].Cache.Level == 1) {
	        line_size = buffer[i].Cache.LineSize;
	        break;
	    }
	}

	free(buffer);
#elif defined(__linux__)
	line_size = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);
#else
#   warning "cache_line_size has not been defined for this OS."
#endif
	return line_size;
}
