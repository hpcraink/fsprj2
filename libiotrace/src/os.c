#include "libiotrace_config.h"

#ifdef HAVE_SYS_SYSCALL_H
#  include <sys/syscall.h>
#endif

#ifdef HAVE_SYS_TYPES_H
#  include <sys/types.h>
#endif

#ifdef HAVE_UTMP_H
#  include <utmp.h>
#endif
#ifdef HAVE_UTMPX_H
#  include <utmpx.h>
#endif
#include <sys/stat.h>
#include <fcntl.h>

#include "common/error.h"

#include "os.h"

/**
 * Get the Thread ID.
 * @return Returns the thread ID of the calling Thread.
 */
inline pid_t iotrace_get_tid() {
    pid_t tmp = -1;

#ifdef __linux__
    tmp = syscall(SYS_gettid);
#elif defined(__APPLE__) || defined(__OSX__)
    // call gettid() as syscall because there is no implementation in glibc
    tmp = syscall(SYS_thread_selfid);
#else
#   error "iotrace_gettid has not been defined for this OS."
#endif

    return tmp;
}

/**
 * Get the boot time
 * @return Returns the boot time.
 */
u_int64_t iotrace_get_boot_time() {
	u_int64_t boot_time = 0;

#ifdef __linux__
	const char *utmp = "/var/run/utmp";
	int file;
	int entrys = 10;
	int i;
	struct utmp log[entrys];
	ssize_t bytes;
	int offset = 0;

	file = CALL_REAL_POSIX_SYNC(open)(utmp, O_RDONLY);
	if (0 > file) {
		LOG_ERROR_AND_EXIT("open of %s failed, errno=%d", utmp, errno);
	}

	// read until EOF
	do {
		bytes = CALL_REAL_POSIX_SYNC(read)(file, (char *)(&(log)) + offset, sizeof(log) - offset);
		if (0 > bytes) {
			if (EINTR == errno) {
				// read interrupted by signal: try again
				continue;
			} else {
				LOG_ERROR_AND_EXIT("read of %s failed, errno=%d", utmp, errno);
			}
		}

		// check all complete structures for boot_time
		for (i = 0; (bytes - (i * sizeof(struct utmp))) >= sizeof(struct utmp); i++) {
			if (log[i].ut_type == BOOT_TIME) {
				// get boot_time in nano s from s and micro s in ut_tv
				boot_time = (u_int64_t)log[i].ut_tv.tv_sec * 1000000000ll + (u_int64_t)log[i].ut_tv.tv_usec * 1000ll;

				break;
			}
		}

		// if incomplete structure was read: prepare input buffer
		if ((bytes - (i * sizeof(struct utmp))) > 0) {
			offset = (bytes - (i * sizeof(struct utmp)));
			if (i > 0) {
				// copy incomplete structure to begin of buffer
				memcpy(&log, (char *)(&(log)) + (i * sizeof(struct utmp)), offset);
			}
		} else {
			offset = 0;
		}
	} while (0 != bytes); // until EOF

	CALL_REAL_POSIX_SYNC(close)(file);

	if (0 == boot_time) {
		LOG_ERROR_AND_EXIT("boot entry in %s not found", utmp);
	}
#elif defined(__APPLE__) || defined(__OSX__)
	struct utmpx * utxent;
	struct utmpx utx_boottime;
	utx_boottime.ut_type = BOOT_TIME;

	utxent = getutxid(&utx_boottime);
	boot_time = (u_int64_t)utxent.ut_tv.tv_sec * 1000000000ll + (u_int64_t)utxent.ut_tv.tv_usec * 1000ll;
#else
#   error "iotrace_get_boot_time has not been defined for this OS."
#endif

	return boot_time;
}


#if !defined(HAVE_MEMRCHR)
/**
 * Replacement for memrchr function on OS, that don't define this.
 * Searches for character c in a string of length n starting from the right.
 */
void *memrchr(const void *s, int c, size_t n)
{
	char * tmp;

	// In case size_t is *not* unsigned int
	if (n < 0)
		return NULL;
	for (tmp = (char*)s; tmp[n] != c && n > 0; n--) /* Just loop */ ;

	if (0 == n)
		return NULL;
	return &(tmp[n]);
}
#endif
