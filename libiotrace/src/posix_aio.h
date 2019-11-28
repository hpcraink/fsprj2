#ifndef LIBIOTRACE_POSIX_AIO_H
#define LIBIOTRACE_POSIX_AIO_H

#include "libiotrace_config.h"

#include <aio.h>
#include "wrapper_defines.h"

BEGIN_C_DECLS

/* Function pointers for glibc functions */

REAL_TYPE int REAL(aio_read)(struct aiocb *aiocbp) REAL_INIT;
#ifdef HAVE_AIO_READ64
REAL_TYPE int REAL(aio_read64)(struct aiocb64 *aiocbp) REAL_INIT;
#endif
REAL_TYPE int REAL(aio_write)(struct aiocb *aiocbp) REAL_INIT;
#ifdef HAVE_AIO_WRITE64
REAL_TYPE int REAL(aio_write64)(struct aiocb64 *aiocbp) REAL_INIT;
#endif
REAL_TYPE int REAL(lio_listio)(int mode, struct aiocb *const list[], int nent, struct sigevent *sig) REAL_INIT;
#ifdef HAVE_AIO_LIO_LISTIO64
REAL_TYPE int REAL(lio_listio64)(int mode, struct aiocb64 *const list[], int nent, struct sigevent *sig) REAL_INIT;
#endif
REAL_TYPE int REAL(aio_error)(const struct aiocb *aiocbp) REAL_INIT;
#ifdef HAVE_AIO_ERROR64
REAL_TYPE int REAL(aio_error64)(const struct aiocb64 *aiocbp) REAL_INIT;
#endif
REAL_TYPE ssize_t REAL(aio_return)(struct aiocb *aiocbp) REAL_INIT;
#ifdef HAVE_AIO_RETURN64
REAL_TYPE ssize_t REAL(aio_return64)(struct aiocb64 *aiocbp) REAL_INIT;
#endif
REAL_TYPE int REAL(aio_fsync)(int op, struct aiocb *aiocbp) REAL_INIT;
#ifdef HAVE_AIO_FSYNC64
REAL_TYPE int REAL(aio_fsync64)(int op, struct aiocb64 *aiocbp) REAL_INIT;
#endif
REAL_TYPE int REAL(aio_suspend)(const struct aiocb *const list[], int nent, const struct timespec *timeout) REAL_INIT;
#ifdef HAVE_AIO_SUSPEND64
REAL_TYPE int REAL(aio_suspend64)(const struct aiocb64 *const list[], int nent, const struct timespec *timeout) REAL_INIT;
#endif
REAL_TYPE int REAL(aio_cancel)(int fildes, struct aiocb *aiocbp) REAL_INIT;
#ifdef HAVE_AIO_CANCEL64
REAL_TYPE int REAL(aio_cancel64)(int fildes, struct aiocb64 *aiocbp) REAL_INIT;
#endif
#ifdef HAVE_AIO_INIT
REAL_TYPE void REAL(aio_init)(const struct aioinit *init) REAL_INIT;
#endif
REAL_TYPE int REAL(shm_open)(const char *name, int oflag, mode_t mode) REAL_INIT;
//ToDo: io_submit

#ifndef IO_LIB_STATIC
void posix_aio_init() ATTRIBUTE_CONSTRUCTOR;
#endif

END_C_DECLS

#endif /* LIBIOTRACE_POSIX_AIO_H */
