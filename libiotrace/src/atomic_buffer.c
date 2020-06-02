#include "atomic_buffer.h"

#include <stdlib.h>
#include <time.h>
//#include <stdalign.h>

#define ATOMIC_BUFFER_PREFIX_SIZE sizeof(struct atomic_buffer_prefix)
/* TODO: alignment of ATOMIC_BUFFER_PREFIX_SIZE */

int atomic_buffer_create(volatile struct atomic_buffer *buf, size_t size) {
	buf->_start = malloc(size);
	if (buf->_start == NULL) {
		return -1;
	}

	buf->_end = (char*) buf->_start + size;
	buf->_current = buf->_start;
	buf->_freed = buf->_start;

	return 0;
}

void* atomic_buffer_alloc(volatile struct atomic_buffer *buf, size_t size) {
	void *old_value;
	void *new_value;
	size_t tmp_size;

	/* reserve a prefix additional to the requested bytes to
	 * store the length of the allocated memory (needed to
	 * free the memory) */
	tmp_size = size + ATOMIC_BUFFER_PREFIX_SIZE;

	/* TODO: calculate alignment: padding to align to max_align_t ? */
	//size += ???;
	/* change buf->current with an atomic instruction */
	old_value = buf->_current;
	do {
		/* evaluate if enough memory for "size" bytes is available */
		if ((char*) old_value + tmp_size <= (char*) buf->_end) {
			new_value = (char*) old_value + tmp_size;
			/* pointers in one buffer are unique and get only increased
			 * => no ABA problem */
		} else {
			return NULL;
		}
	} while (!__atomic_compare_exchange_n(&(buf->_current), &old_value,
			new_value, 0, __ATOMIC_RELAXED, __ATOMIC_RELAXED));
	/* switch old current position with new one, if (and only if) no other
	 * thread has moved the current position */
	/* TODO: test for __atomic_compare_exchange_n compiler support */

	/* save length of allocated memory for atomic_buffer_free() */
	((struct atomic_buffer_prefix*) old_value)->length = size;

	/* return start address of allocated memory beginning after prefix */
	return (char*) old_value + ATOMIC_BUFFER_PREFIX_SIZE;
}

void atomic_buffer_free(volatile struct atomic_buffer *buf, void *memory) {
	void *old_value;
	void *new_value;
	size_t length;

	if ((char*) memory < (char*) buf->_start + ATOMIC_BUFFER_PREFIX_SIZE
			|| memory >= buf->_end) {
		/* TODO create warning */
		return;
	}

	/* calculate number of bytes to be set free */
	length = ATOMIC_BUFFER_PREFIX_SIZE
			+ ((struct atomic_buffer_prefix*) ((char*) memory
					- ATOMIC_BUFFER_PREFIX_SIZE))->length;

	/* change buf->freed with an atomic instruction */
	old_value = buf->_freed;
	do {
		new_value = (char*) old_value + length;
		/* pointers in one buffer are unique and get only increased
		 * => no ABA problem */
	} while (!__atomic_compare_exchange_n(&(buf->_freed), &old_value, new_value,
			0, __ATOMIC_RELAXED, __ATOMIC_RELAXED));
	/* switch old freed position with new one, if (and only if) no other
	 * thread has moved the freed position */
}

void atomic_buffer_wait_until_freed(volatile struct atomic_buffer *buf,
		long sleep_nanoseconds) {
	void *old_value;
	void *new_value;
	struct timespec sleep_time;

	sleep_time.tv_sec = 0;
	if (sleep_nanoseconds < 0) {
		sleep_nanoseconds = 0;
	} else if (sleep_nanoseconds > 999999999) {
		sleep_nanoseconds = 999999999;
	}
	sleep_time.tv_nsec = sleep_nanoseconds;

	/* change buf->current with an atomic instruction */
	old_value = buf->_current;
	do {
		new_value = buf->_end + 1;
		/* pointers in one buffer are unique and get only increased
		 * => no ABA problem */
	} while (!__atomic_compare_exchange_n(&(buf->_current), &old_value,
			new_value, 0, __ATOMIC_RELAXED, __ATOMIC_RELAXED));
	/* switch old current position with new one, if (and only if) no other
	 * thread has moved the current position */

	/* at this point buf->_current is set to a value greater as
	 * buf->_end, so no new allocations are possible (see
	 * atomic_buffer_alloc()). Now we can wait until every
	 * allocated memory is freed and free or reuse the whole
	 * buffer. */

	while (buf->_freed < old_value) {
		/* this is not lock free. It blocks until all threads have freed
		 * their allocated memory. If a single thread doesn't frees all
		 * allocated memory it waits forever. */

		nanosleep(&sleep_time, NULL);
	}

	/* at this point no new allocations are possible and all old
	 * allocations are freed => it's possible to free or reuse
	 * the buffer (but be aware that some threads could be stalled
	 * in or before the CAS-loop of atomic_buffer_alloc(), so
	 * reuse and especially free is not without risk) */
}

void atomic_buffer_destroy(volatile struct atomic_buffer *buf,
		long sleep_nanoseconds) {
	atomic_buffer_wait_until_freed(buf, sleep_nanoseconds);

	/* at this point some threads could be stalled in
	 * atomic_buffer_alloc() and trying to use "buf" in subsequent
	 * instructions. They will fail because
	 * atomic_buffer_wait_until_freed() sets the available size
	 * (_current) to -1. So a call to atomic_buffer_destroy() should
	 * only be done if it's ensured that no thread is calling
	 * atomic_buffer_alloc() with "buf" as an parameter at this
	 * moment or in the future. */

	free(buf->_start);
}

void atomic_buffer_reuse(volatile struct atomic_buffer *buf,
		long sleep_nanoseconds) {
	atomic_buffer_wait_until_freed(buf, sleep_nanoseconds);

	/* at this point it is possible to have a thread trying to allocate
	 * memory from buf with atomic_buffer_alloc() (the thread could be
	 * stalled in or before the CAS-loop). But it is not possible to
	 * have a thread trying to free an allocated memory area, because
	 * all allocated memory areas are already freed. So a call to
	 * atomic_buffer_free() results in undefined behavior and is a bug
	 * and not a useful scenario.
	 * With this in mind it is possible to enable reuse of the buffer.
	 * For that the _freed pointer must set to the initial value before
	 * the _current pointer is set. Only with this order of reseting the
	 * pointers it's ensured that _freed is not greater or equal to
	 * _current after the first successful return of
	 * atomic_buffer_alloc(). This can be acquired with
	 * __ATOMIC_SEQ_CST. */
	__atomic_store_n(&(buf->_freed), buf->_start, __ATOMIC_SEQ_CST);
	__atomic_store_n(&(buf->_current), buf->_start, __ATOMIC_SEQ_CST);
}

size_t atomic_buffer_get_free_memory(volatile struct atomic_buffer *buf) {
	return buf->_end - buf->_current;
}

size_t atomic_buffer_get_freed_memory(volatile struct atomic_buffer *buf) {
	return buf->_freed - buf->_start;
}
