#include "atomic_memory_cache.h"

#include "libiotrace_config.h"

//#define FALL_BACK_TO_SYNC

// TODO: test if invalidate is faster than CAS-loop?
#define ATOMIC_MEMORY_INVALID_PTR(x) ((struct atomic_memory_block *) 1)
#define ATOMIC_MEMORY_IS_PTR_VALID(x) ((struct atomic_memory_block *) x)

static ATTRIBUTE_THREAD union atomic_memory_block_tag_ptr atomic_memory_tls_cache[atomic_memory_size_count] =
{	{	._tag_ptr._ptr = NULL, ._tag_ptr._tag = 0}};
static ATTRIBUTE_THREAD union atomic_memory_block_tag_ptr atomic_memory_tls_free_start[ATOMIC_MEMORY_MAX_THREADS_PER_PROCESS][atomic_memory_size_count] =
{	{	{	._tag_ptr._ptr = NULL, ._tag_ptr._tag = 0}}};
static ATTRIBUTE_THREAD union atomic_memory_block_tag_ptr atomic_memory_tls_free_end[ATOMIC_MEMORY_MAX_THREADS_PER_PROCESS][atomic_memory_size_count] =
{	{	{	._tag_ptr._ptr = NULL, ._tag_ptr._tag = 0}}};
ATTRIBUTE_THREAD size_t free_count[ATOMIC_MEMORY_MAX_THREADS_PER_PROCESS][atomic_memory_size_count] =
		{ { 0 } };
static union atomic_memory_block_tag_ptr atomic_memory_cache[ATOMIC_MEMORY_MAX_THREADS_PER_PROCESS][atomic_memory_size_count] =
		{ { { ._tag_ptr._ptr = NULL, ._tag_ptr._tag = 0 } } };
static union atomic_memory_block_tag_ptr atomic_memory_ready = {
		._tag_ptr._ptr = NULL, ._tag_ptr._tag = 0 };

static char atomic_memory_buffer[ATOMIC_MEMORY_BUFFER_SIZE]__attribute__((aligned (ATOMIC_MEMORY_ALIGNMENT)));
static char *atomic_memory_buffer_pos = atomic_memory_buffer;

static int32_t thread_count = -1;
static ATTRIBUTE_THREAD int32_t thread_id = -1;

inline struct atomic_memory_block* atomic_memory_new_block(
		enum atomic_memory_size size)__attribute__((always_inline))__attribute__((warn_unused_result))__attribute__((malloc));
inline union atomic_memory_block_tag_ptr atomic_memory_new_block_list(
		enum atomic_memory_size size, uint32_t count)__attribute__((always_inline))__attribute__((warn_unused_result));

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

	old_value._integral_type = atomic_memory_tls_cache[size]._integral_type;
	if (NULL != old_value._tag_ptr._ptr) {
		/* thread local cache holds an entry with requested size: return it */
		new_value = old_value._tag_ptr._ptr->_next;

		atomic_memory_tls_cache[size]._integral_type = new_value._integral_type;

		old_value._tag_ptr._tag++; //TODO: check for wrap around and throw error
		return old_value;
	} else {
		old_value._integral_type = atomic_load_16(
				&(atomic_memory_cache[thread_id][size]._integral_type),
				__ATOMIC_RELAXED);
		if (NULL != old_value._tag_ptr._ptr) {
			/* thread local cache is empty and central cache for this thread
			 * has elements of requested size: fill local cache with entries
			 * from central cache and return element from local cache */
			do {
				new_value._tag_ptr._ptr = NULL;
			} while (!atomic_compare_exchange_16(
					&(atomic_memory_cache[thread_id][size]._integral_type),
					&(old_value._integral_type), new_value._integral_type, 1,
					__ATOMIC_RELAXED /* read and write */,
					__ATOMIC_RELAXED /* read */));

			atomic_memory_tls_cache[size]._integral_type =
					old_value._tag_ptr._ptr->_next._integral_type;

			old_value._tag_ptr._tag++; //TODO: check for wrap around and throw error
			return old_value;
		} else {
			/* thread local cache and central cache for this thread are empty:
			 * create a new memory block */
			new_value._tag_ptr._ptr = atomic_memory_new_block(size);
			new_value._tag_ptr._tag = 0;

			/* fill local cache with additional created new blocks */
			if (0 < ATOMIC_MEMORY_CACHE_SIZE) {
				atomic_memory_tls_cache[size] = atomic_memory_new_block_list(
						size,
						ATOMIC_MEMORY_CACHE_SIZE);
//				if (NULL == atomic_memory_tls_cache[size]._tag_ptr._ptr) {
//					fprintf(stderr, "not enough memory available (in atomic_memory_buffer)"); // TODO: use real fprintf and no wrapper
//					abort();
//				}
			}

			return new_value;
		}
	}
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
	if (0 > thread_id) {
		/* get an index to an unique place in intern cache per thread */
		thread_id = __atomic_add_fetch(&thread_count, 1, __ATOMIC_RELAXED);
		if (ATOMIC_MEMORY_MAX_THREADS_PER_PROCESS <= thread_id) {
			/* there are more threads than places in intern cache */
			fprintf(stderr, "more threads than places in intern cache"); // TODO: use real fprintf and no wrapper
			abort();
		}
	}
	((struct atomic_memory_block*) old_value)->_thread = thread_id;
	return (struct atomic_memory_block*) old_value;
}

