#include "atomic_memory_cache.h"

#include "libiotrace_config.h"

//#define FALL_BACK_TO_SYNC

// TODO: test if invalidate is faster than CAS-loop?
//#define ATOMIC_MEMORY_INVALID_PTR(x) ((struct atomic_memory_block *) 1)
//#define ATOMIC_MEMORY_IS_PTR_VALID(x) ((struct atomic_memory_block *) x)

/**
 * Thread local cache of usable free memory blocks belonging to the current
 * thread.
 *
 * Each array element holds a pointer to the first element of a linked list of
 * blocks. Each list holds only blocks of one size. The size of blocks in a
 * list is equal to the size given through the enum \a atomic_memory_size
 * equal to the index of the array element.
 *
 * This cache is only manipulated by the current thread and has therefore no
 * concurrent accesses from other threads. So it's not necessary to guard it
 * with locks or atomic instructions.
 */
static ATTRIBUTE_THREAD union atomic_memory_block_tag_ptr atomic_memory_tls_cache[atomic_memory_size_count] =
{	{	._tag_ptr._ptr = NULL, ._tag_ptr._tag = 0}};
/**
 * Thread local cache of usable free memory blocks belonging to other threads.
 *
 * Each array element holds a pointer to the first element of a linked list of
 * blocks. Each list holds only blocks of one size belonging to one thread.
 * The size of blocks in a list is equal to the size given through the enum
 * \a atomic_memory_size equal to the second index of the array element. The
 * thread the blocks belonging to is given through the \a thread_id equal to
 * the first index of the array element.
 *
 * This cache is only manipulated by the current thread and has therefore no
 * concurrent accesses from other threads. So it's not necessary to guard it
 * with locks or atomic instructions.
 */
static ATTRIBUTE_THREAD union atomic_memory_block_tag_ptr atomic_memory_tls_free_start[ATOMIC_MEMORY_MAX_THREADS_PER_PROCESS][atomic_memory_size_count] =
{	{	{	._tag_ptr._ptr = NULL, ._tag_ptr._tag = 0}}};
/**
 * Thread local cache of last elements of linked lists stored in
 * \a atomic_memory_tls_free_start.
 *
 * Each array element holds a pointer to the last element of the list in
 * \a atomic_memory_tls_free_start with the corresponding indices.
 *
 * This cache is only manipulated by the current thread and has therefore no
 * concurrent accesses from other threads. So it's not necessary to guard it
 * with locks or atomic instructions.
 */
static ATTRIBUTE_THREAD union atomic_memory_block_tag_ptr atomic_memory_tls_free_end[ATOMIC_MEMORY_MAX_THREADS_PER_PROCESS][atomic_memory_size_count] =
{	{	{	._tag_ptr._ptr = NULL, ._tag_ptr._tag = 0}}};
/**
 * Thread local cache of count of blocks stored in each linked lists in
 * \a atomic_memory_tls_free_start.
 *
 * Each array element holds the count of blocks inside the list in
 * \a atomic_memory_tls_free_start with the corresponding indices.
 *
 * This cache is only manipulated by the current thread and has therefore no
 * concurrent accesses from other threads. So it's not necessary to guard it
 * with locks or atomic instructions.
 */
static ATTRIBUTE_THREAD size_t free_count[ATOMIC_MEMORY_MAX_THREADS_PER_PROCESS][atomic_memory_size_count] =
		{ { 0 } };
/**
 * Global cache (per process) of usable free memory blocks.
 *
 * Each array element holds a pointer to the first element of a linked list of
 * blocks. Each list holds only blocks of one size belonging to one thread.
 * The size of blocks in a list is equal to the size given through the enum
 * \a atomic_memory_size equal to the second index of the array element. The
 * thread the blocks belonging to is given through the \a thread_id equal to
 * the first index of the array element.
 *
 * This cache is manipulated by different threads and must therefore be
 * guarded against concurrent accesses.
 */
static union atomic_memory_block_tag_ptr atomic_memory_cache[ATOMIC_MEMORY_MAX_THREADS_PER_PROCESS][atomic_memory_size_count] =
		{ { { ._tag_ptr._ptr = NULL, ._tag_ptr._tag = 0 } } };
