#include "libiotrace_config.h"

#include <assert.h>
#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif
#ifdef HAVE_STDLIB_H
#  include <stdlib.h>
#endif
#ifdef HAVE_SYS_SYSCALL_H
#  include <sys/syscall.h>
#endif
#ifdef HAVE_PTHREAD_H
#  include <pthread.h>
#endif

#ifdef HAVE_TIME_RDTSC
static volatile inline unsigned long long int getrdtsc() {
    uint64_t x;
#if defined (__i386__)
    __asm__ volatile ("rdtsc\n" : "=A" (x));
#elif defined (__x86_64__)
    uint32_t hi, lo;
    __asm__ volatile ("rdtsc\n" : "=a" (lo), "=d" (hi));
     x = ((uint64_t)hi << 32 | lo);
#else
#  error "Internal error: Instruction only supported on Intel __i386__ and __x86_64__, configure with HAVE_RDTSC"
#endif
    return x;
}
#endif /* HAVE_TIME_RDTSC */

uin64_t gettime(void) {
    uint64_t ret;
#if WANT_TIME_FCT = WANT_TIME_GETTIMEOFDAY
    static struct timeval tv;
    gettimeofday(&tv, NULL);
    ret = tv.tv_sec * 1000ll * 1000ll + tv.tv_usec * 1000ll; 
#elif WANT_TIME_FCT = WANT_TIMECLOCK_GETTIME
#  error "Internal error: not implemented yet"
#elif WANT_TIME_FCT = WANT_TIME_RDTSC
    ret = getrdtsc();
#endif
    return ret;
}

