/**
 * @file Implementation of asynchronous Posix-IO functions.
 */
#include "libiotrace_config.h"

#include <aio.h>
#include <fcntl.h>

#include "json_include_struct.h"
#include "event.h"
#include "wrapper_defines.h"
#include "posix_aio.h"

#ifndef IO_LIB_STATIC
REAL_DEFINITION_TYPE int REAL_DEFINITION(aio_read)(struct aiocb *aiocbp) REAL_DEFINITION_INIT;
#ifdef HAVE_AIO_READ64
REAL_DEFINITION_TYPE int REAL_DEFINITION(aio_read64)(struct aiocb64 *aiocbp) REAL_DEFINITION_INIT;
#endif
REAL_DEFINITION_TYPE int REAL_DEFINITION(aio_write)(struct aiocb *aiocbp) REAL_DEFINITION_INIT;
#ifdef HAVE_AIO_WRITE64
REAL_DEFINITION_TYPE int REAL_DEFINITION(aio_write64)(struct aiocb64 *aiocbp) REAL_DEFINITION_INIT;
#endif
REAL_DEFINITION_TYPE int REAL_DEFINITION(lio_listio)(int mode, struct aiocb *const list[], int nent, struct sigevent *sig) REAL_DEFINITION_INIT;
#ifdef HAVE_AIO_LIO_LISTIO64
REAL_DEFINITION_TYPE int REAL_DEFINITION(lio_listio64)(int mode, struct aiocb64 *const list[], int nent, struct sigevent *sig) REAL_DEFINITION_INIT;
#endif
REAL_DEFINITION_TYPE int REAL_DEFINITION(aio_error)(const struct aiocb *aiocbp) REAL_DEFINITION_INIT;
#ifdef HAVE_AIO_ERROR64
REAL_DEFINITION_TYPE int REAL_DEFINITION(aio_error64)(const struct aiocb64 *aiocbp) REAL_DEFINITION_INIT;
#endif
REAL_DEFINITION_TYPE ssize_t REAL_DEFINITION(aio_return)(struct aiocb *aiocbp) REAL_DEFINITION_INIT;
#ifdef HAVE_AIO_RETURN64
REAL_DEFINITION_TYPE ssize_t REAL_DEFINITION(aio_return64)(struct aiocb64 *aiocbp) REAL_DEFINITION_INIT;
#endif
REAL_DEFINITION_TYPE int REAL_DEFINITION(aio_fsync)(int op, struct aiocb *aiocbp) REAL_DEFINITION_INIT;
#ifdef HAVE_AIO_FSYNC64
REAL_DEFINITION_TYPE int REAL_DEFINITION(aio_fsync64)(int op, struct aiocb64 *aiocbp) REAL_DEFINITION_INIT;
#endif
REAL_DEFINITION_TYPE int REAL_DEFINITION(aio_suspend)(const struct aiocb *const list[], int nent, const struct timespec *timeout) REAL_DEFINITION_INIT;
#ifdef HAVE_AIO_SUSPEND64
REAL_DEFINITION_TYPE int REAL_DEFINITION(aio_suspend64)(const struct aiocb64 *const list[], int nent, const struct timespec *timeout) REAL_DEFINITION_INIT;
#endif
REAL_DEFINITION_TYPE int REAL_DEFINITION(aio_cancel)(int fildes, struct aiocb *aiocbp) REAL_DEFINITION_INIT;
#ifdef HAVE_AIO_CANCEL64
REAL_DEFINITION_TYPE int REAL_DEFINITION(aio_cancel64)(int fildes, struct aiocb64 *aiocbp) REAL_DEFINITION_INIT;
#endif
#ifdef HAVE_AIO_INIT
REAL_DEFINITION_TYPE void REAL_DEFINITION(aio_init)(const struct aioinit *init) REAL_DEFINITION_INIT;
#endif
REAL_DEFINITION_TYPE int REAL_DEFINITION(shm_open)(const char *name, int oflag, mode_t mode) REAL_DEFINITION_INIT;
#endif