/**
 * Global cache (per process) of filled and ready to consume memory blocks.
 *
 * Holds a pointer to the first element of a linked list of blocks. Each block
 * could have a different size and could be filled from a different thread.
 *
 * This cache is manipulated by different threads and must therefore be
 * guarded against concurrent accesses.
 */
static union atomic_memory_block_tag_ptr atomic_memory_ready = {
		._tag_ptr._ptr = NULL, ._tag_ptr._tag = 0 };

/**
 * Global buffer (per process) for creating new memory blocks.
 *
 * Created blocks are not returned to this buffer. Instead they are hold in
 * global and thread local caches. This rule makes the buffer handling easy
 * and fast (no real free, no fragmentation and only one pointer to free
 * memory necessary).
 *
 * To build fast linked lists without an additional dereferencing there is a
 * pointer to a following block at the beginning of each new memory block
 * (see struct \a atomic_memory_block). To prevent the ABA-problem this
 * pointer consists of a pointer and a tag (see union
 * \a atomic_memory_block_tag_ptr) and needs therefore 16 bytes of memory.
 * For fast access to this pointer the memory has to be aligned at least to
 * the size of the pointer. E.g. for a x86 architecture the compiler can emit
 * a movdqa instruction if the alignment permits it.
 */
static char atomic_memory_buffer[ATOMIC_MEMORY_BUFFER_SIZE]__attribute__((aligned (ATOMIC_MEMORY_ALIGNMENT)));
/**
 * Pointer to free memory in \a atomic_memory_buffer.
 *
 * Memory is never returned to the buffer (see \a atomic_memory_buffer). So
 * for creating a new block this pointer has only to be increased. This
 * prevents the ABA-problem.
 *
 * Each new block created from the free memory has to be aligned to the size
 * of union \a atomic_memory_block_tag_ptr (see \a atomic_memory_buffer for an
 * explanation). So this pointer has to be increased by a multiple of
 * sizeof(union atomic_memory_block_tag_ptr) only.
 *
 * This pointer is manipulated by different threads and must therefore be
 * guarded against concurrent accesses.
 */
static char *atomic_memory_buffer_pos = atomic_memory_buffer;

/**
 * Global (per process) counter of active threads.
 *
 * Has to be incremented for each thread which calls the function
 * \c atomic_memory_alloc(). This enables creating a new unique \a thread_id
 * for storing the new created block in the different caches.
 *
 * This counter is manipulated by different threads and must therefore be
 * guarded against concurrent accesses.
 */
static int32_t thread_count = -1;
/**
 * Thread local unique id.
 *
 * This id has to be created during first call of function
 * \c atomic_memory_alloc() from a thread. After that the id can be used to
 * store created memory blocks in caches. For that each block stores a copy of
 * the id in the field \a _thread inside the block (see struct
 * \a atomic_memory_block). This copy is used as index to store the block in
 * the caches. So each block is only available for one thread during further
 * calls to \c atomic_memory_alloc() which are served from caches instead of
 * the memory buffer \a atomic_memory_buffer.
 *
 * This id is only manipulated by the current thread and has therefore no
 * concurrent accesses from other threads. So it's not necessary to guard it
 * with locks or atomic instructions.
 */
static ATTRIBUTE_THREAD int32_t thread_id = -1;

// TODO: get atomic_memory_buffer from malloc instead of using stack
static void init_on_load() ATTRIBUTE_CONSTRUCTOR;

/**
 * Creates a new memory block from the buffer \a atomic_memory_buffer.
 *
 * Checks if enough space for \a size in \a atomic_memory_buffer is available
 * and increments the pointer \a atomic_memory_buffer_pos by \a size via
 * atomic instruction. Initializes the so reserved memory as an memory block
 * and generates therefore a new \a thread_id (via atomic increment of
 * \a thread_count) if needed. Aborts the program if \a thread_count gets
 * greater than \c ATOMIC_MEMORY_MAX_THREADS_PER_PROCESS.
 *
 * @param[in]  size       enum which gives the length in bytes of the new
 *                        memory block (see array \a atomic_memory_sizes)
 * @return pointer to a new memory block on success, NULL if not enough space
 *         in \a atomic_memory_buffer is available
 */
