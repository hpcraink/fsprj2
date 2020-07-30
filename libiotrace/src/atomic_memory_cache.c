#include "atomic_memory_cache.h"

#include "libiotrace_config.h"

#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */
#include <unistd.h>

//#define FALL_BACK_TO_SYNC

//#define ATOMIC_MEMORY_CACHE_PER_NODE

#ifdef ATOMIC_MEMORY_CACHE_PER_NODE
#  define ATOMIC_MEMORY_CACHE_GLOBAL_STRUCT (*atomic_memory_global)
#else
#  define ATOMIC_MEMORY_CACHE_GLOBAL_STRUCT (atomic_memory_global)
#endif

#define ATOMIC_MEMORY_CACHE_ABORT(message) do { \
                                               fprintf(stderr, message"\n"); /* TODO: use real fprintf and no wrapper */ \
                                               abort(); \
                                           } while(0)

struct atomic_memory_region {
	/**
	 * Global cache (per process or per node, see
	 * #ATOMIC_MEMORY_CACHE_PER_NODE) of usable free memory blocks.
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
	union atomic_memory_block_tag_ptr atomic_memory_cache[ATOMIC_MEMORY_MAX_THREADS_PER_PROCESS][atomic_memory_size_count];

	/**
	 * Global cache (per process or per node, see
	 * #ATOMIC_MEMORY_CACHE_PER_NODE) of filled and ready to consume memory blocks.
	 *
	 * Holds a pointer to the first element of a linked list of blocks. Each block
	 * could have a different size and could be filled from a different thread.
	 *
	 * This cache is manipulated by different threads and must therefore be
	 * guarded against concurrent accesses.
	 */
	union atomic_memory_block_tag_ptr atomic_memory_ready;

	/**
	 * Global buffer (per process or per node, see
	 * #ATOMIC_MEMORY_CACHE_PER_NODE) for creating new memory blocks.
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
	uint8_t atomic_memory_buffer[ATOMIC_MEMORY_BUFFER_SIZE]__attribute__((aligned (ATOMIC_MEMORY_ALIGNMENT)));

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
	uint8_t *atomic_memory_buffer_pos;

	/**
	 * Global (per process or per node, see
	 * #ATOMIC_MEMORY_CACHE_PER_NODE) counter of active threads.
	 *
	 * Has to be incremented for each thread which calls the function
	 * \c atomic_memory_alloc(). This enables creating a new unique \a thread_id
	 * for storing the new created block in the different caches.
	 *
	 * This counter is manipulated by different threads and must therefore be
	 * guarded against concurrent accesses.
	 */
	int32_t thread_count;
};

#ifdef ATOMIC_MEMORY_CACHE_PER_NODE
#  define ATOMIC_MEMORY_CACHE_SHM_NUMBER "/libiotrace-atomic-memory-cache-number"
#  define ATOMIC_MEMORY_CACHE_SHM "/libiotrace-atomic-memory-cache-"
#  define ATOMIC_MEMORY_CACHE_SHM_NAME_LENGTH 43

struct atomic_memory_region_init {
	int32_t region_init;
	int32_t region_number;
};

static struct atomic_memory_region *atomic_memory_global;

#else