char libio_aio_read = WRAPPER_ACTIVE;
char libio_aio_read64 = WRAPPER_ACTIVE;
char libio_aio_write = WRAPPER_ACTIVE;
char libio_aio_write64 = WRAPPER_ACTIVE;
char libio_lio_listio = WRAPPER_ACTIVE;
char libio_lio_listio64 = WRAPPER_ACTIVE;
char libio_aio_error = WRAPPER_ACTIVE;
char libio_aio_error64 = WRAPPER_ACTIVE;
char libio_aio_return = WRAPPER_ACTIVE;
char libio_aio_return64 = WRAPPER_ACTIVE;
char libio_aio_fsync = WRAPPER_ACTIVE;
char libio_aio_fsync64 = WRAPPER_ACTIVE;
char libio_aio_suspend = WRAPPER_ACTIVE;
char libio_aio_suspend64 = WRAPPER_ACTIVE;
char libio_aio_cancel = WRAPPER_ACTIVE;
char libio_aio_cancel64 = WRAPPER_ACTIVE;
char libio_aio_init = WRAPPER_ACTIVE;
char libio_shm_open = WRAPPER_ACTIVE;

#ifndef IO_LIB_STATIC
char posix_aio_init_done = 0;
/* Initialize pointers for glibc functions. */
void posix_aio_init() {
	if (!posix_aio_init_done) {

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
		DLSYM(shm_open);

		posix_aio_init_done = 1;
	}
}
#endif

char toggle_posix_aio_wrapper(char *line, toggle)
{
	char ret = 1;

	if (!strcmp(line, "")) {
		ret = 0;
	}
	WRAPPER_ACTIVATE(line, aio_read, toggle)
	WRAPPER_ACTIVATE(line, aio_read64, toggle)
	WRAPPER_ACTIVATE(line, aio_write, toggle)
	WRAPPER_ACTIVATE(line, aio_write64, toggle)
	WRAPPER_ACTIVATE(line, lio_listio, toggle)
	WRAPPER_ACTIVATE(line, lio_listio64, toggle)
	WRAPPER_ACTIVATE(line, aio_error, toggle)
	WRAPPER_ACTIVATE(line, aio_error64, toggle)
	WRAPPER_ACTIVATE(line, aio_return, toggle)
	WRAPPER_ACTIVATE(line, aio_return64, toggle)
	WRAPPER_ACTIVATE(line, aio_fsync, toggle)
	WRAPPER_ACTIVATE(line, aio_fsync64, toggle)
	WRAPPER_ACTIVATE(line, aio_suspend, toggle)
	WRAPPER_ACTIVATE(line, aio_suspend64, toggle)
	WRAPPER_ACTIVATE(line, aio_cancel, toggle)
	WRAPPER_ACTIVATE(line, aio_cancel64, toggle)
	WRAPPER_ACTIVATE(line, aio_init, toggle)
	WRAPPER_ACTIVATE(line, shm_open, toggle)
	else
	{
		ret = 0;
	}

	return ret;
}

enum listio_mode get_listio_mode(int mode) {
	switch (mode) {
	case LIO_WAIT:
		return lio_wait;
	case LIO_NOWAIT:
		return lio_nowait;
	default:
		return unknown_listio_mode;
	}
}

enum listio_opcode get_listio_opcode(int opcode) {
	switch (opcode) {
	case LIO_READ:
		return lio_read;
	case LIO_WRITE:
		return lio_write;
	default:
		return unknown_listio_opcode;
	}
}

enum async_state get_async_state(int ret) {
	switch (ret) {
	case EINPROGRESS:
		return inprogress;
	case ECANCELED:
		return canceled;
	case 0:
		return completed;
	default:
		return failed;
	}
}

enum async_sync_mode get_async_sync_mode(int op) {
	switch (op) {
	case O_DSYNC:
		return like_fdatasync;
	case O_SYNC:
		return like_fsync;
	default:
		return unknown_async_sync_mode;
	}
}

enum async_cancel_state get_async_cancel_state(int state) {
	switch (state) {
	case AIO_CANCELED:
		return is_canceled;
	case AIO_NOTCANCELED:
		return not_canceled;
	case AIO_ALLDONE:
		return allready_done;
	default:
		return unknown_async_cancel_state;
	}
}

