/**
 * @file Implementation of alloc functions.
 */
#include "libiotrace_config.h"

#include <stdlib.h>

#include "event.h"
#include "wrapper_defines.h"
#include "alloc.h"

#include "libiotrace_include_struct.h"
#include "wrapper_name.h"

/* must be static because calloc has to return zero initialized memory
 * and static variables are zero initialized */
static char static_calloc_buffer[STATIC_CALLOC_BUFFER_SIZE];
static char *static_calloc_buffer_pos = static_calloc_buffer;

#ifndef IO_LIB_STATIC
REAL_DEFINITION_TYPE void* REAL_DEFINITION(malloc)(size_t size) REAL_DEFINITION_INIT;
REAL_DEFINITION_TYPE void REAL_DEFINITION(free)(void *ptr) REAL_DEFINITION_INIT;
REAL_DEFINITION_TYPE void* REAL_DEFINITION(calloc)(size_t nmemb, size_t size) REAL_DEFINITION_INIT;
REAL_DEFINITION_TYPE void* REAL_DEFINITION(realloc)(void *ptr, size_t size) REAL_DEFINITION_INIT;
#ifdef HAVE_REALLOCARRAY
REAL_DEFINITION_TYPE void* REAL_DEFINITION(reallocarray)(void *ptr, size_t nmemb, size_t size) REAL_DEFINITION_INIT;
#endif
#endif

char toggle_alloc_wrapper(const char *line, const char toggle)
{
	char ret = 1;

	if (!strcmp(line, "")) {
		ret = 0;
	}
#undef WRAPPER_NAME_TO_SOURCE
#define WRAPPER_NAME_TO_SOURCE WRAPPER_NAME_TO_SET_VARIABLE
#include "alloc_wrapper.h"
	else
	{
		ret = 0;
	}
	return ret;
}

#ifndef IO_LIB_STATIC
char alloc_init_done = 0;
/* Initialize pointers for alloc functions. */
void alloc_init(void) {
	if (!alloc_init_done) {

#undef WRAPPER_NAME_TO_SOURCE
#define WRAPPER_NAME_TO_SOURCE WRAPPER_NAME_TO_DLSYM
#include "alloc_wrapper.h"

		alloc_init_done = 1;
	}
}
#endif

