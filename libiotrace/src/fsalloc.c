#include <dlfcn.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

//TODO: remove
#include <unistd.h>

#include "libiotrace_config.h"

#ifdef HAVE_PTHREAD_H
#  include <pthread.h>
#endif

//#include "atomic_memory_cache.h"

#define ATOMIC_MEMORY_CACHE_ABORT(message) do { \
                                               char text[500]; \
                                               snprintf(text, sizeof(text), message"\n"); \
                                               ssize_t ret = write(STDERR_FILENO, text, strlen(text)); /* TODO: use real write and no wrapper */ \
                                               if (ret) { \
                                            	   abort(); \
                                               } else { \
                                            	   abort(); \
                                               } \
                                           } while(0)

#define STATIC_CALLOC_BUFFER_SIZE 1024
#define ATOMIC_MEMORY_BUFFER_SIZE 1000LL * 1000 * 100
#define ATOMIC_MEMORY_MAX_THREADS 20
#define ATOMIC_MEMORY_CACHE_SIZE 100
#define ATOMIC_MEMORY_ALIGNMENT 16
//#define ATOMIC_MEMORY_CACHE_LINE 64 /* must be a multiple of ATOMIC_MEMORY_ALIGNMENT */
#define ATOMIC_MEMORY_CACHE_LINE 16 /* must be a multiple of ATOMIC_MEMORY_ALIGNMENT */
#define ATOMIC_MEMORY_SIZE_ALIGNED(x) (((x + sizeof(struct atomic_memory_block)) \
                                        + ATOMIC_MEMORY_ALIGNMENT - 1) \
                                       & ~(ATOMIC_MEMORY_ALIGNMENT - 1))

enum atomic_memory_size {
//	atomic_memory_size_24,

//	atomic_memory_size_64,
//	atomic_memory_size_128,
//	atomic_memory_size_192,
//	atomic_memory_size_256,
//	atomic_memory_size_320,
//	atomic_memory_size_384,
//	atomic_memory_size_448,
//	atomic_memory_size_512,
//	atomic_memory_size_576,
//	atomic_memory_size_640,
//	atomic_memory_size_704,
//	atomic_memory_size_768,
//	atomic_memory_size_832,
//	atomic_memory_size_896,
//	atomic_memory_size_960,
//	atomic_memory_size_1024,

	atomic_memory_size_0,
	atomic_memory_size_16,
	atomic_memory_size_32,
	atomic_memory_size_48,
	atomic_memory_size_64,
	atomic_memory_size_80,
	atomic_memory_size_96,
	atomic_memory_size_112,
	atomic_memory_size_128,
	atomic_memory_size_144,
	atomic_memory_size_160,
	atomic_memory_size_176,
	atomic_memory_size_192,
	atomic_memory_size_208,
	atomic_memory_size_224,
	atomic_memory_size_240,
	atomic_memory_size_256,
	atomic_memory_size_272,
	atomic_memory_size_288,
	atomic_memory_size_304,
	atomic_memory_size_320,
	atomic_memory_size_336,
	atomic_memory_size_352,
	atomic_memory_size_368,
	atomic_memory_size_384,
	atomic_memory_size_400,
	atomic_memory_size_416,
	atomic_memory_size_432,
	atomic_memory_size_448,
	atomic_memory_size_464,
	atomic_memory_size_480,
	atomic_memory_size_496,
	atomic_memory_size_512,

//	atomic_memory_size_1,
//	atomic_memory_size_2,
//	atomic_memory_size_4,
//	atomic_memory_size_8,
//	atomic_memory_size_16,
//	atomic_memory_size_32,
//	atomic_memory_size_64,
//	atomic_memory_size_128,
//	atomic_memory_size_256,
//	atomic_memory_size_512,
//	atomic_memory_size_1024,
//	atomic_memory_size_2048,
//	atomic_memory_size_4096,
//	atomic_memory_size_8192,
//	atomic_memory_size_16384,
//	atomic_memory_size_32768,
//	atomic_memory_size_65536,
//	atomic_memory_size_131072,
//	atomic_memory_size_262144,
//	atomic_memory_size_524288,
//	atomic_memory_size_1048576,
//	atomic_memory_size_2097152,
//	atomic_memory_size_4194304,
//	atomic_memory_size_8388608,
//	atomic_memory_size_16777216,
//	atomic_memory_size_33554432,
//	67108864,
//	134217728,
//	268435456,
//	536870912,
//	1073741824,
//	2147483648,
//	4294967296,
//	8589934592,
//	17179869184,
//	34359738368,
//	68719476736,
//	137438953472,
//	274877906944,
//	549755813888,
//	1,099511628×10¹²

//	atomic_memory_size_32,
//	atomic_memory_size_64,
//	atomic_memory_size_96,
//	atomic_memory_size_128,
//	atomic_memory_size_160,
//	atomic_memory_size_192,
//	atomic_memory_size_224,
//	atomic_memory_size_256,
//	atomic_memory_size_288,
//	atomic_memory_size_320,

//	atomic_memory_size_88,
//	atomic_memory_size_152,
//	atomic_memory_size_1048,
	atomic_memory_size_count
};