int WRAP(aio_read)(struct aiocb *aiocbp) {
	int ret;
	struct basic data;
	struct asynchronous_read_function asynchronous_read_function_data;
	struct file_descriptor file_descriptor_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, asynchronous_read_function,
			asynchronous_read_function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	file_descriptor_data.descriptor = aiocbp->aio_fildes;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_descriptor,
			file_descriptor_data)
	asynchronous_read_function_data.async = aiocbp;
	asynchronous_read_function_data.bytes_to_read = aiocbp->aio_nbytes;
	asynchronous_read_function_data.position = aiocbp->aio_offset;
	asynchronous_read_function_data.lower_prio = aiocbp->aio_reqprio;

	CALL_REAL_FUNCTION_RET(data, ret, aio_read, aiocbp)

	if (-1 == ret) {
		data.return_state = error;
	} else {
		data.return_state = ok;
	}

	WRAP_END(data)
	return ret;
}

#ifdef HAVE_AIO_READ64
int WRAP(aio_read64)(struct aiocb64 *aiocbp) {
	int ret;
	struct basic data;
	struct asynchronous_read_function asynchronous_read_function_data;
	struct file_descriptor file_descriptor_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, asynchronous_read_function,
			asynchronous_read_function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	file_descriptor_data.descriptor = aiocbp->aio_fildes;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_descriptor,
			file_descriptor_data)
	asynchronous_read_function_data.async = aiocbp;
	asynchronous_read_function_data.bytes_to_read = aiocbp->aio_nbytes;
	asynchronous_read_function_data.position = aiocbp->aio_offset;
	asynchronous_read_function_data.lower_prio = aiocbp->aio_reqprio;

	CALL_REAL_FUNCTION_RET(data, ret, aio_read64, aiocbp)

	if (-1 == ret) {
		data.return_state = error;
	} else {
		data.return_state = ok;
	}

	WRAP_END(data)
	return ret;
}
#endif

int WRAP(aio_write)(struct aiocb *aiocbp) {
	int ret;
	struct basic data;
	struct asynchronous_write_function asynchronous_write_function_data;
	struct file_descriptor file_descriptor_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, asynchronous_write_function,
			asynchronous_write_function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	file_descriptor_data.descriptor = aiocbp->aio_fildes;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_descriptor,
			file_descriptor_data)
	asynchronous_write_function_data.async = aiocbp;
	asynchronous_write_function_data.bytes_to_write = aiocbp->aio_nbytes;
	asynchronous_write_function_data.position = aiocbp->aio_offset;
	asynchronous_write_function_data.lower_prio = aiocbp->aio_reqprio;

	CALL_REAL_FUNCTION_RET(data, ret, aio_write, aiocbp)

	if (-1 == ret) {
		data.return_state = error;
	} else {
		data.return_state = ok;
	}

	WRAP_END(data)
	return ret;
}

#ifdef HAVE_AIO_WRITE64
int WRAP(aio_write64)(struct aiocb64 *aiocbp) {
	int ret;
	struct basic data;
	struct asynchronous_write_function asynchronous_write_function_data;
	struct file_descriptor file_descriptor_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, asynchronous_write_function,
			asynchronous_write_function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	file_descriptor_data.descriptor = aiocbp->aio_fildes;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_descriptor,
			file_descriptor_data)
	asynchronous_write_function_data.async = aiocbp;
	asynchronous_write_function_data.bytes_to_write = aiocbp->aio_nbytes;
	asynchronous_write_function_data.position = aiocbp->aio_offset;
	asynchronous_write_function_data.lower_prio = aiocbp->aio_reqprio;

	CALL_REAL_FUNCTION_RET(data, ret, aio_write64, aiocbp)

	if (-1 == ret) {
		data.return_state = error;
	} else {
		data.return_state = ok;
	}

	WRAP_END(data)
	return ret;
}
#endif

