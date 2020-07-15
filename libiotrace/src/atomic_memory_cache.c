#include "atomic_memory_cache.h"

static union atomic_memory_block_tag_ptr atomic_memory_cache[atomic_memory_size_count] =
		{ { ._tag_ptr._ptr = NULL, ._tag_ptr._tag = 0 } };
static union atomic_memory_block_tag_ptr atomic_memory_ready = {
		._tag_ptr._ptr = NULL, ._tag_ptr._tag = 0 };
static char atomic_memory_buffer[ATOMIC_MEMORY_BUFFER_SIZE]__attribute__((aligned (ATOMIC_MEMORY_ALIGNMENT)));
static char *atomic_memory_buffer_pos = atomic_memory_buffer;

inline struct atomic_memory_block* atomic_memory_new_block(
		enum atomic_memory_size size)__attribute__((always_inline))__attribute__((warn_unused_result))__attribute__((malloc));

union atomic_memory_block_tag_ptr atomic_memory_alloc(
		enum atomic_memory_size size) {
	union atomic_memory_block_tag_ptr old_value;
	union atomic_memory_block_tag_ptr new_value;

	old_value._integral_type = __atomic_load_n(
			&(atomic_memory_cache[size]._integral_type), __ATOMIC_RELAXED);
	do {
		if (NULL != old_value._tag_ptr._ptr) {
			new_value = old_value._tag_ptr._ptr->_next;
		} else {
			new_value._tag_ptr._ptr = atomic_memory_new_block(size);
			new_value._tag_ptr._tag = 0;
			return new_value;
		}
	} while (!__atomic_compare_exchange_n(
			&(atomic_memory_cache[size]._integral_type),
			&(old_value._integral_type), new_value._integral_type, 1,
			__ATOMIC_RELAXED /* read and write */,
			__ATOMIC_RELAXED /* read */));

	old_value._tag_ptr._tag++; //TODO: check for wrap around and throw error
	return old_value;
}

struct atomic_memory_block* atomic_memory_new_block(
		enum atomic_memory_size size) {
	char *old_value;
	char *new_value;
	// TODO: check for wrap around
	size_t real_size = atomic_memory_sizes[size]
			+ sizeof(struct atomic_memory_block);

	/* keep the beginning of the following block aligned to ATOMIC_MEMORY_ALIGNMENT */
	// TODO: check for wrap around
	real_size = (real_size + ATOMIC_MEMORY_ALIGNMENT - 1)
			& ~(ATOMIC_MEMORY_ALIGNMENT - 1);

	old_value = __atomic_load_n(&(atomic_memory_buffer_pos), __ATOMIC_RELAXED);
	do {
		if (atomic_memory_buffer + ATOMIC_MEMORY_BUFFER_SIZE - old_value
				>= real_size) {
			new_value = (void*) (old_value + real_size);
		} else {
			return NULL; //TODO: abort() ???
		}
	} while (!__atomic_compare_exchange_n(&(atomic_memory_buffer_pos),
			&old_value, (void*) new_value, 1,
			__ATOMIC_RELAXED /* read and write */,
			__ATOMIC_RELAXED /* read */));

	((struct atomic_memory_block*) old_value)->_size = size;
	return (struct atomic_memory_block*) old_value;
}

void atomic_memory_push(union atomic_memory_block_tag_ptr block) {
	union atomic_memory_block_tag_ptr old_value;

	old_value._integral_type = __atomic_load_n(
			&(atomic_memory_ready._integral_type), __ATOMIC_RELAXED);
	do {
		block._tag_ptr._ptr->_next = old_value;
	} while (!__atomic_compare_exchange_n(&(atomic_memory_ready._integral_type),
			&(old_value._integral_type), block._integral_type, 1,
			__ATOMIC_RELAXED /* read and write */,
			__ATOMIC_RELAXED /* read */));
}

union atomic_memory_block_tag_ptr atomic_memory_pop() {
	union atomic_memory_block_tag_ptr old_value;
	union atomic_memory_block_tag_ptr new_value;

	old_value._integral_type = __atomic_load_n(
			&(atomic_memory_ready._integral_type), __ATOMIC_RELAXED);
	do {
		if (NULL != old_value._tag_ptr._ptr) {
			new_value = old_value._tag_ptr._ptr->_next;
		} else {
			new_value._tag_ptr._ptr = NULL;
			new_value._tag_ptr._tag = 0;
			return new_value;
		}
	} while (!__atomic_compare_exchange_n(&(atomic_memory_ready._integral_type),
			&(old_value._integral_type), new_value._integral_type, 1,
			__ATOMIC_RELAXED /* read and write */,
			__ATOMIC_RELAXED /* read */));

	old_value._tag_ptr._tag++; //TODO: check for wrap around and throw error
	return old_value;
}

void atomic_memory_free(union atomic_memory_block_tag_ptr block) {
	union atomic_memory_block_tag_ptr old_value;
	enum atomic_memory_size size = block._tag_ptr._ptr->_size;

	old_value._integral_type = __atomic_load_n(
			&(atomic_memory_cache[size]._integral_type), __ATOMIC_RELAXED);
	do {
		block._tag_ptr._ptr->_next = old_value;
	} while (!__atomic_compare_exchange_n(
			&(atomic_memory_cache[size]._integral_type),
			&(old_value._integral_type), block._integral_type, 1,
			__ATOMIC_RELAXED /* read and write */,
			__ATOMIC_RELAXED /* read */));
}
