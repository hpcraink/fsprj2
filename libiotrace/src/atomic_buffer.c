#include "atomic_buffer.h"

#include <stdlib.h>
#include <time.h>
//#include <stdalign.h>

#ifdef ATOMIC_BUFFER_TEST
    /**
     * 0: wait point reached or leaved
     * 1: in atomic_buffer_alloc() after read of buf->_current but before CAS-loop
     * 2: in atomic_buffer_free() after read of  buf->_freed but before CAS-loop
     * 3: in atomic_buffer_wait_until_freed() after read of buf->_current but before CAS-loop
     * 4: in atomic_buffer_reuse() after write of buf->_freed but before write of buf->_current */
    ATTRIBUTE_THREAD int *atomic_buffer_wait_pos;
    static struct timespec wait_time = {0, 0};
    /**
     * Implements a wait point for testing thread concurrency.
     *
     * If *atomic_buffer_wait_pos is not equal to "pos" the wait point
     * is ignored. If it is equal to "pos" the thread sets
     * *atomic_buffer_wait_pos to 0. This signals the wait point is
     * reached. After that the thread waits until
     * *atomic_buffer_wait_pos is again set to "pos". Then the thread
     * sets *atomic_buffer_wait_pos again to 0 (signal for wait point
     * is leaved) and resumes after the wait point.
     *
     * @param[in]  pos        unique number of the wait point
     *                        (0 signals that last wait point was reached,
     *                        so valid values for pos are integers greater
     *                        than 0)
     */
#   define ATOMIC_BUFFER_TEST_WAIT(pos) do { \
                                            if (pos == __atomic_load_n(atomic_buffer_wait_pos, __ATOMIC_ACQUIRE)) { \
                                                __atomic_store_n(atomic_buffer_wait_pos, 0, __ATOMIC_RELEASE); \
                                                while(pos != __atomic_load_n(atomic_buffer_wait_pos, __ATOMIC_ACQUIRE)) { \
                                                    nanosleep(&wait_time, NULL); \
                                                }; \
                                                __atomic_store_n(atomic_buffer_wait_pos, 0, __ATOMIC_RELEASE); \
                                            } \
                                        } while(0)
#else
#  define ATOMIC_BUFFER_TEST_WAIT(pos)
#endif

#define ATOMIC_BUFFER_PREFIX_SIZE sizeof(struct atomic_buffer_prefix)
/* TODO: alignment of ATOMIC_BUFFER_PREFIX_SIZE */

int atomic_buffer_create(struct atomic_buffer *buf, size_t size) {
	buf->_start = malloc(size);
	if (buf->_start == NULL) {
		return -1;
	}

	buf->_end = (char*) buf->_start + size;
	buf->_current = buf->_start;
	buf->_freed = buf->_start;

	/* Memory Fence to ensure all caches are written (for multi-core systems) */
	__atomic_thread_fence(__ATOMIC_SEQ_CST);
	/* at this point all threads have an initialized struct buf */

	return 0;
}

void* atomic_buffer_alloc(struct atomic_buffer *buf, size_t size) {
	void *old_value;
	void *new_value;
	size_t tmp_size;

	/* reserve a prefix additional to the requested bytes to
	 * store the length of the allocated memory (needed to
	 * free the memory) */
	tmp_size = size + ATOMIC_BUFFER_PREFIX_SIZE;

	/* TODO: calculate alignment: padding to align to max_align_t ? */
	//size += ???;
	/* change buf->_current with an atomic instruction */
	old_value = __atomic_load_n(&(buf->_current), __ATOMIC_ACQUIRE);
	ATOMIC_BUFFER_TEST_WAIT(1);
	do {
		/* evaluate if enough memory for "size" bytes is available */
		if ((char*) old_value + tmp_size
				<= (char*) __atomic_load_n(&(buf->_end), __ATOMIC_ACQUIRE)) {
			new_value = (char*) old_value + tmp_size;
			/* pointers in one buffer are unique and get only increased
			 * => no ABA problem */
		} else {
			return NULL;
		}
	} while (!__atomic_compare_exchange_n(&(buf->_current), &old_value,
			new_value, 1,
			__ATOMIC_ACQ_REL /* read and write of buf->_current */,
			__ATOMIC_ACQUIRE /* read of buf->_current */));
	/* switch old current position with new one, if (and only if) no other
	 * thread has moved the current position */
	/* TODO: test for __atomic_compare_exchange_n compiler support */

	/* save length of allocated memory for atomic_buffer_free()
	 * (has to be atomic, to ensure that an following free from
	 * another thread is safely executed) */
	__atomic_store_n(&(((struct atomic_buffer_prefix*) old_value)->length),
			size, __ATOMIC_RELEASE);

	/* return start address of allocated memory beginning after prefix */
	return (char*) old_value + ATOMIC_BUFFER_PREFIX_SIZE;
}

