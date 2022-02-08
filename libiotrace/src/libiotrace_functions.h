#ifndef LIBIOTRACE_INCLUDE_FUNCTION_H
#define LIBIOTRACE_INCLUDE_FUNCTION_H

#include <stddef.h>

#include "libiotrace_include_struct.h"

size_t libiotrace_struct_max_size_basic(void);
int libiotrace_struct_print_basic(char* buf, size_t size, struct basic *data);
int libiotrace_struct_sizeof_basic(struct basic *libiotrace_struct_data);
void* libiotrace_struct_copy_basic(void *libiotrace_struct_buf, struct basic *libiotrace_struct_data);
void libiotrace_struct_free_basic(struct basic *libiotrace_struct_data);

size_t libiotrace_struct_max_size_working_dir(void);
int libiotrace_struct_print_working_dir(char* buf, size_t size, struct working_dir *data);

#endif /* LIBIOTRACE_INCLUDE_FUNCTION_H */
