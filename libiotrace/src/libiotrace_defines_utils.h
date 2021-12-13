#ifndef LIBIOTRACE_DEFINES_UTILS_H
#define LIBIOTRACE_DEFINES_UTILS_H

#include <stddef.h>

size_t libiotrace_struct_print_cstring(char* libiotrace_struct_buf, size_t libiotrace_struct_size, const char* libiotrace_struct_src);
size_t libiotrace_struct_write(char* libiotrace_struct_buf, size_t libiotrace_struct_size, const char* libiotrace_struct_src);

#endif /* LIBIOTRACE_DEFINES_UTILS_H */
