/*
 * Derived version from libiotrace (which DOESN'T USE any libiotrace facilities (such as macros))
 */
#ifndef STRACER_UTILS_H_
#define STRACER_UTILS_H_

#ifdef LIBIOTRACE_UTILS_H_
#  error "Included libiotrace's version of header as well"
#endif

#include <stdint.h>

/* -- Function prototypes -- */
/**
 * Parse C string (w/ number in base 10) as signed long
 *
 * @param[in]  str String to be parsed
 * @param[out] num Parsed long
 * @return     0 on success, -1 on failure
 */
int str_to_long(char* str, long* num);

/**
 * Returns the 64 bit time in nanosecond resolution since start of the computer
 *
 * @ret   time in ns
 */
uint64_t gettime(void);

#endif /* STRACER_UTILS_H_ */
