#ifndef LIBIOTRACE_IO_LOG_FILE_H
#define LIBIOTRACE_IO_LOG_FILE_H

#include "libiotrace_config.h"

#include "libiotrace_include_struct.h"

BEGIN_C_DECLS

void io_log_file_buffer_init_process(const char *logfile_name);
void io_log_file_buffer_init_thread(void);
void io_log_file_buffer_clear(void);
void io_log_file_buffer_destroy(void);
void io_log_file_buffer_write(struct basic *data);

END_C_DECLS

#endif /* LIBIOTRACE_IO_LOG_FILE_H */
