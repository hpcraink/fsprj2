#ifndef LIBIOTRACE_ATOMIC_BUFFER_WRITE_ONCE_H
#define LIBIOTRACE_ATOMIC_BUFFER_WRITE_ONCE_H

#include <stddef.h>
#include <stdlib.h>

#include "libiotrace_config.h"

#define ATOMIC_BUFFER_WRITE_ONCE_PREFIX_SIZE sizeof(struct atomic_buffer_write_once_prefix)

BEGIN_C_DECLS

struct atomic_buffer_write_once {
	void *_start;
	void *_end;
	void *_free;
	void *_ready;
};

struct atomic_buffer_write_once_prefix {
	void *next;
};

int atomic_buffer_write_once_create(struct atomic_buffer_write_once *buf,
		size_t size);

static inline __attribute__((always_inline)) __attribute__((warn_unused_result)) __attribute__((malloc)) __attribute__((hot)) void* atomic_buffer_write_once_alloc(
		struct atomic_buffer_write_once *buf, size_t size)
				__attribute__((nonnull (1)));
static inline void* atomic_buffer_write_once_alloc(
		struct atomic_buffer_write_once *buf, size_t size) {
	void *old_value;
	void *new_value;

	/* reserve a prefix additional to the requested bytes to
	 * store the linked list for readied memory blocks */
	size += ATOMIC_BUFFER_WRITE_ONCE_PREFIX_SIZE;

	/* change buf->_free with atomic instructions (a CAS loop instead of a atomic
	 * add is needed to ensure that the new _free value is not the result of an
	 * overflow or an wrap around) */
	old_value = __atomic_load_n(&(buf->_free), __ATOMIC_RELAXED);
	do {

		/* Omit overflow or wrap around during evaluation of available bytes in buf
		 * by checking difference between _end and _free against size instead of
		 * adding size to _free and check it against _end (_end and _free have known
		 * values, size has unknown value).
		 * The value of buf->_end doesn't change during runtime (after
		 * initialization), so it's not necessary to get it with an atomic load. */
		if ((char*) buf->_end - (char*) old_value >= size) {
			//TODO: faster size check ??? if ((char*) buf->_end + ~((char*) old_value) +1 >= size) {
			new_value = (char*) old_value + size;
			/* pointers in one buffer are unique and get only increased
			 * => no ABA problem */
		} else {
			return NULL; //TODO: abort() ???
		}

		/* switch old _free position with new one, if (and only if) no other
		 * thread has moved the _free position */
		/* TODO: test for __atomic_compare_exchange_n compiler support */
	} while (!__atomic_compare_exchange_n(&(buf->_free), &old_value, new_value,
			1,
			__ATOMIC_RELAXED /* read and write of buf->_free */,
			__ATOMIC_RELAXED /* read of buf->_free */));

//	/* not really faster than CAS-loop and unsafe in regard of overflow */
//	old_value = __atomic_fetch_add(&(buf->_free), size, __ATOMIC_RELAXED);
//	if ((char*) buf->_end - (char*) old_value < size) {
//		abort();
//	}

	/* return start address of allocated memory beginning after prefix */
	return (char*) old_value + ATOMIC_BUFFER_WRITE_ONCE_PREFIX_SIZE;
}

void atomic_buffer_write_once_push(struct atomic_buffer_write_once *buf,
		void *ready);

void* atomic_buffer_write_once_pop(struct atomic_buffer_write_once *buf);

void atomic_buffer_write_once_destroy(struct atomic_buffer_write_once *buf);

END_C_DECLS

#endif /* LIBIOTRACE_ATOMIC_BUFFER_WRITE_ONCE_H */
