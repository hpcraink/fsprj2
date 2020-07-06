#include "atomic_buffer_write_once.h"

#include <stdlib.h>

int atomic_buffer_write_once_create(struct atomic_buffer_write_once *buf,
		size_t size) {
	buf->_start = malloc(size);
	if (buf->_start == NULL) {
		return -1;
	}

	buf->_end = (char*) buf->_start + size;
	buf->_free = buf->_start;
	buf->_ready = NULL;

	/* Memory Fence to ensure all caches are written (for multi-core systems) */
	__atomic_thread_fence(__ATOMIC_SEQ_CST);
	/* at this point all threads have an initialized struct buf */

	return 0;
}

void atomic_buffer_write_once_destroy(struct atomic_buffer_write_once *buf) {
	free(buf->_start);
}

void atomic_buffer_write_once_push(struct atomic_buffer_write_once *buf, void *ready) {
	void *old_value;
	void *new_value;
	struct atomic_buffer_write_once_prefix *node =
			(struct atomic_buffer_write_once_prefix*) ((char*) ready
					- ATOMIC_BUFFER_WRITE_ONCE_PREFIX_SIZE);

	old_value = __atomic_load_n(&(buf->_ready), __ATOMIC_RELAXED);
	do {
		node->next = old_value;
		new_value = node;
	} while (!__atomic_compare_exchange_n(&(buf->_ready), &old_value, new_value,
			1,
			__ATOMIC_RELAXED /* read and write of buf->_free */,
			__ATOMIC_RELAXED /* read of buf->_free */));
}

void *atomic_buffer_write_once_pop(struct atomic_buffer_write_once *buf) {
	void *old_value;
	void *new_value;
	struct atomic_buffer_write_once_prefix *node;

	old_value = __atomic_load_n(&(buf->_ready), __ATOMIC_RELAXED);
	do {
		node = (struct atomic_buffer_write_once_prefix*) old_value;
		new_value = node->next;
	} while (!__atomic_compare_exchange_n(&(buf->_ready), &old_value, new_value,
			1,
			__ATOMIC_RELAXED /* read and write of buf->_free */,
			__ATOMIC_RELAXED /* read of buf->_free */));

	return (char*) old_value + ATOMIC_BUFFER_WRITE_ONCE_PREFIX_SIZE;
}