void atomic_buffer_free(struct atomic_buffer *buf, void *memory) {
	void *old_value;
	void *new_value;
	size_t length;

	if ((char*) memory
			< (char*) __atomic_load_n(&(buf->_start), __ATOMIC_ACQUIRE)
					+ ATOMIC_BUFFER_PREFIX_SIZE
			|| memory >= __atomic_load_n(&(buf->_end), __ATOMIC_ACQUIRE)) {
		/* TODO create warning */
		return;
	}

	/* calculate number of bytes to be set free */
	length = ATOMIC_BUFFER_PREFIX_SIZE
			+ __atomic_load_n(
					&(((struct atomic_buffer_prefix*) ((char*) memory
							- ATOMIC_BUFFER_PREFIX_SIZE))->length),
					__ATOMIC_ACQUIRE);

	/* change buf->_freed with an atomic instruction */
	old_value = __atomic_load_n(&(buf->_freed), __ATOMIC_ACQUIRE);
	ATOMIC_BUFFER_TEST_WAIT(2);
	do {
		new_value = (char*) old_value + length;
		/* pointers in one buffer are unique and get only increased
		 * => no ABA problem */
	} while (!__atomic_compare_exchange_n(&(buf->_freed), &old_value, new_value,
			1, __ATOMIC_ACQ_REL /* read and write of buf->_freed */,
			__ATOMIC_ACQUIRE /* read of buf->_freed */));
	/* switch old freed position with new one, if (and only if) no other
	 * thread has moved the freed position */
}

void atomic_buffer_wait_until_freed(struct atomic_buffer *buf,
		long sleep_nanoseconds) {
	void *old_value;
	void *new_value;
	void *tmp_end;
	struct timespec sleep_time;

	sleep_time.tv_sec = 0;
	if (sleep_nanoseconds < 0) {
		sleep_nanoseconds = 0;
	} else if (sleep_nanoseconds > 999999999) {
		sleep_nanoseconds = 999999999;
	}
	sleep_time.tv_nsec = sleep_nanoseconds;

	/* change buf->_current with an atomic instruction */
	tmp_end = __atomic_load_n(&(buf->_end), __ATOMIC_ACQUIRE);
	old_value = __atomic_load_n(&(buf->_current), __ATOMIC_ACQUIRE);
	ATOMIC_BUFFER_TEST_WAIT(3);
	do {
		new_value = tmp_end + 1;
		/* pointers in one buffer are unique and get only increased
		 * => no ABA problem */
	} while (!__atomic_compare_exchange_n(&(buf->_current), &old_value,
			new_value, 1,
			__ATOMIC_ACQ_REL /* read and write of buf->_current */,
			__ATOMIC_ACQUIRE /* read of buf->_current */));
	/* switch old current position with new one, if (and only if) no other
	 * thread has moved the current position */

	/* at this point buf->_current is set to a value greater as
	 * buf->_end, so no new allocations are possible (see
	 * atomic_buffer_alloc()). Now we can wait until every
	 * allocated memory is freed and free or reuse the whole
	 * buffer. */

	while (__atomic_load_n(&(buf->_freed), __ATOMIC_ACQUIRE) < old_value) {
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

void atomic_buffer_destroy(struct atomic_buffer *buf,
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

	free(__atomic_load_n(&(buf->_start), __ATOMIC_ACQUIRE));
}

void atomic_buffer_reuse(struct atomic_buffer *buf,
		long sleep_nanoseconds) {
	void *tmp_start;

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
	tmp_start = __atomic_load_n(&(buf->_start), __ATOMIC_ACQUIRE);
	__atomic_store_n(&(buf->_freed), tmp_start, __ATOMIC_SEQ_CST);
	ATOMIC_BUFFER_TEST_WAIT(4);
	__atomic_store_n(&(buf->_current), tmp_start, __ATOMIC_SEQ_CST);
}

int atomic_buffer_try_reuse(struct atomic_buffer *buf) {

}

size_t atomic_buffer_get_free_memory(struct atomic_buffer *buf) {
	return buf->_end - buf->_current;
}

size_t atomic_buffer_get_freed_memory(struct atomic_buffer *buf) {
	return buf->_freed - buf->_start;
}
