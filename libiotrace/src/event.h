#ifndef LIBIOTRACE_EVENT_H
#define LIBIOTRACE_EVENT_H

#include <stdio.h>
#include <string.h>
#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif
#include "libiotrace_config.h"

#ifdef WITH_POSIX_IO
#  include "posix_io.h"
#endif
#ifdef WITH_POSIX_AIO
#  include "posix_aio.h"
#endif
#ifdef WITH_DL_IO
#  include "dl_io.h"
#endif
#ifdef WITH_MPI_IO
#  include "mpi_io.h"
#endif
#ifdef WITH_ALLOC
#  include "alloc.h"
#endif

#include "libiotrace_include_struct.h"
#include "wrapper_defines.h"
#include "io_log_file.h"

BEGIN_C_DECLS

extern struct wrapper_status active_wrapper_status;

extern char init_done;
void init_process(void);

void get_basic(struct basic *data);

void get_file_id(int fd, struct file_id *data);
void get_file_id_by_path(const char *filename, struct file_id *data);

#ifdef IOTRACE_ENABLE_LOGFILE
void write_into_buffer(struct basic *data);
#endif
#ifdef IOTRACE_ENABLE_INFLUXDB
void write_into_influxdb(struct basic *data);
#endif
void free_memory(struct basic *data);

/* All exec-functions replace the current process image without calling __attribute__((destructor)).
 * So the data_buffer is not successfully cleaned. Instead all remaining data is overwritten and lost.
 * Thats the reason for the following wrapper functions. */
REAL_TYPE int REAL(execve)(const char *filename, char *const argv[], char *const envp[]) REAL_INIT;
REAL_TYPE int REAL(execv)(const char *path, char *const argv[]) REAL_INIT;
REAL_TYPE int REAL(execl)(const char *path, const char *arg, ... /* (char  *) NULL */) REAL_INIT;
REAL_TYPE int REAL(execvp)(const char *file, char *const argv[]) REAL_INIT;
REAL_TYPE int REAL(execlp)(const char *file, const char *arg, ... /* (char  *) NULL */) REAL_INIT;
#ifdef HAVE_EXECVPE
REAL_TYPE int REAL(execvpe)(const char *file, char *const argv[], char *const envp[]) REAL_INIT;
#endif
REAL_TYPE int REAL(execle)(const char *path, const char *arg, ... /*, (char *) NULL, char * const envp[] */) REAL_INIT;

/* __attribute__((destructor)) functions are not every time called before _exit is called.
 * So the following wrappers are necessary. */
REAL_TYPE void REAL(_exit)(int status) REAL_INIT;
#ifdef HAVE_EXIT
REAL_TYPE void REAL(_Exit)(int status) REAL_INIT;
#endif
#ifdef HAVE_EXIT_GROUP
REAL_TYPE void REAL(exit_group)(int status) REAL_INIT;
#endif

REAL_TYPE int REAL(pthread_create)(pthread_t *restrict thread,
const pthread_attr_t *restrict attr,
void *(*start_routine)(void *),
void *restrict arg) REAL_INIT;

END_C_DECLS

#endif /* LIBIOTRACE_EVENT_H */
