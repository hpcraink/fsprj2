#include "wrapper_name.h"

WRAPPER_NAME(aio_read)
#ifdef HAVE_AIO_READ64
WRAPPER_NAME(aio_read64)
#endif
WRAPPER_NAME(aio_write)
#ifdef HAVE_AIO_WRITE64
WRAPPER_NAME(aio_write64)
#endif
WRAPPER_NAME(lio_listio)
#ifdef HAVE_AIO_LIO_LISTIO64
WRAPPER_NAME(lio_listio64)
#endif
WRAPPER_NAME(aio_error)
#ifdef HAVE_AIO_ERROR64
WRAPPER_NAME(aio_error64)
#endif
WRAPPER_NAME(aio_return)
#ifdef HAVE_AIO_RETURN64
WRAPPER_NAME(aio_return64)
#endif
WRAPPER_NAME(aio_fsync)
#ifdef HAVE_AIO_FSYNC64
WRAPPER_NAME(aio_fsync64)
#endif
WRAPPER_NAME(aio_suspend)
#ifdef HAVE_AIO_SUSPEND64
WRAPPER_NAME(aio_suspend64)
#endif
WRAPPER_NAME(aio_cancel)
#ifdef HAVE_AIO_CANCEL64
WRAPPER_NAME(aio_cancel64)
#endif
#ifdef HAVE_AIO_INIT
WRAPPER_NAME(aio_init)
#endif
WRAPPER_NAME(shm_open)
