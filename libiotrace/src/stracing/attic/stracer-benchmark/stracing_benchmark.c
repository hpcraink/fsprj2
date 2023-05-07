/**
 * Simple program for benchmarking the overhead of stracing
 *   - Assumptions:
 *     - 1 tracee (this program)
 *     - No lsep related IPC overhead (i.e., libiotrace is compiled W/O fnres)
 *     - CPU has constant rate TSC (check via `lscpu | grep constant_tsc`)
 *   - NOTE: Output is provided in csv format via stdout
 *
 *
 *  !!  FOR MORE ACCURATE RESULTS, COMMENT OUT "TASK: WARN" in libiotrace/src/stracing/stracer/tasks/task_hooks.c  !!
 */

#define _GNU_SOURCE
#include <sched.h>
#include <assert.h>

#include <sys/syscall.h>
#include <unistd.h>
#include <fcntl.h>

#include <bsd/stdlib.h>

#include <stdio.h>

#include <errno.h>
#include <stdlib.h>
#include <string.h>



/* -- Consts -- */
#define DEFAULT_NR_MEASUREMENTS 5000LU


/* -- Macros -- */
#define LOG_ERROR_AND_DIE(format, ...)                                                                  \
  do {                                                                                                   \
    fprintf(stderr, "[ERROR] `%s` (%s:%d): " format ".\n", __func__, __FILE__, __LINE__, ##__VA_ARGS__); \
    exit(EXIT_FAILURE);                                                                                  \
  } while(0)

#define DIE_WHEN_ERRNO(FUNC) __extension__({ ({                                   \
    int __val = (FUNC);                                                           \
    (-1 == __val ? ({ LOG_ERROR_AND_DIE("%s", strerror(errno)); -1; }) : __val); \
  }); })



/* -- Functions -- */
inline unsigned long long int getrdtscp(int *aux) {
    unsigned long long int x;
    int ecx = -1;
#if defined(__i386__)
    __asm__ __volatile__ ("rdtscp\n\t" : "=A" (x), "=c" (ecx));
#elif defined(__x86_64__)
    unsigned int hi, lo;
    __asm__ __volatile__ ("rdtscp\n\t" : "=a" (lo), "=d" (hi), "=c" (ecx));
    x = ((unsigned long long int)hi << 32 | lo);
#else
#  error "Unsupported architecture"
#endif
    *aux = ecx;
    return x;
}



int main(int argc, char** argv) {
    // Determine # of measurements to be taken
    unsigned long nr_measurements = DEFAULT_NR_MEASUREMENTS;
    if (2 == argc) {
        const char* err_str = NULL;
        nr_measurements = (unsigned long)strtonum(argv[1], 1, 999999, &err_str);
        if (err_str || 0 != nr_measurements % 2) {
            LOG_ERROR_AND_DIE("Couldn't parse number of iterations  (NOTE: must be also an even number)");
        }
    } else if (2 < argc) {
        fprintf(stderr, "Usage: %s [num-measurements]\n", argv[0]);
        return -1;
    }

{
    // Set CPU affinity  (so that we don't have to use `taskset 0x1 <command>`)
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(0, &mask);
    DIE_WHEN_ERRNO( sched_setaffinity(0, sizeof(mask), &mask) );
}


    // Print run info
    fprintf(stderr, "Performing %lu measurements\n", nr_measurements);


    // csv output starts here ...
    fprintf(stdout, "measurement,used glibc wrapper,rdtscp start,rdtscp end,rdtscp delta\n");

    for (unsigned long i = 0; i < nr_measurements; ++i) {
        const unsigned char use_glibc_wrapper = i % 2;


        // Take start time
        int start_core = -1;
        const unsigned long long start_rdtscp = getrdtscp(&start_core);

        // Perform syscall
        int fd = (use_glibc_wrapper) ?
                DIE_WHEN_ERRNO( open("/dev/stdout", O_WRONLY) ) :
                DIE_WHEN_ERRNO( (int)syscall(SYS_open, "/dev/stdout", O_WRONLY) );

        // Take end time
        int end_core = -1;
        const unsigned long long end_rdtscp = getrdtscp(&end_core);


        assert ( (start_core == end_core)  && "Got scheduled on a different core, aborting ..." );

        // Cleanup
        DIE_WHEN_ERRNO( close(fd) );

        // Print csv output to console
        fprintf(stdout, "%lu,%hhu,%llu,%llu,%llu\n",
                i +1,
                use_glibc_wrapper,
                start_rdtscp,
                end_rdtscp,
                end_rdtscp - start_rdtscp);
    }

    fprintf(stderr, "Done\n");

    return 0;
}