static inline struct atomic_memory_block* atomic_memory_new_block(
		enum atomic_memory_size size)__attribute__((always_inline))__attribute__((warn_unused_result))__attribute__((malloc));
/**
 * Creates multiple memory blocks at once.
 *
 * Checks if enough space for \a count times of \a size in
 * \a atomic_memory_buffer is available and increments the pointer
 * \a atomic_memory_buffer_pos by \a count * \a size via atomic instruction.
 * Initializes the so reserved memory as \a count memory blocks. The multiple
 * blocks are linked together via \a atomic_memory_block_tag_ptr to a linked
 * list.
 *
 * Should only be used after the first call to \c atomic_memory_new_block()
 * from the current thread was already made. This is because the new created
 * \a thread_id is needed.
 *
 * @param[in]  size       enum which gives the length in bytes of each new
 *                        memory block (see array \a atomic_memory_sizes)
 * @param[in]  count      count of new memory blocks
 * @return \a atomic_memory_block_tag_ptr to a linked list with all new memory
 *         blocks on success, \a atomic_memory_block_tag_ptr with field
 *         \c _ptr set to NULL if not enough space in \a atomic_memory_buffer
 *         is available
 */
static inline union atomic_memory_block_tag_ptr atomic_memory_new_block_list(
		enum atomic_memory_size size, uint32_t count)__attribute__((always_inline))__attribute__((warn_unused_result));

/**
 * Compare and swap 16 bytes
 *
 * Compares value of \a ptr with value of \a expected. If the values are
 * equal, \a desired is written to the value of \a ptr. If they are unequal,
 * the value of \a ptr is written to the value of \a expected. All that is
 * done atomic.
 *
 * @param[in,out] ptr              pointer to value which should be compared
 *                                 with value of \a expected and changed to
 *                                 \a desired
 * @param[in,out] expected         pointer to expected value which will be
 *                                 overwritten with value of \a ptr if it is
 *                                 unequal to value of \a ptr
 * @param[in]     desired          desired value which is written to value of
 *                                 \a ptr if value of \a ptr was equal to
 *                                 value of \a expected
 * @param[in]     weak             true for weak compare exchange (can fail
 *                                 also if value of \a ptr was equal to value
 *                                 of \a expected; is faster on some
 *                                 architectures, will be ignored on others)
 *                                 and false for strong compare exchange
 *                                 (never fails if value of \a ptr was equal
 *                                 to value of \a expected)
 * @param[in]     success_memorder memory order if value of \a ptr was equal
 *                                 to value if \a expected
 * @param[in]     failure_memorder memory order if value of \a ptr was not
 *                                 equal to value of \a expected
 * @return 1 if value of \a ptr was successfully changed to \a desired, 0 if
 *         value of \a ptr wasn't changed and value of \a expected was set to
 *         actual value of \a ptr
 */
static inline char atomic_compare_exchange_16(unsigned __int128 *ptr,
		unsigned __int128 *expected, unsigned __int128 desired, char weak,
		int success_memorder, int failure_memorder)__attribute__((always_inline));

/**
 * Atomic load of 16 bytes
 *
 * @param[in]     ptr              pointer to load from
 * @param[in]     memorder         memory order for load
 * @return value of \a ptr
 */
static inline unsigned __int128 atomic_load_16(unsigned __int128 *ptr,
		int memorder)__attribute__((always_inline));

static void init_on_load() {
	// TODO: malloc
}