int WRAP(lio_listio)(int mode, struct aiocb * const list[], int nent,
		struct sigevent *sig) {
	int ret;
	struct basic data;
	struct asynchronous_listio_function asynchronous_listio_function_data;
	struct asynchronous_read_function asynchronous_read_function_data;
	struct asynchronous_write_function asynchronous_write_function_data;
	struct file_descriptor file_descriptor_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, asynchronous_listio_function,
			asynchronous_listio_function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_descriptor,
			file_descriptor_data)
	asynchronous_listio_function_data.mode = get_listio_mode(mode);

	CALL_REAL_FUNCTION_RET(data, ret, lio_listio, mode, list, nent, sig)

	if (-1 == ret) {
		data.return_state = error;
	} else {
		data.return_state = ok;
	}

	for (int i = 0; i < nent; i++) {
		if (NULL != list[i]) {
			file_descriptor_data.descriptor = list[i]->aio_fildes;
			asynchronous_listio_function_data.opcode = get_listio_opcode(
					list[i]->aio_lio_opcode);
			switch (asynchronous_listio_function_data.opcode) {
			case lio_write:
				JSON_STRUCT_SET_VOID_P(asynchronous_listio_function_data,
						request_data, asynchronous_write_function,
						asynchronous_write_function_data)
				asynchronous_write_function_data.async = list[i];
				asynchronous_write_function_data.bytes_to_write =
						list[i]->aio_nbytes;
				asynchronous_write_function_data.position = list[i]->aio_offset;
				asynchronous_write_function_data.lower_prio =
						list[i]->aio_reqprio;
				WRAP_END(data)
				break;
			case lio_read:
				JSON_STRUCT_SET_VOID_P(asynchronous_listio_function_data,
						request_data, asynchronous_read_function,
						asynchronous_read_function_data)
				asynchronous_read_function_data.async = list[i];
				asynchronous_read_function_data.bytes_to_read =
						list[i]->aio_nbytes;
				asynchronous_read_function_data.position = list[i]->aio_offset;
				asynchronous_read_function_data.lower_prio =
						list[i]->aio_reqprio;
				WRAP_END(data)
				break;
			default:
				/* ignore LIO_NOP */
				break;
			}
		}
	}

	return ret;
}

#ifdef HAVE_AIO_LIO_LISTIO64
int WRAP(lio_listio64)(int mode, struct aiocb64 *const list[], int nent, struct sigevent *sig) {
	int ret;
	struct basic data;
	struct asynchronous_listio_function asynchronous_listio_function_data;
	struct asynchronous_read_function asynchronous_read_function_data;
	struct asynchronous_write_function asynchronous_write_function_data;
	struct file_descriptor file_descriptor_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, asynchronous_listio_function,
			asynchronous_listio_function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_descriptor,
			file_descriptor_data)
	asynchronous_listio_function_data.mode = get_listio_mode(mode);

	CALL_REAL_FUNCTION_RET(data, ret, lio_listio64, mode, list, nent, sig)

	if (-1 == ret) {
		data.return_state = error;
	} else {
		data.return_state = ok;
	}

	for (int i = 0; i < nent; i++) {
		if (NULL != list[i]) {
			file_descriptor_data.descriptor = list[i]->aio_fildes;
			asynchronous_listio_function_data.opcode = get_listio_opcode(
					list[i]->aio_lio_opcode);
			switch (asynchronous_listio_function_data.opcode) {
				case lio_write:
				JSON_STRUCT_SET_VOID_P(asynchronous_listio_function_data,
						request_data, asynchronous_write_function,
						asynchronous_write_function_data)
				asynchronous_write_function_data.async = list[i];
				asynchronous_write_function_data.bytes_to_write =
				list[i]->aio_nbytes;
				asynchronous_write_function_data.position = list[i]->aio_offset;
				asynchronous_write_function_data.lower_prio =
				list[i]->aio_reqprio;
				WRAP_END(data)
				break;
				case lio_read:
				JSON_STRUCT_SET_VOID_P(asynchronous_listio_function_data,
						request_data, asynchronous_read_function,
						asynchronous_read_function_data)
				asynchronous_read_function_data.async = list[i];
				asynchronous_read_function_data.bytes_to_read =
				list[i]->aio_nbytes;
				asynchronous_read_function_data.position = list[i]->aio_offset;
				asynchronous_read_function_data.lower_prio =
				list[i]->aio_reqprio;
				WRAP_END(data)
				break;
				default:
				/* ignore LIO_NOP */
				break;
			}
		}
	}

	return ret;
}
#endif