void* WRAP(malloc)(size_t size) {
	void *ret;
	struct basic data;
	struct alloc_function alloc_function_data;
	struct file_alloc file_alloc_data;
	WRAP_START(data)

	get_basic(&data);
	LIBIOTRACE_STRUCT_SET_VOID_P(data, function_data, alloc_function,
			alloc_function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	LIBIOTRACE_STRUCT_SET_VOID_P(data, file_type, file_alloc, file_alloc_data)
	alloc_function_data.size = size;

	CALL_REAL_FUNCTION_RET(data, ret, malloc, size)

	if (NULL == ret && 0 != size) {
		data.return_state = error;
	} else {
		data.return_state = ok;
	}
	file_alloc_data.address = ret;

	WRAP_END(data, malloc)
	return ret;
}

void WRAP(free)(void *ptr) {
	struct basic data;
	struct file_alloc file_alloc_data;

	if ((char*) ptr
			>= &(static_calloc_buffer[0])&& (char*)ptr < &(static_calloc_buffer[0]) + STATIC_CALLOC_BUFFER_SIZE) {
		/* ptr was returned by wrapper of calloc from static memory: don't
		 * free it (will result in undefined behavior) */
		return;
	}

	WRAP_START(data)

	get_basic(&data);
	LIBIOTRACE_STRUCT_SET_VOID_P_NULL(data, function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	LIBIOTRACE_STRUCT_SET_VOID_P(data, file_type, file_alloc, file_alloc_data)

	CALL_REAL_FUNCTION(data, free, ptr)

	data.return_state = ok;
	file_alloc_data.address = ptr;
	WRAP_END(data, free)
}

void* WRAP(calloc)(size_t nmemb, size_t size) {
	void *ret;
	struct basic data;
	struct alloc_function alloc_function_data;
	struct file_alloc file_alloc_data;
	size_t real_size;
	char *old_value;
	char *new_value;

	/* gcc assumes the address of ‘__real_calloc’ will never be NULL
	 * this produces wrong warnings in the following if-statement */
#pragma GCC diagnostic ignored "-Waddress"
	if (NULL == CALL_REAL(calloc)) {
#pragma GCC diagnostic pop
		/* dlsym compiled with pthread uses calloc: first call of dlsym calls
		 * calloc before initialization of pointer to real calloc is done.
		 * That will start initialization via alloc_init() which calls dlsym
		 * which calls calloc which calls alloc_init() and so on.
		 *
		 * To prevent that a call of the wrapper of calloc with uninitialized
		 * pointer to real calloc must return memory from a static allocated
		 * area. So no recursive function calls will happen. This means also
		 * that calls of calloc before initialization via ctor are not logged. */

		if (0 == nmemb || 0 == size) {
			return NULL;
		}

		/* get count of needed bytes by multiplication of parameter nmemb and size */
		if (__builtin_mul_overflow(nmemb, size, &real_size)) {
			// TODO: check for __builtin_mul_overflow
			/* we have an overflow */
			errno = ENOMEM;
			return NULL;
		}

		old_value = __atomic_load_n(&static_calloc_buffer_pos,
		__ATOMIC_RELAXED);
		// TODO: check for __atomic_load_n
		do {
			/* check if static_calloc_buffer has enough free memory left for
			 * needed size (do it without adding size to static_calloc_buffer_pos
			 * to prevent wrap around during evaluation) */
			if ((size_t)(&(static_calloc_buffer[0]) + STATIC_CALLOC_BUFFER_SIZE
					- old_value) >= real_size) {
				new_value = (void*) (old_value + real_size);
			} else {
				errno = ENOMEM;
				return NULL;
			}
		} while (!__atomic_compare_exchange_n(&static_calloc_buffer_pos,
				&old_value, (void*) new_value, 1,
				__ATOMIC_RELAXED,
				__ATOMIC_RELAXED));

		return old_value;
	}

	WRAP_START(data)

	get_basic(&data);
	LIBIOTRACE_STRUCT_SET_VOID_P(data, function_data, alloc_function,
			alloc_function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	LIBIOTRACE_STRUCT_SET_VOID_P(data, file_type, file_alloc, file_alloc_data)
	alloc_function_data.size = nmemb * size;

	CALL_REAL_FUNCTION_RET(data, ret, calloc, nmemb, size)

	if (NULL == ret && 0 != size) {
		data.return_state = error;
	} else {
		data.return_state = ok;
	}
	file_alloc_data.address = ret;

	WRAP_END(data, calloc)
	return ret;
}

void* WRAP(realloc)(void *ptr, size_t size) {
	void *ret;
	struct basic data;
	struct alloc_function alloc_function_data;
	struct file_alloc file_alloc_data;
	WRAP_START(data)

	get_basic(&data);
	LIBIOTRACE_STRUCT_SET_VOID_P(data, function_data, alloc_function,
			alloc_function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	LIBIOTRACE_STRUCT_SET_VOID_P(data, file_type, file_alloc, file_alloc_data)
	alloc_function_data.size = size;

	CALL_REAL_FUNCTION_RET(data, ret, realloc, ptr, size)

	if (NULL == ret && 0 != size) {
		data.return_state = error;
	} else {
		data.return_state = ok;
	}
	file_alloc_data.address = ret;

	WRAP_END(data, realloc)
	return ret;
}

#ifdef HAVE_REALLOCARRAY
void* WRAP(reallocarray)(void *ptr, size_t nmemb, size_t size) {
	void *ret;
	struct basic data;
	struct alloc_function alloc_function_data;
	struct file_alloc file_alloc_data;
	WRAP_START(data)

	get_basic(&data);
	LIBIOTRACE_STRUCT_SET_VOID_P(data, function_data, alloc_function,
			alloc_function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	LIBIOTRACE_STRUCT_SET_VOID_P(data, file_type, file_alloc, file_alloc_data)
	alloc_function_data.size = nmemb * size;

	CALL_REAL_FUNCTION_RET(data, ret, reallocarray, ptr, nmemb, size)

	if (NULL == ret && 0 != size) {
		data.return_state = error;
	} else {
		data.return_state = ok;
	}
	file_alloc_data.address = ret;

	WRAP_END(data, reallocarray)
	return ret;
}
#endif