union atomic_memory_block_tag_ptr atomic_memory_new_block_list(
		enum atomic_memory_size size, uint32_t count) {
	char *old_value;
	char *new_value;
	union atomic_memory_block_tag_ptr begin;
	union atomic_memory_block_tag_ptr end;
	union atomic_memory_block_tag_ptr tmp;
	size_t real_size = atomic_memory_sizes[size] * count; // TODO: check for wrap around/overflow

	old_value = __atomic_load_n(&(atomic_memory_buffer_pos), __ATOMIC_RELAXED);
	do {
		if (atomic_memory_buffer + ATOMIC_MEMORY_BUFFER_SIZE - old_value
				>= real_size) {
			new_value = (void*) (old_value + real_size);
		} else {
			tmp._tag_ptr._ptr = NULL;
			return tmp; //TODO: abort() ???
		}
	} while (!__atomic_compare_exchange_n(&(atomic_memory_buffer_pos),
			&old_value, (void*) new_value, 1,
			__ATOMIC_RELAXED /* read and write */,
			__ATOMIC_RELAXED /* read */));

	((struct atomic_memory_block*) old_value)->_size = size;
	((struct atomic_memory_block*) old_value)->_thread = thread_id;
	begin._tag_ptr._ptr = ((struct atomic_memory_block*) old_value);
	begin._tag_ptr._tag = 0;
	end._integral_type = begin._integral_type;
	for (uint32_t i = 1; i < count; i++) {
		((struct atomic_memory_block*) (old_value
				+ (i * atomic_memory_sizes[size])))->_size = size;
		((struct atomic_memory_block*) (old_value
				+ (i * atomic_memory_sizes[size])))->_thread = thread_id;

		tmp._tag_ptr._ptr = ((struct atomic_memory_block*) (old_value
				+ (i * atomic_memory_sizes[size])));
		tmp._tag_ptr._tag = 0;
		end._tag_ptr._ptr->_next._integral_type = tmp._integral_type;
		end._integral_type = tmp._integral_type;
	}
	tmp._tag_ptr._ptr = NULL;
	end._tag_ptr._ptr->_next._integral_type = tmp._integral_type;
	return begin;
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

	return old_value;
}

void atomic_memory_free(union atomic_memory_block_tag_ptr block) {
	union atomic_memory_block_tag_ptr old_value;
	enum atomic_memory_size size = block._tag_ptr._ptr->_size;
	int32_t thread = block._tag_ptr._ptr->_thread;

	if (0 == free_count[thread][size]) {
		atomic_memory_tls_free_end[thread][size]._integral_type =
				block._integral_type;
	}

	old_value._integral_type =
			atomic_memory_tls_free_start[thread][size]._integral_type;
	block._tag_ptr._ptr->_next = old_value;
	atomic_memory_tls_free_start[thread][size]._integral_type =
			block._integral_type;

	free_count[thread][size]++;

	if (ATOMIC_MEMORY_CACHE_SIZE <= free_count[thread][size]) {
		old_value._integral_type = atomic_load_16(
				&(atomic_memory_cache[thread][size]._integral_type),
				__ATOMIC_RELAXED);
		do {
			atomic_memory_tls_free_end[thread][size]._tag_ptr._ptr->_next =
					old_value;
		} while (!atomic_compare_exchange_16(
				&(atomic_memory_cache[thread][size]._integral_type),
				&(old_value._integral_type),
				atomic_memory_tls_free_start[thread][size]._integral_type, 1,
				__ATOMIC_RELAXED /* read and write */,
				__ATOMIC_RELAXED /* read */));

		free_count[thread][size] = 0;
	}
}
