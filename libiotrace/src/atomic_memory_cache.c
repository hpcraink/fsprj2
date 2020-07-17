#include "atomic_memory_cache.h"

#include "libiotrace_config.h"

//#define FALL_BACK_TO_SYNC
#define THREAD_LOCAL_CACHE

#ifdef THREAD_LOCAL_CACHE
static ATTRIBUTE_THREAD union atomic_memory_block_tag_ptr atomic_memory_cache[atomic_memory_size_count] =
		{ { ._tag_ptr._ptr = NULL, ._tag_ptr._tag = 0 } };
#else
static union atomic_memory_block_tag_ptr atomic_memory_cache[atomic_memory_size_count] =
		{ { ._tag_ptr._ptr = NULL, ._tag_ptr._tag = 0 } };
#endif
static union atomic_memory_block_tag_ptr atomic_memory_ready = {
		._tag_ptr._ptr = NULL, ._tag_ptr._tag = 0 };
static char atomic_memory_buffer[ATOMIC_MEMORY_BUFFER_SIZE]__attribute__((aligned (ATOMIC_MEMORY_ALIGNMENT)));
static char *atomic_memory_buffer_pos = atomic_memory_buffer;

inline struct atomic_memory_block* atomic_memory_new_block(
		enum atomic_memory_size size)__attribute__((always_inline))__attribute__((warn_unused_result))__attribute__((malloc));

static inline char atomic_compare_exchange_16(unsigned __int128 *ptr,
		unsigned __int128 *expected, unsigned __int128 desired, char weak,
		int success_memorder, int failure_memorder)__attribute__((always_inline));

static inline unsigned __int128 atomic_load_16(unsigned __int128 *ptr,
		int memorder)__attribute__((always_inline));

/* fall back to __sync builtin to prevent external call to libatomic.so
 * (fall back needs an additional compare but no call) */
static inline char atomic_compare_exchange_16(unsigned __int128 *ptr,
		unsigned __int128 *expected, unsigned __int128 desired,
		char weak __attribute__((unused)),
		int success_memorder __attribute__((unused)),
		int failure_memorder __attribute__((unused))) {
#ifdef FALL_BACK_TO_SYNC
	unsigned __int128 cmpval = *expected;
	unsigned __int128 oldval = __sync_val_compare_and_swap_16(ptr, cmpval,
			desired);
	if (oldval == cmpval)
		return 1;
	*expected = oldval;
	return 0;
#else
	return __atomic_compare_exchange_n(ptr, expected, desired, weak,
			success_memorder /* read and write */, failure_memorder /* read */);
#endif
}

/* fall back to __sync builtin to prevent external call to libatomic.so */
static inline unsigned __int128 atomic_load_16(unsigned __int128 *ptr,
		int memorder __attribute__((unused))) {
#ifdef FALL_BACK_TO_SYNC
	return __sync_add_and_fetch (ptr, 0);
#else
	return __atomic_load_n(ptr, memorder);
#endif
}

union atomic_memory_block_tag_ptr atomic_memory_alloc(
		enum atomic_memory_size size) {
	union atomic_memory_block_tag_ptr old_value;
	union atomic_memory_block_tag_ptr new_value;

#ifdef THREAD_LOCAL_CACHE
	old_value._integral_type = atomic_memory_cache[size]._integral_type;
#else
	old_value._integral_type = atomic_load_16(
			&(atomic_memory_cache[size]._integral_type), __ATOMIC_RELAXED);
	do {
#endif
		if (NULL != old_value._tag_ptr._ptr) {
			new_value = old_value._tag_ptr._ptr->_next;
		} else {
			new_value._tag_ptr._ptr = atomic_memory_new_block(size);
			new_value._tag_ptr._tag = 0;
			return new_value;
		}
#ifdef THREAD_LOCAL_CACHE
	atomic_memory_cache[size]._integral_type = new_value._integral_type;
#else
	} while (!atomic_compare_exchange_16(
			&(atomic_memory_cache[size]._integral_type),
			&(old_value._integral_type), new_value._integral_type, 1,
			__ATOMIC_RELAXED /* read and write */,
			__ATOMIC_RELAXED /* read */));
#endif

	old_value._tag_ptr._tag++; //TODO: check for wrap around and throw error
	return old_value;
}

struct atomic_memory_block* atomic_memory_new_block(
		enum atomic_memory_size size) {
	char *old_value;
	char *new_value;
	size_t real_size = atomic_memory_sizes[size];

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

	old_value._integral_type = atomic_load_16(
			&(atomic_memory_ready._integral_type), __ATOMIC_RELAXED);
	do {
		block._tag_ptr._ptr->_next = old_value;
	} while (!atomic_compare_exchange_16(&(atomic_memory_ready._integral_type),
			&(old_value._integral_type), block._integral_type, 1,
			__ATOMIC_RELAXED /* read and write */,
			__ATOMIC_RELAXED /* read */));
}

union atomic_memory_block_tag_ptr atomic_memory_pop() {
	union atomic_memory_block_tag_ptr old_value;
	union atomic_memory_block_tag_ptr new_value;

	old_value._integral_type = atomic_load_16(
			&(atomic_memory_ready._integral_type), __ATOMIC_RELAXED);
	do {
		if (NULL != old_value._tag_ptr._ptr) {
			new_value = old_value._tag_ptr._ptr->_next;
		} else {
			new_value._tag_ptr._ptr = NULL;
			new_value._tag_ptr._tag = 0;
			return new_value;
		}
	} while (!atomic_compare_exchange_16(&(atomic_memory_ready._integral_type),
			&(old_value._integral_type), new_value._integral_type, 1,
			__ATOMIC_RELAXED /* read and write */,
			__ATOMIC_RELAXED /* read */));

	//old_value._tag_ptr._tag++; //TODO: check for wrap around and throw error
	return old_value;
}

void atomic_memory_free(union atomic_memory_block_tag_ptr block) {
	union atomic_memory_block_tag_ptr old_value;
	enum atomic_memory_size size = block._tag_ptr._ptr->_size;

#ifdef THREAD_LOCAL_CACHE
	old_value._integral_type = atomic_memory_cache[size]._integral_type;
#else
	old_value._integral_type = atomic_load_16(
			&(atomic_memory_cache[size]._integral_type), __ATOMIC_RELAXED);
	do {
#endif
		block._tag_ptr._ptr->_next = old_value;
#ifdef THREAD_LOCAL_CACHE
	atomic_memory_cache[size]._integral_type = block._integral_type;
#else
	} while (!atomic_compare_exchange_16(
			&(atomic_memory_cache[size]._integral_type),
			&(old_value._integral_type), block._integral_type, 1,
			__ATOMIC_RELAXED /* read and write */,
			__ATOMIC_RELAXED /* read */));
#endif
}