static struct atomic_memory_region atomic_memory_global;
#endif

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
{	{	.tag_ptr.ptr = NULL, .tag_ptr._tag = 0}};
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
{	{	{	.tag_ptr.ptr = NULL, .tag_ptr._tag = 0}}};
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
{	{	{	.tag_ptr.ptr = NULL, .tag_ptr._tag = 0}}};
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
 *         \c ptr set to NULL if not enough space in \a atomic_memory_buffer
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
static inline uint8_t atomic_compare_exchange_16(unsigned __int128 *ptr,
		unsigned __int128 *expected, unsigned __int128 desired, uint8_t weak,
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

/**
 * TODO
 */
static inline void init_cache()__attribute__((always_inline));

/**
 * TODO
 */
static void init_on_load() {
#ifdef ATOMIC_MEMORY_CACHE_PER_NODE
	int fd;
	struct atomic_memory_region_init *memory_region_init;
	int32_t region_number;
	int32_t region_init;
	char shared_memory_name[ATOMIC_MEMORY_CACHE_SHM_NAME_LENGTH];

	// TODO: use real functions and no wrapper

	/* get an existing or create a new shared memory atomic_memory_region_init */
	fd = shm_open(ATOMIC_MEMORY_CACHE_SHM_NUMBER, O_RDWR | O_CREAT,
	S_IRUSR | S_IWUSR);
	if (0 > fd) {
		ATOMIC_MEMORY_CACHE_ABORT("shared memory could not be created");
	}

	/* Set the size of the region:
	 *
	 * After creating a new shared memory region, the size of the region is
	 * 0, so the first ftruncate increases the size. The POSIX specification
	 * is unclear about initialization of shared memory objects during an
	 * increase of size with ftruncate. The man page for ftruncate specifies
	 * that the extended parts of shared memory will be initialized to null
	 * bytes ('\0'). The further logic depends on that initialization. So it
	 * could be prone to errors on non Linux implementations of ftruncate. */
	if (0 != ftruncate(fd, sizeof(struct atomic_memory_region_init))) {
		ATOMIC_MEMORY_CACHE_ABORT("set size of shared memory was unsuccessful");
	}

	/* map the region into the virtual address space of this process */
	memory_region_init = mmap(NULL, sizeof(struct atomic_memory_region_init),
	PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (MAP_FAILED == memory_region_init) {
		ATOMIC_MEMORY_CACHE_ABORT("mmap of shared memory was unsuccessful");
	}
	close(fd);

	/* get number of created and initialized shared memory */
	region_init = __atomic_load_n(&(memory_region_init->region_init),
	/* ensure that each read and write to memory_region_init is
	 * sequentially consistent */
	__ATOMIC_SEQ_CST);

	if (0 == region_init) {
		/* shared memory for cache is not initialized: do it */

		/* get a unique number for creating and initializing shared memory from
		 * this process */
		region_number = __atomic_add_fetch(&(memory_region_init->region_number),
				1,
				/* ensure that each read and write to memory_region_init is
				 * sequentially consistent */
				__ATOMIC_SEQ_CST);

		/* Build a unique name for shared memory created from this process:
		 *
		 * needs 32 + length of region_number as string (10) + 1 for terminating '\0'
		 * bytes in shared_memory_name */
		if (ATOMIC_MEMORY_CACHE_SHM_NAME_LENGTH < sprintf(shared_memory_name,
		ATOMIC_MEMORY_CACHE_SHM"%d", region_number)) {
			ATOMIC_MEMORY_CACHE_ABORT(
					"name of shared memory could not be created");
		}

		/* get an existing or create a new shared memory atomic_memory_global */
		fd = shm_open(shared_memory_name, O_RDWR | O_CREAT,
		S_IRUSR | S_IWUSR);
		if (0 > fd) {
			ATOMIC_MEMORY_CACHE_ABORT("shared memory could not be created");
		}

		/* Set the size of the region:
		 *
		 * see ftruncate above for further details */
		if (0 != ftruncate(fd, sizeof(struct atomic_memory_region))) {
			ATOMIC_MEMORY_CACHE_ABORT(
					"set size of shared memory was unsuccessful");
		}

		/* Map the region into the virtual address space of this process:
		 *
		 * With MAP_FIXED it is possible to give the mapping the same address in
		 * each process. This would be useful for storing pointers in the shared
		 * memory and use them to get access to other parts of the shared memory
		 * independent from the current process. But it is not guaranteed that the
		 * same address is available for mapping shared memory in each process.
		 * It's also possible that a address is already used by a other mapping
		 * and a new call to mmap with this address unmaps this existing (and
		 * further used) mapping. So MAP_FIXED is not usable.
		 * => only store relative addresses inside the shared memory region and
		 *    add value of atomic_memory_global to them during dereferencing
		 *
		 * The internal caches are build out of linked lists. These lists are
		 * linked via 16 byte pointers (pointer and a tag to prevent the
		 * ABA-problem). To guarantee fast access to this pointers the memory has
		 * to be at least aligned to 16. The POSIX specification says nothing
		 * about the alignment of the returned pointer from mmap if the flag
		 * MAP_FIXED isn't used (it's implementation-defined). The linux man page
		 * also says nothing about it if mmap is called with NULL as a address.
		 * The man page only says that a page boundary will be used if MAP_FIXED
		 * isn't set and a address for the mapping is provided as a hint for
		 * placing the mapping. This suggests that the address will be align to
		 * the page boundary (which is a multiple of 16). TODO: ensure the alignment */
		atomic_memory_global = mmap(NULL, sizeof(struct atomic_memory_region),
		PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
		if (MAP_FAILED == atomic_memory_global) {
			ATOMIC_MEMORY_CACHE_ABORT("mmap of shared memory was unsuccessful");
		}
		close(fd);

		/* initialize the shared memory region */
		init_cache();

		if (__atomic_compare_exchange_n(&(memory_region_init->region_init),
				&region_init, region_number, 0,
				/* ensure that each read and write to memory_region_init is
				 * sequentially consistent */
				__ATOMIC_SEQ_CST,
				/* ensure that each read and write to memory_region_init is
				 * sequentially consistent */
				__ATOMIC_SEQ_CST)) {
			/* no other number of a initialized shared memory was set to
			 * memory_region_init->region_init, so the number of this
			 * initialized shared memory is set
			 * => atomic_memory_global contains the initialized shared memory
			 * cache */
			munmap(memory_region_init,
					sizeof(struct atomic_memory_region_init));
			return;
		} else {
			/* a other number of a initialized shared memory was set to
			 * memory_region_init->region_init, so this shared memory is no
			 * longer needed */
			munmap(atomic_memory_global, sizeof(struct atomic_memory_region));
			shm_unlink(shared_memory_name);
		}
	}

	munmap(memory_region_init, sizeof(struct atomic_memory_region_init));

	/* region_init contains the number of the initialized shared memory cache
	 * => map it */

	/* Build the name of the initialized shared memory:
	 *
	 * needs 32 + length of region_number as string (10) + 1 for terminating '\0'
	 * bytes in shared_memory_name */
	if (ATOMIC_MEMORY_CACHE_SHM_NAME_LENGTH < sprintf(shared_memory_name,
	ATOMIC_MEMORY_CACHE_SHM"%d", region_init)) {
		ATOMIC_MEMORY_CACHE_ABORT("name of shared memory could not be created");
	}

	fd = shm_open(shared_memory_name, O_RDWR, 0);
	if (0 > fd) {
		ATOMIC_MEMORY_CACHE_ABORT("shared memory could not be opened");
	}

	if (0 != ftruncate(fd, sizeof(struct atomic_memory_region))) {
		ATOMIC_MEMORY_CACHE_ABORT("set size of shared memory was unsuccessful");
	}

	atomic_memory_global = mmap(NULL, sizeof(struct atomic_memory_region),
	PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (MAP_FAILED == atomic_memory_global) {
		ATOMIC_MEMORY_CACHE_ABORT("mmap of shared memory was unsuccessful");
	}
	close(fd);

	/* later started processes need initialized atomic_memory_global and
	 * memory_region_init
	 * => don't unlink the two shared memory regions
	 * TODO: process counter in memory_region_init, so last process can unlink
	 * the two regions in dtor (this will be prone to errors if some dtor's
	 * use wrapped functions) */
#else
	init_cache();
#endif
}

static inline void init_cache() {
	for (int i = 0; i < ATOMIC_MEMORY_MAX_THREADS_PER_PROCESS; i++) {
		for (int l = 0; l < atomic_memory_size_count; l++) {
			ATOMIC_MEMORY_CACHE_GLOBAL_STRUCT.atomic_memory_cache[i][l].tag_ptr.ptr =
			NULL;
			ATOMIC_MEMORY_CACHE_GLOBAL_STRUCT.atomic_memory_cache[i][l].tag_ptr._tag =
					0;
		}
	}
	ATOMIC_MEMORY_CACHE_GLOBAL_STRUCT.atomic_memory_ready.tag_ptr.ptr = NULL;
	ATOMIC_MEMORY_CACHE_GLOBAL_STRUCT.atomic_memory_ready.tag_ptr._tag = 0;
	ATOMIC_MEMORY_CACHE_GLOBAL_STRUCT.atomic_memory_buffer_pos =
	ATOMIC_MEMORY_CACHE_GLOBAL_STRUCT.atomic_memory_buffer;
	ATOMIC_MEMORY_CACHE_GLOBAL_STRUCT.thread_count = -1;
}

static inline uint8_t atomic_compare_exchange_16(unsigned __int128 *ptr,
		unsigned __int128 *expected, unsigned __int128 desired,
		uint8_t weak __attribute__((unused)),
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
	if (NULL != old_value.tag_ptr.ptr) {

		/* thread local cache holds an entry with requested size: return it */
		new_value = old_value.tag_ptr.ptr->_next;
		atomic_memory_tls_cache[size]._integral_type = new_value._integral_type;

		/* increase tag of pointer to prevent ABA-problem (every reuse of
		 * memory blocks from cache inserts a formerly known pointer back
		 * into the workflow) */
		if (__builtin_add_overflow(old_value.tag_ptr._tag, 1,
				&(old_value.tag_ptr._tag))) {
			ATOMIC_MEMORY_CACHE_ABORT("overflow during increment of _tag");
		}

		return old_value;
	} else {
		if (-1 < thread_id) {
			old_value._integral_type =
					atomic_load_16(
							&(ATOMIC_MEMORY_CACHE_GLOBAL_STRUCT.atomic_memory_cache[thread_id][size]._integral_type),
							/* ensure that previous stores to old_value.tag_ptr.ptr->_next
							 * with __ATOMIC_RELEASE can be seen */
							__ATOMIC_ACQUIRE);
			if (NULL != old_value.tag_ptr.ptr) {

				/* thread local cache is empty and central cache for this thread
				 * has elements of requested size: fill local cache with entries
				 * from central cache and return element from local cache */
				new_value.tag_ptr.ptr = NULL;
				do {
				} while (!atomic_compare_exchange_16(
						&(ATOMIC_MEMORY_CACHE_GLOBAL_STRUCT.atomic_memory_cache[thread_id][size]._integral_type),
						&(old_value._integral_type), new_value._integral_type,
						1,
						/* ensure that previous stores to old_value.tag_ptr.ptr->_next
						 * with __ATOMIC_RELEASE can be seen */
						__ATOMIC_ACQUIRE,
						/* ensure that previous stores to old_value.tag_ptr.ptr->_next
						 * with __ATOMIC_RELEASE can be seen */
						__ATOMIC_ACQUIRE));

				atomic_memory_tls_cache[size]._integral_type =
						old_value.tag_ptr.ptr->_next._integral_type;

				/* increase tag of pointer to prevent ABA-problem (every reuse of
				 * memory blocks from cache inserts a formerly known pointer back
				 * into the workflow) */
				if (__builtin_add_overflow(old_value.tag_ptr._tag, 1,
						&(old_value.tag_ptr._tag))) {
					ATOMIC_MEMORY_CACHE_ABORT(
							"overflow during increment of _tag");
				}

				return old_value;
			}
		}

		/* thread local cache and central cache for this thread are empty:
		 * create a new memory block */
		new_value.tag_ptr.ptr = atomic_memory_new_block(size);
		new_value.tag_ptr._tag = 0;

		/* fill local cache with additional created new blocks */
		if (0 < ATOMIC_MEMORY_CACHE_SIZE) {
			atomic_memory_tls_cache[size] = atomic_memory_new_block_list(size,
			ATOMIC_MEMORY_CACHE_SIZE);
//			if (NULL == atomic_memory_tls_cache[size].tag_ptr.ptr) {
//				ATOMIC_MEMORY_CACHE_ABORT(
//						"not enough memory available (in atomic_memory_buffer)");
//			}
		}

		return new_value;
	}
}

static struct atomic_memory_block* atomic_memory_new_block(
		enum atomic_memory_size size) {
	uint8_t *old_value;
	uint8_t *new_value;
	size_t real_size = atomic_memory_sizes[size];

	/* atomic get needed bytes from buffer */
	old_value = __atomic_load_n(
			&(ATOMIC_MEMORY_CACHE_GLOBAL_STRUCT.atomic_memory_buffer_pos),
			__ATOMIC_RELAXED);
	// TODO: check for __atomic_load_n
	do {
		/* check if atomic_memory_buffer has enough free memory left for
		 * needed size (do it without adding size to atomic_memory_buffer_pos
		 * to prevent wrap around during evaluation) */
		if (ATOMIC_MEMORY_CACHE_GLOBAL_STRUCT.atomic_memory_buffer
				+ ATOMIC_MEMORY_BUFFER_SIZE - old_value >= real_size) {
			new_value = (void*) (old_value + real_size);
		} else {
			return NULL; //TODO: abort() ???
		}
	} while (!__atomic_compare_exchange_n(
			&(ATOMIC_MEMORY_CACHE_GLOBAL_STRUCT.atomic_memory_buffer_pos),
			&old_value, (void*) new_value, 1,
			__ATOMIC_RELAXED,
			__ATOMIC_RELAXED));
	// TODO: check for __atomic_compare_exchange_n

	/* initialize memory block */
	((struct atomic_memory_block*) old_value)->_size = size;
	if (0 > thread_id) {
		/* get an index to an unique place in intern cache per thread */
		thread_id = __atomic_add_fetch(
				&ATOMIC_MEMORY_CACHE_GLOBAL_STRUCT.thread_count, 1,
				__ATOMIC_RELAXED);
		if (ATOMIC_MEMORY_MAX_THREADS_PER_PROCESS <= thread_id) {
			/* there are more threads than places in intern cache */
			ATOMIC_MEMORY_CACHE_ABORT(
					"more threads than places in intern cache");
		}
	}
	((struct atomic_memory_block*) old_value)->_thread = thread_id;

	return (struct atomic_memory_block*) old_value;
}

static union atomic_memory_block_tag_ptr atomic_memory_new_block_list(
		enum atomic_memory_size size, uint32_t count) {
	uint8_t *old_value;
	uint8_t *new_value;
	union atomic_memory_block_tag_ptr begin;
	union atomic_memory_block_tag_ptr end;
	union atomic_memory_block_tag_ptr tmp;
	size_t real_size;

	/* get count of needed bytes by multiplication of parameter size and count */
	if (__builtin_mul_overflow(atomic_memory_sizes[size], count, &real_size)) {
		// TODO: check for __builtin_mul_overflow
		/* we have an overflow */
		tmp.tag_ptr.ptr = NULL;
		return tmp; //TODO: abort() ???
	}

	/* atomic get needed bytes from buffer */
	old_value = __atomic_load_n(
			&(ATOMIC_MEMORY_CACHE_GLOBAL_STRUCT.atomic_memory_buffer_pos),
			__ATOMIC_RELAXED);
	do {
		/* check if atomic_memory_buffer has enough free memory left for
		 * needed size (do it without adding size to atomic_memory_buffer_pos
		 * to prevent wrap around during evaluation) */
		if (ATOMIC_MEMORY_CACHE_GLOBAL_STRUCT.atomic_memory_buffer
				+ ATOMIC_MEMORY_BUFFER_SIZE - old_value >= real_size) {
			new_value = (void*) (old_value + real_size);
		} else {
			tmp.tag_ptr.ptr = NULL;
			return tmp; //TODO: abort() ???
		}
	} while (!__atomic_compare_exchange_n(
			&(ATOMIC_MEMORY_CACHE_GLOBAL_STRUCT.atomic_memory_buffer_pos),
			&old_value, (void*) new_value, 1,
			__ATOMIC_RELAXED,
			__ATOMIC_RELAXED));

	/* initialize each memory block and link all blocks together to a linked
	 * list */
	((struct atomic_memory_block*) old_value)->_size = size;
	((struct atomic_memory_block*) old_value)->_thread = thread_id;
	begin.tag_ptr.ptr = ((struct atomic_memory_block*) old_value);
	begin.tag_ptr._tag = 0;
	end._integral_type = begin._integral_type;
	for (uint32_t i = 1; i < count; i++) {
		((struct atomic_memory_block*) (old_value
				+ (i * atomic_memory_sizes[size])))->_size = size;
		((struct atomic_memory_block*) (old_value
				+ (i * atomic_memory_sizes[size])))->_thread = thread_id;

		tmp.tag_ptr.ptr = ((struct atomic_memory_block*) (old_value
				+ (i * atomic_memory_sizes[size])));
		tmp.tag_ptr._tag = 0;
		end.tag_ptr.ptr->_next._integral_type = tmp._integral_type;
		end._integral_type = tmp._integral_type;
	}
	tmp.tag_ptr.ptr = NULL;
	end.tag_ptr.ptr->_next._integral_type = tmp._integral_type;

	return begin;
}

void atomic_memory_push(union atomic_memory_block_tag_ptr block) {
	union atomic_memory_block_tag_ptr old_value;

	old_value._integral_type =
			atomic_load_16(
					&(ATOMIC_MEMORY_CACHE_GLOBAL_STRUCT.atomic_memory_ready._integral_type),
					__ATOMIC_RELAXED);
	do {
		block.tag_ptr.ptr->_next = old_value;
	} while (!atomic_compare_exchange_16(
			&(ATOMIC_MEMORY_CACHE_GLOBAL_STRUCT.atomic_memory_ready._integral_type),
			&(old_value._integral_type), block._integral_type, 1,
			/* ensure that store to block.tag_ptr.ptr->_next
			 * can be seen by a later load with __ATOMIC_ACQUIRE */
			__ATOMIC_RELEASE,
			__ATOMIC_RELAXED));
}

union atomic_memory_block_tag_ptr atomic_memory_pop() {
	union atomic_memory_block_tag_ptr old_value;
	union atomic_memory_block_tag_ptr new_value;

	old_value._integral_type =
			atomic_load_16(
					&(ATOMIC_MEMORY_CACHE_GLOBAL_STRUCT.atomic_memory_ready._integral_type),
					/* ensure that previous stores to old_value.tag_ptr.ptr->_next
					 * with __ATOMIC_RELEASE can be seen */
					__ATOMIC_ACQUIRE);
	do {
		if (NULL != old_value.tag_ptr.ptr) {
			new_value = old_value.tag_ptr.ptr->_next;
		} else {
			new_value.tag_ptr.ptr = NULL;
			new_value.tag_ptr._tag = 0;
			return new_value;
		}
	} while (!atomic_compare_exchange_16(
			&(ATOMIC_MEMORY_CACHE_GLOBAL_STRUCT.atomic_memory_ready._integral_type),
			&(old_value._integral_type), new_value._integral_type, 1,
			/* ensure that store to old_value.tag_ptr.ptr->_next
			 * can be seen by a later load with __ATOMIC_ACQUIRE */
			__ATOMIC_RELEASE,
			/* ensure that previous stores to old_value.tag_ptr.ptr->_next
			 * with __ATOMIC_RELEASE can be seen */
			__ATOMIC_ACQUIRE));

	return old_value;
}

void atomic_memory_free(union atomic_memory_block_tag_ptr block) {
	union atomic_memory_block_tag_ptr old_value;
	enum atomic_memory_size size = block.tag_ptr.ptr->_size;
	int32_t thread = block.tag_ptr.ptr->_thread;

	if (0 == free_count[thread][size]) {
		if (thread == thread_id) {
			/* local cache of free blocks belonging to other threads has no
			 * entry for size and thread id of current block because the
			 * current block belongs to this thread itself
			 * => add current block direct to local cache of free blocks (and
			 * not to local cache of free blocks belonging to other threads) */
			old_value._integral_type =
					atomic_memory_tls_cache[size]._integral_type;
			block.tag_ptr.ptr->_next = old_value;
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
	block.tag_ptr.ptr->_next = old_value;
	atomic_memory_tls_free_start[thread][size]._integral_type =
			block._integral_type;

	free_count[thread][size]++;

	if (ATOMIC_MEMORY_CACHE_SIZE <= free_count[thread][size]) {
		/* local cache of free blocks belonging to other threads for size and
		 * thread id of current block is full
		 * => copy linked list to global cache (make it available for further
		 * allocations) */
		old_value._integral_type =
				atomic_load_16(
						&(ATOMIC_MEMORY_CACHE_GLOBAL_STRUCT.atomic_memory_cache[thread][size]._integral_type),
						__ATOMIC_RELAXED);
		do {
			atomic_memory_tls_free_end[thread][size].tag_ptr.ptr->_next =
					old_value;
		} while (!atomic_compare_exchange_16(
				&(ATOMIC_MEMORY_CACHE_GLOBAL_STRUCT.atomic_memory_cache[thread][size]._integral_type),
				&(old_value._integral_type),
				atomic_memory_tls_free_start[thread][size]._integral_type, 1,
				/* ensure that store to
				 * atomic_memory_tls_free_end[thread][size].tag_ptr.ptr->_next
				 * can be seen by a later load with __ATOMIC_ACQUIRE */
				__ATOMIC_RELEASE,
				__ATOMIC_RELAXED));

		atomic_memory_tls_free_start[thread][size].tag_ptr.ptr = NULL;
		free_count[thread][size] = 0;
	}
}
