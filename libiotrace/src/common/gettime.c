#include "libiotrace_config.h"

#include <assert.h>
#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif
#ifdef HAVE_STDINT_H
#  include <stdint.h>
#endif
#ifdef HAVE_STDLIB_H
#  include <stdlib.h>
#endif
#ifdef HAVE_SYS_SYSCALL_H
#  include <sys/syscall.h>
#endif
#ifdef HAVE_SYS_TIME_H
#  include <sys/time.h>
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

uint64_t gettime(void) {
    uint64_t ret;
#if defined(WANT_GETTIME)
    struct timespec t;
#  ifdef REALTIME
    clock_gettime(CLOCK_REALTIME, &t);
#  else
    clock_gettime(CLOCK_MONOTONIC_RAW, &t);
#  endif
    ret = (u_int64_t)t.tv_sec * 1000000000ll + (u_int64_t)t.tv_nsec;
#elif defined(WANT_GETTIMEOFDAY)
    static struct timeval tv;
    gettimeofday(&tv, NULL);
    ret = (tv.tv_sec * 1000ll * 1000ll + tv.tv_usec) * 1000ll; 
#elif defined(WANT_RDTSC)
    ret = getrdtsc();
#else
#  error "Internal error: invalid selection for time functionality"
#endif
    return ret;
}