int WRAP(aio_error)(const struct aiocb *aiocbp) {
	int ret;
	struct basic data;
	struct asynchronous_error_function asynchronous_error_function_data;
	struct file_async file_async_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, asynchronous_error_function,
			asynchronous_error_function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	file_async_data.async = aiocbp;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_async, file_async_data)

	CALL_REAL_FUNCTION_RET(data, ret, aio_error, aiocbp)

	asynchronous_error_function_data.state = get_async_state(ret);
	if (failed == asynchronous_error_function_data.state) {
		data.return_state = error;
	} else {
		data.return_state = ok;
	}

	WRAP_END(data)
	return ret;
}

#ifdef HAVE_AIO_ERROR64
int WRAP(aio_error64)(const struct aiocb64 *aiocbp) {
	int ret;
	struct basic data;
	struct asynchronous_error_function asynchronous_error_function_data;
	struct file_async file_async_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, asynchronous_error_function,
			asynchronous_error_function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	file_async_data.async = aiocbp;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_async, file_async_data)

	CALL_REAL_FUNCTION_RET(data, ret, aio_error64, aiocbp)

	asynchronous_error_function_data.state = get_async_state(ret);
	if (failed == asynchronous_error_function_data.state) {
		data.return_state = error;
	} else {
		data.return_state = ok;
	}

	WRAP_END(data)
	return ret;
}
#endif

ssize_t WRAP(aio_return)(struct aiocb *aiocbp) {
	ssize_t ret;
	struct basic data;
	struct asynchronous_return_function asynchronous_return_function_data;
	struct file_async file_async_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, asynchronous_return_function,
			asynchronous_return_function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	file_async_data.async = aiocbp;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_async, file_async_data)

	CALL_REAL_FUNCTION_RET(data, ret, aio_return, aiocbp)

	asynchronous_return_function_data.return_value = ret;
	if (-1 == ret) {
		data.return_state = error;
	} else {
		data.return_state = ok;
	}

	WRAP_END(data)
	return ret;
}

#ifdef HAVE_AIO_RETURN64
ssize_t WRAP(aio_return64)(struct aiocb64 *aiocbp) {
	ssize_t ret;
	struct basic data;
	struct asynchronous_return_function asynchronous_return_function_data;
	struct file_async file_async_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, asynchronous_return_function,
			asynchronous_return_function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	file_async_data.async = aiocbp;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_async, file_async_data)

	CALL_REAL_FUNCTION_RET(data, ret, aio_return64, aiocbp)

	asynchronous_return_function_data.return_value = ret;
	if (-1 == ret) {
		data.return_state = error;
	} else {
		data.return_state = ok;
	}

	WRAP_END(data)
	return ret;
}
#endif

int WRAP(aio_fsync)(int op, struct aiocb *aiocbp) {
	int ret;
	struct basic data;
	struct asynchronous_sync_function asynchronous_sync_function_data;
	struct file_async file_async_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, asynchronous_sync_function,
			asynchronous_sync_function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	file_async_data.async = aiocbp;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_async, file_async_data)
	asynchronous_sync_function_data.sync_mode = get_async_sync_mode(op);

	CALL_REAL_FUNCTION_RET(data, ret, aio_fsync, op, aiocbp)

	if (-1 == ret) {
		data.return_state = error;
	} else {
		data.return_state = ok;
	}

	WRAP_END(data)
	return ret;
}

#ifdef HAVE_AIO_FSYNC64
int WRAP(aio_fsync64)(int op, struct aiocb64 *aiocbp) {
	int ret;
	struct basic data;
	struct asynchronous_sync_function asynchronous_sync_function_data;
	struct file_async file_async_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, asynchronous_sync_function,
			asynchronous_sync_function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	file_async_data.async = aiocbp;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_async, file_async_data)
	asynchronous_sync_function_data.sync_mode = get_async_sync_mode(op);

	CALL_REAL_FUNCTION_RET(data, ret, aio_fsync64, op, aiocbp)

	if (-1 == ret) {
		data.return_state = error;
	} else {
		data.return_state = ok;
	}

	WRAP_END(data)
	return ret;
}
#endif

