#ifndef LIBIOTRACE_INCLUDE_FUNCTION_H
#define LIBIOTRACE_INCLUDE_FUNCTION_H

#include <stddef.h>

#include "libiotrace_include_struct.h"

size_t libiotrace_struct_max_size_basic(void);
int libiotrace_struct_print_basic(char* buf, size_t size, struct basic *data);
int libiotrace_struct_sizeof_basic(struct basic *libiotrace_struct_data);
void* libiotrace_struct_copy_basic(void *libiotrace_struct_buf, struct basic *libiotrace_struct_data);
void libiotrace_struct_free_basic(struct basic *libiotrace_struct_data);

size_t libiotrace_struct_push_max_size_basic(size_t prefix_length);
size_t libiotrace_struct_push_basic(char* libiotrace_struct_buffer_to_post, size_t libiotrace_struct_length_of_buffer_to_post, struct basic *libiotrace_struct_data, const char* prefix);

#if defined(IOTRACE_ENABLE_INFLUXDB) && defined(ENABLE_REMOTE_CONTROL)
size_t libiotrace_struct_push_max_size_influx_meta(size_t prefix_length);
size_t libiotrace_struct_push_influx_meta(char* libiotrace_struct_buffer_to_post, size_t libiotrace_struct_length_of_buffer_to_post, struct influx_meta *libiotrace_struct_data, const char* prefix);
#endif
#if defined(ENABLE_REMOTE_CONTROL) && defined(IOTRACE_ENABLE_LOGFILE)
size_t libiotrace_struct_max_size_control_meta(void);
int libiotrace_struct_print_control_meta(char* buf, size_t size, struct control_meta *data);
#endif

size_t libiotrace_struct_max_size_working_dir(void);
int libiotrace_struct_print_working_dir(char* buf, size_t size, struct working_dir *data);

size_t libiotrace_struct_max_size_wrapper_status(void);
int libiotrace_struct_print_wrapper_status(char* buf, size_t size, struct wrapper_status *data);

size_t libiotrace_struct_max_size_filesystem(void);
int libiotrace_struct_print_filesystem(char* buf, size_t size, struct filesystem *data);

size_t libiotrace_struct_push_max_size_filesystem(size_t prefix_length);
size_t libiotrace_struct_push_filesystem(char* libiotrace_struct_buffer_to_post, size_t libiotrace_struct_length_of_buffer_to_post, struct filesystem *libiotrace_struct_data, const char* prefix);

#endif /* LIBIOTRACE_INCLUDE_FUNCTION_H */