union atomic_memory_block_tag_ptr {
	struct {
		struct atomic_memory_block *ptr;
		uint64_t _tag;
	} tag_ptr;
	unsigned __int128 _integral_type; // TODO: check for __int128
} __attribute__ ((aligned (ATOMIC_MEMORY_ALIGNMENT)));

struct atomic_memory_block {
	union atomic_memory_block_tag_ptr _next; // must be the first element, to ensure alignment
	int32_t _thread;
	enum atomic_memory_size _size;
	char memory[] __attribute__((aligned (ATOMIC_MEMORY_CACHE_LINE)));
};

static const size_t atomic_memory_sizes[atomic_memory_size_count] = {
//		ATOMIC_MEMORY_SIZE_ALIGNED(24),

//		ATOMIC_MEMORY_SIZE_ALIGNED(64),
//		ATOMIC_MEMORY_SIZE_ALIGNED(128),
//		ATOMIC_MEMORY_SIZE_ALIGNED(192),
//		ATOMIC_MEMORY_SIZE_ALIGNED(256),
//		ATOMIC_MEMORY_SIZE_ALIGNED(320),
//		ATOMIC_MEMORY_SIZE_ALIGNED(384),
//		ATOMIC_MEMORY_SIZE_ALIGNED(448),
//		ATOMIC_MEMORY_SIZE_ALIGNED(512),
//		ATOMIC_MEMORY_SIZE_ALIGNED(576),
//		ATOMIC_MEMORY_SIZE_ALIGNED(640),
//		ATOMIC_MEMORY_SIZE_ALIGNED(704),
//		ATOMIC_MEMORY_SIZE_ALIGNED(768),
//		ATOMIC_MEMORY_SIZE_ALIGNED(832),
//		ATOMIC_MEMORY_SIZE_ALIGNED(896),
//		ATOMIC_MEMORY_SIZE_ALIGNED(960),
//		ATOMIC_MEMORY_SIZE_ALIGNED(1024)

		ATOMIC_MEMORY_SIZE_ALIGNED(0),
		ATOMIC_MEMORY_SIZE_ALIGNED(16),
		ATOMIC_MEMORY_SIZE_ALIGNED(32),
		ATOMIC_MEMORY_SIZE_ALIGNED(48),
		ATOMIC_MEMORY_SIZE_ALIGNED(64),
		ATOMIC_MEMORY_SIZE_ALIGNED(80),
		ATOMIC_MEMORY_SIZE_ALIGNED(96),
		ATOMIC_MEMORY_SIZE_ALIGNED(112),
		ATOMIC_MEMORY_SIZE_ALIGNED(128),
		ATOMIC_MEMORY_SIZE_ALIGNED(144),
		ATOMIC_MEMORY_SIZE_ALIGNED(160),
		ATOMIC_MEMORY_SIZE_ALIGNED(176),
		ATOMIC_MEMORY_SIZE_ALIGNED(192),
		ATOMIC_MEMORY_SIZE_ALIGNED(208),
		ATOMIC_MEMORY_SIZE_ALIGNED(224),
		ATOMIC_MEMORY_SIZE_ALIGNED(240),
		ATOMIC_MEMORY_SIZE_ALIGNED(256),
		ATOMIC_MEMORY_SIZE_ALIGNED(272),
		ATOMIC_MEMORY_SIZE_ALIGNED(288),
		ATOMIC_MEMORY_SIZE_ALIGNED(304),
		ATOMIC_MEMORY_SIZE_ALIGNED(320),
		ATOMIC_MEMORY_SIZE_ALIGNED(336),
		ATOMIC_MEMORY_SIZE_ALIGNED(352),
		ATOMIC_MEMORY_SIZE_ALIGNED(368),
		ATOMIC_MEMORY_SIZE_ALIGNED(384),
		ATOMIC_MEMORY_SIZE_ALIGNED(400),
		ATOMIC_MEMORY_SIZE_ALIGNED(416),
		ATOMIC_MEMORY_SIZE_ALIGNED(432),
		ATOMIC_MEMORY_SIZE_ALIGNED(448),
		ATOMIC_MEMORY_SIZE_ALIGNED(464),
		ATOMIC_MEMORY_SIZE_ALIGNED(480),
		ATOMIC_MEMORY_SIZE_ALIGNED(496),
		ATOMIC_MEMORY_SIZE_ALIGNED(512)

//		ATOMIC_MEMORY_SIZE_ALIGNED(1),
//		ATOMIC_MEMORY_SIZE_ALIGNED(2),
//		ATOMIC_MEMORY_SIZE_ALIGNED(4),
//		ATOMIC_MEMORY_SIZE_ALIGNED(8),
//		ATOMIC_MEMORY_SIZE_ALIGNED(16),
//		ATOMIC_MEMORY_SIZE_ALIGNED(32),
//		ATOMIC_MEMORY_SIZE_ALIGNED(64)
//		ATOMIC_MEMORY_SIZE_ALIGNED(128),
//		ATOMIC_MEMORY_SIZE_ALIGNED(256),
//		ATOMIC_MEMORY_SIZE_ALIGNED(512),
//		ATOMIC_MEMORY_SIZE_ALIGNED(1024),
//		ATOMIC_MEMORY_SIZE_ALIGNED(2048),
//		ATOMIC_MEMORY_SIZE_ALIGNED(4096),
//		ATOMIC_MEMORY_SIZE_ALIGNED(8192),
//		ATOMIC_MEMORY_SIZE_ALIGNED(16384),
//		ATOMIC_MEMORY_SIZE_ALIGNED(32768),
//		ATOMIC_MEMORY_SIZE_ALIGNED(65536),
//		ATOMIC_MEMORY_SIZE_ALIGNED(131072),
//		ATOMIC_MEMORY_SIZE_ALIGNED(262144),
//		ATOMIC_MEMORY_SIZE_ALIGNED(524288),
//		ATOMIC_MEMORY_SIZE_ALIGNED(1048576),
//		ATOMIC_MEMORY_SIZE_ALIGNED(2097152),
//		ATOMIC_MEMORY_SIZE_ALIGNED(4194304),
//		ATOMIC_MEMORY_SIZE_ALIGNED(8388608),
//		ATOMIC_MEMORY_SIZE_ALIGNED(16777216),
//		ATOMIC_MEMORY_SIZE_ALIGNED(33554432)

//		ATOMIC_MEMORY_SIZE_ALIGNED(32),
//		ATOMIC_MEMORY_SIZE_ALIGNED(64),
//		ATOMIC_MEMORY_SIZE_ALIGNED(96),
//		ATOMIC_MEMORY_SIZE_ALIGNED(128),
//		ATOMIC_MEMORY_SIZE_ALIGNED(160),
//		ATOMIC_MEMORY_SIZE_ALIGNED(192),
//		ATOMIC_MEMORY_SIZE_ALIGNED(224),
//		ATOMIC_MEMORY_SIZE_ALIGNED(256),
//		ATOMIC_MEMORY_SIZE_ALIGNED(288),
//		ATOMIC_MEMORY_SIZE_ALIGNED(320)

//		ATOMIC_MEMORY_SIZE_ALIGNED(88),
//		ATOMIC_MEMORY_SIZE_ALIGNED(152),
//		ATOMIC_MEMORY_SIZE_ALIGNED(1048)
		};

