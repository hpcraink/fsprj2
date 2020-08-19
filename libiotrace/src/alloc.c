/**
 * @file Implementation of alloc functions.
 */
#include "libiotrace_config.h"

#include <stdlib.h>

#include "json_include_struct.h"
#include "event.h"
#include "wrapper_defines.h"
#include "alloc.h"

#define STATIC_CALLOC_BUFFER_SIZE 1024

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

#ifndef IO_LIB_STATIC
char alloc_init_done = 0;
/* Initialize pointers for alloc functions. */
void alloc_init() {
	if (!alloc_init_done) {

		DLSYM(malloc);
		DLSYM(free);
		DLSYM(calloc);
		DLSYM(realloc);
#ifdef HAVE_REALLOCARRAY
		DLSYM(reallocarray);
#endif

		alloc_init_done = 1;
	}
}
#endif

void* WRAP(malloc)(size_t size) {
	void *ret;
	struct basic data;
	struct alloc_function alloc_function_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, alloc_function,
			alloc_function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P_NULL(data, file_type)
	alloc_function_data.size = size;

	CALL_REAL_FUNCTION_RET(data, ret, malloc, size)

	if (NULL == ret && 0 != size) {
		data.return_state = error;
	} else {
		data.return_state = ok;
	}

	WRAP_END(data)
	return ret;
}

void WRAP(free)(void *ptr) {
	if ((char*) ptr
			>= &(static_calloc_buffer[0])&& (char*)ptr < &(static_calloc_buffer[0]) + STATIC_CALLOC_BUFFER_SIZE) {
		/* ptr was returned by wrapper of calloc from static memory: don't
		 * free it (will result in undefined behavior) */
		return;
	}

	if (!init_done) {
		init_basic();
		/* if some __attribute__((constructor))-function calls a wrapped function:
		 * init_basic() must be called first */
	}

	CALL_REAL(free)(ptr);
}

void* WRAP(calloc)(size_t nmemb, size_t size) {
	void *ret;
	struct basic data;
	struct alloc_function alloc_function_data;
	size_t real_size;
	char *old_value;
	char *new_value;

	if (NULL == (void*) CALL_REAL(calloc)) {
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
			if (&(static_calloc_buffer[0]) + STATIC_CALLOC_BUFFER_SIZE
					- old_value >= real_size) {
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
	JSON_STRUCT_SET_VOID_P(data, function_data, alloc_function,
			alloc_function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P_NULL(data, file_type)
	alloc_function_data.size = nmemb * size;

	CALL_REAL_FUNCTION_RET(data, ret, calloc, nmemb, size)

	if (NULL == ret && 0 != size) {
		data.return_state = error;
	} else {
		data.return_state = ok;
	}

	WRAP_END(data)
	return ret;
}

void* WRAP(realloc)(void *ptr, size_t size) {
	void *ret;
	struct basic data;
	struct alloc_function alloc_function_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, alloc_function,
			alloc_function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P_NULL(data, file_type)
	alloc_function_data.size = size;

	CALL_REAL_FUNCTION_RET(data, ret, realloc, ptr, size)

	if (NULL == ret && 0 != size) {
		data.return_state = error;
	} else {
		data.return_state = ok;
	}

	WRAP_END(data)
	return ret;
}

#ifdef HAVE_REALLOCARRAY
void* WRAP(reallocarray)(void *ptr, size_t nmemb, size_t size) {
	void *ret;
	struct basic data;
	struct alloc_function alloc_function_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, alloc_function,
			alloc_function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P_NULL(data, file_type)
	alloc_function_data.size = nmemb * size;

	CALL_REAL_FUNCTION_RET(data, ret, reallocarray, ptr, nmemb, size)

	if (NULL == ret && 0 != size) {
		data.return_state = error;
	} else {
		data.return_state = ok;
	}

	WRAP_END(data)
	return ret;
}
#endif