int WRAP(aio_suspend)(const struct aiocb * const list[], int nent,
		const struct timespec *timeout) {
	int ret;
	struct basic data;
	struct asynchronous_suspend_function asynchronous_suspend_function_data;
	struct file_async file_async_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, asynchronous_suspend_function,
			asynchronous_suspend_function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_async, file_async_data)
	asynchronous_suspend_function_data.timeout.sec = timeout->tv_sec;
	asynchronous_suspend_function_data.timeout.nano_sec = timeout->tv_nsec;

	CALL_REAL_FUNCTION_RET(data, ret, aio_suspend, list, nent, timeout)

	if (-1 == ret) {
		data.return_state = error;
	} else {
		data.return_state = ok;
	}

	for (int i = 0; i < nent; i++) {
		if (NULL != list[i]) {
			file_async_data.async = list[i];
			WRAP_END(data)
		}
	}

	return ret;
}

#ifdef HAVE_AIO_SUSPEND64
int WRAP(aio_suspend64)(const struct aiocb64 *const list[], int nent, const struct timespec *timeout) {
	int ret;
	struct basic data;
	struct asynchronous_suspend_function asynchronous_suspend_function_data;
	struct file_async file_async_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, asynchronous_suspend_function,
			asynchronous_suspend_function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_async, file_async_data)
	asynchronous_suspend_function_data.timeout.sec = timeout->tv_sec;
	asynchronous_suspend_function_data.timeout.nano_sec = timeout->tv_nsec;

	CALL_REAL_FUNCTION_RET(data, ret, aio_suspend64, list, nent, timeout)

	if (-1 == ret) {
		data.return_state = error;
	} else {
		data.return_state = ok;
	}

	for (int i = 0; i < nent; i++) {
		if (NULL != list[i]) {
			file_async_data.async = list[i];
			WRAP_END(data)
		}
	}

	return ret;
}
#endif

int WRAP(aio_cancel)(int fildes, struct aiocb *aiocbp) {
	int ret;
	struct basic data;
	struct asynchronous_cancel_function asynchronous_cancel_function_data;
	struct file_descriptor file_descriptor_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, asynchronous_cancel_function,
			asynchronous_cancel_function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	file_descriptor_data.descriptor = fildes;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_descriptor,
			file_descriptor_data)
	asynchronous_cancel_function_data.async = aiocbp;

	CALL_REAL_FUNCTION_RET(data, ret, aio_cancel, fildes, aiocbp)

	asynchronous_cancel_function_data.state = get_async_cancel_state(ret);
	if (-1 == ret) {
		data.return_state = error;
	} else {
		data.return_state = ok;
	}

	WRAP_END(data)
	return ret;
}

#ifdef HAVE_AIO_CANCEL64
int WRAP(aio_cancel64)(int fildes, struct aiocb64 *aiocbp) {
	int ret;
	struct basic data;
	struct asynchronous_cancel_function asynchronous_cancel_function_data;
	struct file_descriptor file_descriptor_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, asynchronous_cancel_function,
			asynchronous_cancel_function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	file_descriptor_data.descriptor = fildes;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_descriptor,
			file_descriptor_data)
	asynchronous_cancel_function_data.async = aiocbp;

	CALL_REAL_FUNCTION_RET(data, ret, aio_cancel64, fildes, aiocbp)

	asynchronous_cancel_function_data.state = get_async_cancel_state(ret);
	if (-1 == ret) {
		data.return_state = error;
	} else {
		data.return_state = ok;
	}

	WRAP_END(data)
	return ret;
}
#endif

#ifdef HAVE_AIO_INIT
void WRAP(aio_init)(const struct aioinit *init) {
	struct basic data;
	struct asynchronous_init_function asynchronous_init_function_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, asynchronous_init_function,
			asynchronous_init_function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P_NULL(data, file_type)
	asynchronous_init_function_data.max_threads = init->aio_threads;
	asynchronous_init_function_data.max_simultaneous_requests = init->aio_num;
	asynchronous_init_function_data.seconds_idle_before_terminate = init->aio_idle_time;

	CALL_REAL_FUNCTION(data, aio_init, init)

	WRAP_END(data)
	return;
}
#endif

int WRAP(shm_open)(const char *name, int oflag, mode_t mode) {
	int ret;
	struct basic data;
	struct file_descriptor file_descriptor_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P_NULL(data, function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_descriptor,
			file_descriptor_data)

	CALL_REAL_FUNCTION_RET(data, ret, shm_open, name, oflag, mode)

	file_descriptor_data.descriptor = ret;
	if (-1 == ret) {
		data.return_state = error;
	} else {
		data.return_state = ok;
	}

	WRAP_END(data)
	return ret;
}