static inline void atomic_memory_init()__attribute__((always_inline));
static inline union atomic_memory_block_tag_ptr atomic_memory_alloc(
		enum atomic_memory_size size) __attribute__((warn_unused_result))__attribute__((always_inline));
static inline void atomic_memory_free(union atomic_memory_block_tag_ptr block)__attribute__((always_inline));
static inline struct atomic_memory_block* atomic_memory_new_block(
		enum atomic_memory_size size)__attribute__((always_inline))__attribute__((warn_unused_result))__attribute__((malloc));
static inline union atomic_memory_block_tag_ptr atomic_memory_new_block_list(
		enum atomic_memory_size size, uint32_t count)__attribute__((always_inline))__attribute__((warn_unused_result));
static inline char atomic_compare_exchange_16(unsigned __int128 *ptr,
		unsigned __int128 *expected, unsigned __int128 desired, char weak,
		int success_memorder, int failure_memorder)__attribute__((always_inline));
static inline unsigned __int128 atomic_load_16(unsigned __int128 *ptr,
		int memorder)__attribute__((always_inline));

static union atomic_memory_block_tag_ptr atomic_memory_cache[ATOMIC_MEMORY_MAX_THREADS][atomic_memory_size_count];
static char *atomic_memory_buffer;
static char *atomic_memory_buffer_pos;
static int32_t thread_count;

static ATTRIBUTE_THREAD union atomic_memory_block_tag_ptr atomic_memory_tls_cache[atomic_memory_size_count] =
{	{	.tag_ptr.ptr = NULL, .tag_ptr._tag = 0}};
static ATTRIBUTE_THREAD union atomic_memory_block_tag_ptr atomic_memory_tls_free_start[ATOMIC_MEMORY_MAX_THREADS][atomic_memory_size_count] =
{	{	{	.tag_ptr.ptr = NULL, .tag_ptr._tag = 0}}};
static ATTRIBUTE_THREAD union atomic_memory_block_tag_ptr atomic_memory_tls_free_end[ATOMIC_MEMORY_MAX_THREADS][atomic_memory_size_count] =
{	{	{	.tag_ptr.ptr = NULL, .tag_ptr._tag = 0}}};
static ATTRIBUTE_THREAD size_t free_count[ATOMIC_MEMORY_MAX_THREADS][atomic_memory_size_count] =
		{ { 0 } };
static ATTRIBUTE_THREAD int32_t thread_id = -1;

/* must be static because calloc has to return zero initialized memory
 * and static variables are zero initialized */
static char static_calloc_buffer[STATIC_CALLOC_BUFFER_SIZE]__attribute__((aligned (ATOMIC_MEMORY_ALIGNMENT)));
static char *static_calloc_buffer_pos = static_calloc_buffer;

