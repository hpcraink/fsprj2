#ifndef LIBIOTRACE_POSIX_AIO_H
#define LIBIOTRACE_POSIX_AIO_H

#include "libiotrace_config.h"

#include <aio.h>
#include "wrapper_defines.h"

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

#ifndef IO_LIB_STATIC
#undef DLSYM_INIT_DONE
#undef DLSYM_INIT_FUNCTION
#define DLSYM_INIT_DONE posix_aio_init_done
#define DLSYM_INIT_FUNCTION posix_aio_init
static char DLSYM_INIT_DONE = 0;
static void DLSYM_INIT_FUNCTION() ATTRIBUTE_CONSTRUCTOR;
/* Initialize pointers for glibc functions.
 * This has to be in the header file because other files use the "__real_" functions
 * instead of the normal posix functions (e.g. see event.c or json_defines.h). */
static void DLSYM_INIT_FUNCTION() {
	if (!DLSYM_INIT_DONE) {

		DLSYM(aio_read);
#ifdef HAVE_AIO_READ64
		DLSYM(aio_read64);
#endif
		DLSYM(aio_write);
#ifdef HAVE_AIO_WRITE64
		DLSYM(aio_write64);
#endif
		DLSYM(lio_listio);
#ifdef HAVE_AIO_LIO_LISTIO64
		DLSYM(lio_listio64);
#endif
		DLSYM(aio_error);
#ifdef HAVE_AIO_ERROR64
		DLSYM(aio_error64);
#endif
		DLSYM(aio_return);
#ifdef HAVE_AIO_RETURN64
		DLSYM(aio_return64);
#endif
		DLSYM(aio_fsync);
#ifdef HAVE_AIO_FSYNC64
		DLSYM(aio_fsync64);
#endif
		DLSYM(aio_suspend);
#ifdef HAVE_AIO_SUSPEND64
		DLSYM(aio_suspend64);
#endif
		DLSYM(aio_cancel);
#ifdef HAVE_AIO_CANCEL64
		DLSYM(aio_cancel64);
#endif
#ifdef HAVE_AIO_INIT
		DLSYM(aio_init);
#endif

		DLSYM_INIT_DONE = 1;
	}
}
#endif

#endif /* LIBIOTRACE_POSIX_AIO_H */
