/**
 * Functions utilizing `ptrace`(2)
 */
#ifndef PTRACE_UTILS_H
#define PTRACE_UTILS_H

#include <unistd.h>
#include "arch/ptrace_utils.h"


/* -- Function prototypes -- */
int ptrace_get_regs_content(pid_t tid, struct user_regs_struct_full *regs);

/**
 * @brief                           Reads string in address space of corresponding process
 *
 * @param[in] tid                   Process, from which address space the string shall be read
 * @param[in] addr                  Address in the process'es address space
 * @param[in] bytes_to_read         # of bytes to read; -1 = unknown (i.e., until NUL terminator has been reached)
 * @param[out] read_str_ptr_ptr     Will contain pointer to read string, which has been `malloc`'ed
 *                                  IMPORTANT: MUST BE `free`'ed
 *
 * @return                          Length of read string -1 (NUL byte)
 */
size_t ptrace_read_string(pid_t tid, unsigned long addr,
                          ssize_t bytes_to_read,
                          char** read_str_ptr_ptr);

#endif /* PTRACE_UTILS_H */