void* (*__real_malloc)(size_t size) = NULL;
void (*__real_free)(void *ptr) = NULL;
//void* (*__real_calloc)(size_t nmemb, size_t size) = NULL;
void* (*__real_realloc)(void *ptr, size_t size) = NULL;
#ifdef HAVE_REALLOCARRAY
void* (*__real_reallocarray)(void *ptr, size_t nmemb, size_t size) = NULL;
#endif
int (*__real_posix_memalign)(void **memptr, size_t alignment, size_t size);

static void fsalloc_init() ATTRIBUTE_CONSTRUCTOR __attribute__((hot));
void* calloc(size_t nmemb, size_t size)__attribute__((hot));
void* realloc(void *ptr, size_t size)__attribute__((hot));
void free(void *ptr)__attribute__((hot));
size_t malloc_usable_size(void *ptr)__attribute__((hot));
int posix_memalign(void **memptr, size_t alignment, size_t size)__attribute__((hot));

void* malloc(size_t size)__attribute__((hot));

static inline void* get_mem_ptr(union atomic_memory_block_tag_ptr block)__attribute__((always_inline));
static inline enum atomic_memory_size get_mem_size(size_t size)__attribute__((always_inline));

static char fsalloc_init_done = 0;

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

static inline void atomic_memory_init() {
	int ret;

	ret = __real_posix_memalign((void**)&atomic_memory_buffer, ATOMIC_MEMORY_CACHE_LINE, ATOMIC_MEMORY_BUFFER_SIZE);
	if (0 != ret) {
		ATOMIC_MEMORY_CACHE_ABORT("not enough memory for atomic buffer");
	}
	atomic_memory_buffer_pos = atomic_memory_buffer;
}

static inline union atomic_memory_block_tag_ptr atomic_memory_alloc(
		enum atomic_memory_size size) {
	union atomic_memory_block_tag_ptr old_value;
	union atomic_memory_block_tag_ptr new_value;

//	char text[50];
//		snprintf(text, sizeof(text), "alloc\n");
//		write(STDERR_FILENO, text, strlen(text));

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

			old_value._integral_type = atomic_load_16(
					&(atomic_memory_cache[thread_id][size]._integral_type),
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
						&(atomic_memory_cache[thread_id][size]._integral_type),
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
		if (!fsalloc_init_done) {
			fsalloc_init();
		}
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
	char *old_value;
	char *new_value;
	size_t real_size = atomic_memory_sizes[size];

//	char text[50];
//	snprintf(text, sizeof(text), "new block: %ld\n", real_size);
//	write(STDERR_FILENO, text, strlen(text));

	/* atomic get needed bytes from buffer */
	old_value = __atomic_load_n(&(atomic_memory_buffer_pos),
	__ATOMIC_RELAXED);
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
			__ATOMIC_RELAXED,
			__ATOMIC_RELAXED));
	// TODO: check for __atomic_compare_exchange_n

	/* initialize memory block */
	((struct atomic_memory_block*) old_value)->_size = size;
	if (0 > thread_id) {
		/* get an index to an unique place in intern cache per thread */
		thread_id = __atomic_fetch_add(&thread_count, 1,
		__ATOMIC_RELAXED);
		if (ATOMIC_MEMORY_MAX_THREADS <= thread_id) {
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
	char *old_value;
	char *new_value;
	union atomic_memory_block_tag_ptr begin;
	union atomic_memory_block_tag_ptr end;
	union atomic_memory_block_tag_ptr tmp;
	size_t real_size;

//	char text[50];
//	snprintf(text, sizeof(text), "list: %d\n", count);
//	write(STDERR_FILENO, text, strlen(text));

	/* get count of needed bytes by multiplication of parameter size and count */
	if (__builtin_mul_overflow(atomic_memory_sizes[size], count, &real_size)) {
		// TODO: check for __builtin_mul_overflow
		/* we have an overflow */
		tmp.tag_ptr.ptr = NULL;
		return tmp; //TODO: abort() ???
	}

//	char text[50];
//	snprintf(text, sizeof(text), "new block: %ld\n", real_size);
//	write(STDERR_FILENO, text, strlen(text));

//	snprintf(text, sizeof(text), "list2\n", count);
//	write(STDERR_FILENO, text, strlen(text));

	/* atomic get needed bytes from buffer */
	old_value = __atomic_load_n(&(atomic_memory_buffer_pos),
	__ATOMIC_RELAXED);
	do {
		/* check if atomic_memory_buffer has enough free memory left for
		 * needed size (do it without adding size to atomic_memory_buffer_pos
		 * to prevent wrap around during evaluation) */
		if (atomic_memory_buffer + ATOMIC_MEMORY_BUFFER_SIZE - old_value
				>= real_size) {
			new_value = (void*) (old_value + real_size);
		} else {
			tmp.tag_ptr.ptr = NULL;
			return tmp; //TODO: abort() ???
		}
	} while (!__atomic_compare_exchange_n(&(atomic_memory_buffer_pos),
			&old_value, (void*) new_value, 1,
			__ATOMIC_RELAXED,
			__ATOMIC_RELAXED));

//	snprintf(text, sizeof(text), "list3\n", count);
//	write(STDERR_FILENO, text, strlen(text));

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

		tmp.tag_ptr.ptr = (struct atomic_memory_block*) (old_value
				+ (i * atomic_memory_sizes[size]));
		tmp.tag_ptr._tag = 0;
		end.tag_ptr.ptr->_next._integral_type = tmp._integral_type;
		end._integral_type = tmp._integral_type;
	}
	tmp.tag_ptr.ptr = NULL;
	end.tag_ptr.ptr->_next._integral_type = tmp._integral_type;

//	snprintf(text, sizeof(text), "list4\n", count);
//	write(STDERR_FILENO, text, strlen(text));

	return begin;
}