static inline char atomic_compare_exchange_16(unsigned __int128 *ptr,
		unsigned __int128 *expected, unsigned __int128 desired,
		char weak __attribute__((unused)),
		int success_memorder __attribute__((unused)),
		int failure_memorder __attribute__((unused))) {
#ifdef FALL_BACK_TO_SYNC
	/* fall back to __sync builtin to prevent external call to libatomic.so
	 * (fall back needs an additional compare but no call) */
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

static inline unsigned __int128 atomic_load_16(unsigned __int128 *ptr,
		int memorder __attribute__((unused))) {
#ifdef FALL_BACK_TO_SYNC
	/* fall back to __sync builtin to prevent external call to libatomic.so */
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

		/* increase tag of pointer to prevent ABA-problem (every reuse of
		 * memory blocks from cache inserts a formerly known pointer back
		 * into the workflow) */
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

			/* increase tag of pointer to prevent ABA-problem (every reuse of
			 * memory blocks from cache inserts a formerly known pointer back
			 * into the workflow) */
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

static struct atomic_memory_block* atomic_memory_new_block(
		enum atomic_memory_size size) {
	char *old_value;
	char *new_value;
	size_t real_size = atomic_memory_sizes[size];

	/* atomic get needed bytes from buffer */
	old_value = __atomic_load_n(&(atomic_memory_buffer_pos), __ATOMIC_RELAXED);
	// TODO: check for __atomic_load_n
	do {
		/* check if atomic_memory_buffer has enough free memory left for
		 * needed size (do it without adding size to atomic_memory_buffer_pos
		 * to prevent wrap around during evaluation) */
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
	// TODO: check for __atomic_compare_exchange_n

	/* initialize memory block */
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

static union atomic_memory_block_tag_ptr atomic_memory_new_block_list(
		enum atomic_memory_size size, uint32_t count) {
	char *old_value;
	char *new_value;
	union atomic_memory_block_tag_ptr begin;
	union atomic_memory_block_tag_ptr end;
	union atomic_memory_block_tag_ptr tmp;
	size_t real_size;

	/* get count of needed bytes by multiplication of parameter size and count */
	if (__builtin_mul_overflow(atomic_memory_sizes[size], count, &real_size)) {
		// TODO: check for __builtin_mul_overflow
		/* we have an overflow */
		tmp._tag_ptr._ptr = NULL;
		return tmp; //TODO: abort() ???
	}

	/* atomic get needed bytes from buffer */
	old_value = __atomic_load_n(&(atomic_memory_buffer_pos), __ATOMIC_RELAXED);
	do {
		/* check if atomic_memory_buffer has enough free memory left for
		 * needed size (do it without adding size to atomic_memory_buffer_pos
		 * to prevent wrap around during evaluation) */
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

	/* initialize each memory block and link all blocks together to a linked
	 * list */
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
		if (thread == thread_id) {
			/* local cache of free blocks belonging to other threads has no
			 * entry for size and thread id of current block because the
			 * current block belongs to the this thread itself
			 * => add current block direct to local cache of free blocks (and
			 * not to local cache of free blocks belonging to other threads) */
			old_value._integral_type =
					atomic_memory_tls_cache[size]._integral_type;
			block._tag_ptr._ptr->_next = old_value;
			atomic_memory_tls_cache[size]._integral_type = block._integral_type;
			return;
		} else {
			/* local cache of free blocks for other threads has no entry for
			 * size and thread id of current block so current block will be
			 * added as first element of a new linked list
			 * => to insert this new linked list later to the global cache,
			 * the first element needs to be saved (as end of the list) */
			atomic_memory_tls_free_end[thread][size]._integral_type =
					block._integral_type;
		}
	}

	/* add current block to local cache of free blocks belonging to other
	 * threads */
	old_value._integral_type =
			atomic_memory_tls_free_start[thread][size]._integral_type;
	block._tag_ptr._ptr->_next = old_value;
	atomic_memory_tls_free_start[thread][size]._integral_type =
			block._integral_type;

	free_count[thread][size]++;

	if (ATOMIC_MEMORY_CACHE_SIZE <= free_count[thread][size]) {
		/* local cache of free blocks belonging to other threads for size and
		 * thread id of current block is full
		 * => copy linked list to global cache (make it available for further
		 * allocations) */
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

		atomic_memory_tls_free_start[thread][size]._tag_ptr._ptr = NULL;
		free_count[thread][size] = 0;
	}
}
