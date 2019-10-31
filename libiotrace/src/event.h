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

#include "json_include_struct.h"


extern char init_done;
// ToDo: use macro ATTRIBUTE_CONSTRUCTOR
void init_basic()__attribute__((constructor));

void get_basic(struct basic *data);

// ToDo: as macro with return value?
u_int64_t gettime(void);

void writeData(struct basic *data);


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

#endif /* LIBIOTRACE_EVENT_H */