static inline void atomic_memory_free(union atomic_memory_block_tag_ptr block) {
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
		old_value._integral_type = atomic_load_16(
				&(atomic_memory_cache[thread][size]._integral_type),
				__ATOMIC_RELAXED);
		do {
			atomic_memory_tls_free_end[thread][size].tag_ptr.ptr->_next =
					old_value;
		} while (!atomic_compare_exchange_16(
				&(atomic_memory_cache[thread][size]._integral_type),
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

static void clear_init() {
//	char text[50];
//	snprintf(text, sizeof(text), "clear_init\n");
//	write(STDERR_FILENO, text, strlen(text));

	atomic_memory_buffer_pos = atomic_memory_buffer;
	thread_count = 0;

	for (int i = 0; i < atomic_memory_size_count; i++) {
		atomic_memory_tls_cache[i].tag_ptr.ptr = NULL;
		atomic_memory_tls_cache[i].tag_ptr._tag = 0;
	}

	for (int i = 0; i < ATOMIC_MEMORY_MAX_THREADS; i++) {
		for (int l = 0; l < atomic_memory_size_count; l++) {
			atomic_memory_cache[i][l].tag_ptr.ptr = NULL;
			atomic_memory_cache[i][l].tag_ptr._tag = 0;
			atomic_memory_tls_free_start[i][l].tag_ptr.ptr = NULL;
			atomic_memory_tls_free_start[i][l].tag_ptr._tag = 0;
			atomic_memory_tls_free_end[i][l].tag_ptr.ptr = NULL;
			atomic_memory_tls_free_end[i][l].tag_ptr._tag = 0;
			free_count[i][l] = 0;
		}
	}

	thread_id = -1;
}

static void debug_print() {
	char text[50];
	snprintf(text, sizeof(text), "debug_print\n");
	write(STDERR_FILENO, text, strlen(text));

	snprintf(text, sizeof(text), "free buffer: %ld\n",
			atomic_memory_buffer + ATOMIC_MEMORY_BUFFER_SIZE
					- atomic_memory_buffer_pos);
	write(STDERR_FILENO, text, strlen(text));

	snprintf(text, sizeof(text), "thread count: %d\n", thread_count);
	write(STDERR_FILENO, text, strlen(text));

	for (int i = 0; i < atomic_memory_size_count; i++) {
		if (NULL == atomic_memory_tls_cache[i].tag_ptr.ptr) {
			snprintf(text, sizeof(text), "local cache %d: empty\n", i);
			write(STDERR_FILENO, text, strlen(text));
		} else {
			struct atomic_memory_block *tmp =
					atomic_memory_tls_cache[i].tag_ptr.ptr;
			int l = 0;
			do {
				snprintf(text, sizeof(text), "local cache %d: %p\n", i, tmp);
				write(STDERR_FILENO, text, strlen(text));
				tmp = tmp->_next.tag_ptr.ptr;
				l++;
			} while (NULL != tmp && l < 2);
		}
	}
}

void fsalloc_init() {
	if (!fsalloc_init_done) {

		dlerror(); /* clear old error conditions */
		__real_malloc = dlsym(RTLD_NEXT, "malloc");
		char *dlsym_dlerror_malloc = dlerror();
		assert(NULL == dlsym_dlerror_malloc);

		dlerror(); /* clear old error conditions */
		__real_free = dlsym(RTLD_NEXT, "free");
		char *dlsym_dlerror_free = dlerror();
		assert(NULL == dlsym_dlerror_free);

		dlerror(); /* clear old error conditions */
		__real_realloc = dlsym(RTLD_NEXT, "realloc");
		char *dlsym_dlerror_realloc = dlerror();
		assert(NULL == dlsym_dlerror_realloc);

		dlerror(); /* clear old error conditions */
		__real_posix_memalign = dlsym(RTLD_NEXT, "posix_memalign");
		char *dlsym_dlerror_posix_memalign = dlerror();
		assert(NULL == dlsym_dlerror_posix_memalign);

		//pthread_atfork(NULL, NULL, clear_init);

		atomic_memory_init();

		fsalloc_init_done = 1;
	}
}

void* malloc(size_t size) {
	union atomic_memory_block_tag_ptr block;
	struct atomic_memory_block *block_ptr;
	int real_size;
//	int success;
	enum atomic_memory_size block_size;
	void *ret;

//	char text[100];
//	snprintf(text, sizeof(text), "malloc: %ld\n", size);
//	write(STDERR_FILENO, text, strlen(text));

	block_size = get_mem_size(size);
	if (atomic_memory_size_count == block_size) {
//		snprintf(text, sizeof(text), "malloc0 old\n");
//		write(STDERR_FILENO, text, strlen(text));

		if (!fsalloc_init_done) {
			fsalloc_init();
		}

		if (__builtin_add_overflow(size, sizeof(struct atomic_memory_block),
				&real_size)) {
			errno = ENOMEM;
			return NULL;
		}
		block_ptr = __real_malloc(real_size); /* in GNU systems malloc returns address aligned to 16 for 64 bit systems */
//		success = __real_posix_memalign((void**) &block_ptr, 16, real_size);
		if (NULL == block_ptr) {
			return NULL;
		}
//		if (0 != success) {
//			return NULL;
//		}
		block_ptr->_size = atomic_memory_size_count;
		block_ptr->_thread = size;
		block_ptr->_next.tag_ptr.ptr = block_ptr;

//		snprintf(text, sizeof(text), "malloc1 old: %p\n", block_ptr->memory);
//		write(STDERR_FILENO, text, strlen(text));
//		snprintf(text, sizeof(text), "malloc2 old: %p\n", block_ptr);
//		write(STDERR_FILENO, text, strlen(text));

//		for (int i = 0; i < size; i++) {
//			block_ptr->memory[i] = 0;
//		}
//		for (int i = 0; i < size; i++) {
//			assert(block_ptr->memory[i] == 0);
//		}

		return block_ptr->memory;
	} else {
//		snprintf(text, sizeof(text), "malloc0 new\n");
//		write(STDERR_FILENO, text, strlen(text));

//		snprintf(text, sizeof(text), "segfault test malloc: %p\n",
//				atomic_memory_buffer_pos);
//		write(STDERR_FILENO, text, strlen(text));

		block = atomic_memory_alloc(block_size);
		ret = get_mem_ptr(block);

//		snprintf(text, sizeof(text), "segfault test malloc: %p\n",
//				atomic_memory_buffer_pos);
//		write(STDERR_FILENO, text, strlen(text));

//		snprintf(text, sizeof(text), "segfault test malloc: %p, %d, %p, %ld\n",
//				ret, block.tag_ptr.ptr->_thread,
//				block.tag_ptr.ptr->_next.tag_ptr.ptr,
//				block.tag_ptr.ptr->_next.tag_ptr._tag);
//		write(STDERR_FILENO, text, strlen(text));
//		debug_print();
//		snprintf(text, sizeof(text), "malloc1 new: %p\n", ret);
//		write(STDERR_FILENO, text, strlen(text));
//		snprintf(text, sizeof(text), "malloc2 new: %p\n", block.tag_ptr.ptr);
//		write(STDERR_FILENO, text, strlen(text));
//		snprintf(text, sizeof(text), "test malloc: %p, %p, %p, %p, %ld\n",
//						&(block.tag_ptr.ptr->_next),
//						&(block.tag_ptr.ptr->_thread),
//						&(block.tag_ptr.ptr->_size),
//						block.tag_ptr.ptr->memory,
//						sizeof(struct atomic_memory_block));
//				write(STDERR_FILENO, text, strlen(text));

		return ret;
	}
}

void* calloc(size_t nmemb, size_t size) {
	size_t real_size;
	char *old_value;
	char *new_value;
	void *ret;

//	char text[50];
//	snprintf(text, sizeof(text), "calloc\n");
//	write(STDERR_FILENO, text, strlen(text));

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

	if (NULL == (void*) __real_posix_memalign) {
		/* dlsym compiled with pthread uses calloc: first call of dlsym calls
		 * calloc before initialization of pointer to real calloc is done.
		 * That will start initialization via alloc_init() which calls dlsym
		 * which calls calloc which calls alloc_init() and so on.
		 *
		 * To prevent that a call of the wrapper of calloc with uninitialized
		 * pointer to real calloc must return memory from a static allocated
		 * area. So no recursive function calls will happen. This means also
		 * that calls of calloc before initialization via ctor are not logged. */

		if (__builtin_add_overflow(real_size,
				sizeof(struct atomic_memory_block), &real_size)) {
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

		((struct atomic_memory_block*) old_value)->_size =
				atomic_memory_size_count;
		((struct atomic_memory_block*) old_value)->_thread = size;
		((struct atomic_memory_block*) old_value)->_next.tag_ptr.ptr =
				(struct atomic_memory_block*) old_value;
		return ((struct atomic_memory_block*) old_value)->memory;
	}

	ret = malloc(real_size);
	if (NULL == ret) {
		return ret;
	}
	return memset(ret, 0, real_size);
}

void* realloc(void *ptr, size_t size) {
	struct atomic_memory_block *old_block;
	struct atomic_memory_block *new_block;
	void *ret;
	size_t real_size;

//	char text[50];
//	snprintf(text, sizeof(text), "realloc\n");
//	write(STDERR_FILENO, text, strlen(text));

	if (NULL == ptr) {
		return malloc(size);
	}

	if (0 == size) {
		free(ptr);
		return NULL;
	}

	old_block = (struct atomic_memory_block*) ((char*) ptr
			- sizeof(struct atomic_memory_block));

	if (atomic_memory_size_count == old_block->_size) {
		if (!fsalloc_init_done) {
			fsalloc_init();
		}

		if (__builtin_add_overflow(size, sizeof(struct atomic_memory_block),
				&real_size)) {
			errno = ENOMEM;
			return NULL;
		}

		new_block = __real_realloc(old_block, real_size);
		new_block->_thread = size;
		new_block->_next.tag_ptr.ptr = new_block;

		return new_block->memory;
	} else {
		if (size
				<= atomic_memory_sizes[old_block->_size]
						- sizeof(struct atomic_memory_block)) {
			return ptr;
		}

		ret = malloc(size);
		if (NULL == ret) {
			return ret;
		}

		if (size
				> atomic_memory_sizes[old_block->_size]
						- sizeof(struct atomic_memory_block)) {
			size = atomic_memory_sizes[old_block->_size]
					- sizeof(struct atomic_memory_block);
		}

		memcpy(ret, old_block->memory, size);

		atomic_memory_free(old_block->_next);
		return ret;
	}
}

void free(void *ptr) {
	struct atomic_memory_block *block;

//	char text[100];
//	snprintf(text, sizeof(text), "free\n");
//	write(STDERR_FILENO, text, strlen(text));

	if (NULL == ptr) {
		return;
	}

	if ((char*) ptr
			>= &(static_calloc_buffer[0])&& (char*)ptr < &(static_calloc_buffer[0]) + STATIC_CALLOC_BUFFER_SIZE) {
		/* ptr was returned by wrapper of calloc from static memory: don't
		 * free it (will result in undefined behavior) */
		return;
	}

	block = (struct atomic_memory_block*) ((char*) ptr
			- sizeof(struct atomic_memory_block));

	if (atomic_memory_size_count == block->_size) {
		if (!fsalloc_init_done) {
			fsalloc_init();
		}

//		snprintf(text, sizeof(text), "free1 old: %p\n", ptr);
//		write(STDERR_FILENO, text, strlen(text));
//		snprintf(text, sizeof(text), "free2 old: %p\n", block);
//		write(STDERR_FILENO, text, strlen(text));

		__real_free(block->_next.tag_ptr.ptr);
	} else {
//		snprintf(text, sizeof(text), "segfault test free: %p, %d, %p\n", ptr,
//				block->_thread, block->_next.tag_ptr.ptr);
//		write(STDERR_FILENO, text, strlen(text));

//		snprintf(text, sizeof(text), "free1 new: %p\n", ptr);
//		write(STDERR_FILENO, text, strlen(text));
//		snprintf(text, sizeof(text), "free2 new: %p\n", block);
//		write(STDERR_FILENO, text, strlen(text));

//		union atomic_memory_block_tag_ptr tmp = block->_next;
//		block->_next.tag_ptr.ptr = NULL;

		atomic_memory_free(block->_next);

//		debug_print();
	}
}

size_t malloc_usable_size(void *ptr) {
	struct atomic_memory_block *block;

	if (NULL == ptr) {
		return 0;
	}

	block = (struct atomic_memory_block*) ((char*) ptr
			- sizeof(struct atomic_memory_block));

	if (atomic_memory_size_count == block->_size) {
		return block->_thread;
	} else {
		return atomic_memory_sizes[block->_size]
				- sizeof(struct atomic_memory_block);
	}
}

int posix_memalign(void **memptr, size_t alignment, size_t size) {
	struct atomic_memory_block *block_ptr;
	int real_size;
	void *ret;
	char *new_pos;

//	char text[50];
//	snprintf(text, sizeof(text), "posix_memalign: %ld\n", size);
//	write(STDERR_FILENO, text, strlen(text));

	/* alignment must be a power of 2 */
	if (!alignment || (alignment & (alignment - 1))) {
		return EINVAL;
	}

	/* alignment must be a multiple of sizeof(void *) */
	if (alignment % sizeof(void*) != 0) {
		return EINVAL;
	}

	/* Only possible values < 16 are 0 and 8 (must be a power of 2 and a
	 * multiple of 8).
	 * We increase it for better performance during evaluation of prefix
	 * (struct atomic_memory_block). */
	if (16 > alignment) {
		alignment = 16;
	}

	if (!fsalloc_init_done) {
		fsalloc_init();
	}

	if (__builtin_add_overflow(size, sizeof(struct atomic_memory_block),
			&real_size)) {
		return ENOMEM;
	}
	if (__builtin_add_overflow(real_size, alignment - 1, &real_size)) {
		return ENOMEM;
	}
	ret = __real_malloc(real_size);
	if (NULL == ret) {
		return ENOMEM;
	}
	block_ptr = ret;

	/* move block_ptr to fit alignment for block_ptr->memory */
	new_pos = (char*) (((uint64_t) block_ptr->memory + (alignment - 1))
			& ~(alignment - 1));
	block_ptr = (struct atomic_memory_block*) ((char*) block_ptr
			+ (new_pos - block_ptr->memory));

	block_ptr->_size = atomic_memory_size_count;
	block_ptr->_thread = size;
	block_ptr->_next.tag_ptr.ptr = ret;

	*memptr = block_ptr->memory;

	return 0;
}

//void* aligned_alloc(size_t alignment, size_t size) {
//	char text[50];
//	snprintf(text, sizeof(text), "aligned_alloc\n");
//	write(STDERR_FILENO, text, strlen(text));
//
//	abort();
//}

//void* valloc(size_t size) {
//	char text[50];
//	snprintf(text, sizeof(text), "valloc\n");
//	write(STDERR_FILENO, text, strlen(text));
//
//	abort();
//}
//
//void* memalign(size_t alignment, size_t size) {
//	char text[50];
//	snprintf(text, sizeof(text), "memalign\n");
//	write(STDERR_FILENO, text, strlen(text));
//
//	abort();
//}
//
//void* pvalloc(size_t size) {
//	char text[50];
//	snprintf(text, sizeof(text), "pvalloc\n");
//	write(STDERR_FILENO, text, strlen(text));
//
//	abort();
//}

static inline void* get_mem_ptr(union atomic_memory_block_tag_ptr block) {
	if (NULL != block.tag_ptr.ptr) {
		block.tag_ptr.ptr->_next._integral_type = block._integral_type;
		return block.tag_ptr.ptr->memory;
	} else {
		errno = ENOMEM;
		return NULL;
	}
}

static inline enum atomic_memory_size get_mem_size(size_t size) {
//	if (size <= 24) {
//		return atomic_memory_size_24;
////	} else if (size <= 1000) {
////		return atomic_memory_size_1000;
////	} else if (size <= 10000) {
////		return atomic_memory_size_10000;
//	} else if (size <= 152) {
//		return atomic_memory_size_152;
//	} else if (size <= 1048) {

	//return (enum atomic_memory_size) (64 - __builtin_clzl(size + 1));

	size = ((size + 16 - 1) & ~(16 - 1)) >> 4;
	if (size > 32) {
		return atomic_memory_size_count;
	} else {
		return size;
	}

//	if (size < 1024) {
//		//return (enum atomic_memory_size) ((size + 32 - 1) & ~(32 - 1)) / 32;
//		return (enum atomic_memory_size) (size >> 6);
//	} else {
//		return atomic_memory_size_count;
//	}

//	if (size <= 32) {
//		return atomic_memory_size_32;
//	}else if(size<=1056) {
//		return atomic_memory_size_1056;
//	} else {
//		return atomic_memory_size_count;
//	}

//	switch (size) {
//	case 16:
//		return atomic_memory_size_16;
//	case 8:
//		return atomic_memory_size_8;
//	case 12:
//		return atomic_memory_size_12;
//	case 24:
//		return atomic_memory_size_24;
//	case 32:
//		return atomic_memory_size_32;
//	case 4:
//		return atomic_memory_size_4;
//	case 152:
//		return atomic_memory_size_152;
//	case 20:
//		return atomic_memory_size_20;
//	case 28:
//		return atomic_memory_size_28;
//	case 40:
//		return atomic_memory_size_40;
//	case 36:
//		return atomic_memory_size_36;
//	case 88:
//		return atomic_memory_size_88;
//	case 44:
//		return atomic_memory_size_44;
//	case 58:
//		return atomic_memory_size_58;
//	case 48:
//		return atomic_memory_size_48;
//	case 232:
//		return atomic_memory_size_232;
//	case 52:
//		return atomic_memory_size_52;
//	case 56:
//		return atomic_memory_size_56;
//	case 31:
//		return atomic_memory_size_31;
//	case 64:
//		return atomic_memory_size_64;
//		/*
//		 8498689 function_data":{"size":16}}
//		 7852957 function_data":{"size":8}}
//		 7160032 function_data":{"size":12}}
//		 5703916 function_data":{"size":24}}
//		 5595432 function_data":{"size":32}}
//		 5243898 function_data":{"size":4}}
//		 5145887 function_data":{"size":152}}
//		 5028626 function_data":{"size":20}}
//		 3482313 function_data":{"size":28}}
//		 3326830 function_data":{"size":40}}
//		 2913915 function_data":{"size":36}}
//		 2491322 function_data":{"size":88}}
//		 2135248 function_data":{"size":44}}
//		 2022163 function_data":{"size":58}}
//		 1996998 function_data":{"size":48}}
//		 1448418 function_data":{"size":232}}
//		 1367466 function_data":{"size":52}}
//		 1169029 function_data":{"size":56}}
//		 1062700 function_data":{"size":31}}
//		 901786 function_data":{"size":64}} */
//	default:
//		return atomic_memory_size_count;
//	}
}
