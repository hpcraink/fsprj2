/**
 * A new created memory block belongs to the thread it was created by.
 *
 * Thread local caches are only accessed by one thread. It's therefore not
 * necessary to guard them against concurrent accesses. This makes stores and
 * loads of blocks in these caches fast.
 */
#ifndef LIBIOTRACE_ATOMIC_MEMORY_CACHE_H
#define LIBIOTRACE_ATOMIC_MEMORY_CACHE_H

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>

/**
 * Size of global buffer (per process) to create memory blocks from.
 */
#define ATOMIC_MEMORY_BUFFER_SIZE 1000 * 1000 * 1000
/**
 * Maximum of caches for different threads (per process).
 */
#define ATOMIC_MEMORY_MAX_THREADS_PER_PROCESS 200
/**
 * Count of memory blocks per cache.
 *
 * During first allocation of a memory block of one size from one thread the
 * cache of the allocating thread is filled with this many blocks of the
 * requested size before an additional block is returned to the thread. Each
 * time the cache for one size in one thread is empty and no free blocks for
 * this size and this thread are available in the global cache it is filled
 * with this many new blocks of the corresponding size.
 *
 * Before a freed block is returned to the cache of the thread it was created
 * by it is cached in the thread which frees it. If the count of freed but not
 * returned blocks of one size belonging to one thread reaches the count of
 * memory blocks per cache the thread local cache is pushed to the global
 * cache.
 */
#define ATOMIC_MEMORY_CACHE_SIZE 100

/**
 * Alignment of memory blocks and pointers to them.
 *
 * For fast access to 16 bytes width values the memory has to be aligned to at
 * least 16. E.g. for a x86 architecture the compiler can emit a movdqa
 * instruction if the alignment permits it.
 */
#define ATOMIC_MEMORY_ALIGNMENT 16
/**
 * Get needed bytes to store a memory block with \a x bytes of usable memory.
 *
 * \a x is increased by memory needed to store meta data for struct
 * \a atomic_memory_block and padding to align to #ATOMIC_MEMORY_ALIGNMENT.
 *
 * @param[in]  x          bytes of needed usable memory
 * @return count of bytes needed to store a memory block with \a x bytes of
 *         usable memory
 */
#define ATOMIC_MEMORY_SIZE_ALIGNED(x) (((x + sizeof(struct atomic_memory_block)) \
                                        + ATOMIC_MEMORY_ALIGNMENT - 1) \
                                       & ~(ATOMIC_MEMORY_ALIGNMENT - 1))

/**
 * Possible memory block sizes.
 *
 * Each value represents one possible size of a memory block. The last part of
 * each enum value name is the usable memory in bytes for this block size.
 *
 * The last enum value ist used to evaluate the number of different possible
 * sizes. So new sizes have to be added before the last element.
 *
 * For each enum value before the last one a corresponding entry in the array
 * \a atomic_memory_sizes is needed.
 */
enum atomic_memory_size {
	atomic_memory_size_56,
	atomic_memory_size_110,
	atomic_memory_size_520,
	atomic_memory_size_1678,
//	atomic_memory_size_3200,
	/* each new enum needs an corresponding entry in atomic_memory_sizes */
	atomic_memory_size_count
};

/**
 * Pointer to a memory block.
 *
 * Enables fast and atomic (e.g. via gcc builtins) access to a tagged pointer.
 *
 * A simple pointer to struct \a atomic_memory_block is prone to the
 * ABA-problem. Combined with a 8 byte long tag, prevention of the ABA-problem
 * is possible. To read and write the combination of a 8 byte long pointer
 * (\a _tag_ptr._ptr) and the 8 byte long tag (\a _tag_ptr._tag) with single
 * instructions, it's necessary to keep them together in one 16 byte long
 * memory area and make this area accessible through one 16 bytes long
 * variable (\a _integral_type). For instructions which manipulate 16 bytes at
 * once it's additional necessary to align this variable to at least 16 bytes.
 */
union atomic_memory_block_tag_ptr {
	struct {
		struct atomic_memory_block *_ptr;
		uint64_t _tag;
	} _tag_ptr;
	unsigned __int128 _integral_type; // TODO: check for __int128
} __attribute__ ((aligned (ATOMIC_MEMORY_ALIGNMENT))); // TODO: check for attribute aligned

/**
 * Memory block.
 *
 * Combines all needed metadata for handling a memory block and the usable
 * data of the block in one struct. Access to the members of this struct
 * should be restricted to the member \a memory. The other members are needed
 * inside the functions of \c atomic_memory_cache. A manipulation of these
 * members from the outside will break the functions.
 *
 * To get a new fully initialized \a atomic_memory_block, call the function
 * \c atomic_memory_alloc().
 */
struct atomic_memory_block {
	/**
	 * Pointer to the next memory block in a linked list of blocks.
	 *
	 * Used for each thread local and the global cache and for the list of
	 * ready to consume blocks. The pointer is included inside the memory
	 * block structure so no wrapper node to build a linked list is necessary.
	 * This means a block can only be part of one list at a time but to
	 * process this list no additional dereferencing is necessary.
	 */
	union atomic_memory_block_tag_ptr _next; // must be the first element, to ensure alignment
	/**
	 * Id of the thread this memory block belongs to.
	 *
	 * The id of the thread which created this block. After freeing this block
	 * it will be returned to the local cache of the thread with this id.
	 * In this thread this block will be available for further allocations.
	 */
	int32_t _thread;
	/**
	 * Size of usable memory in this block.
	 *
	 * Is used to store the block inside a separate list for each size in the
	 * local or the global cache. This ensures fast access to cached free
	 * blocks without the need to iterate over blocks of wrong size. For that
	 * the enum value of \a _size is used as an index to an array of linked
	 * lists in each cache.
	 */
	enum atomic_memory_size _size;
	/**
	 * Usable memory.
	 *
	 * This member can be used after a call to \c atomic_memory_alloc() to
	 * store data. It should not be accessed after it's freed with a call to
	 * \c atomic_memory_free().
	 */
	char memory[]; // with at least _size array length (optional padding for alignment)
};

/**
 * Array of all possible memory block sizes.
 *
 * Each element in this array represents one possible block size. So for each
 * enum value of \a atomic_memory_size (with exception of the last one) a
 * separate element in this array exists. The elements in this array have the
 * same order as the values in the enum.
 *
 * Each enum value of \a atomic_memory_size (with exception of the last one)
 * can be used as an index to get the corresponding size from this array.
 *
 * The size is the summary of the memory needed to store meta data for struct
 * \a atomic_memory_block, the usable memory and padding to align to
 * #ATOMIC_MEMORY_ALIGNMENT.
 */
static const size_t atomic_memory_sizes[atomic_memory_size_count] = {
		ATOMIC_MEMORY_SIZE_ALIGNED(56), ATOMIC_MEMORY_SIZE_ALIGNED(110),
		ATOMIC_MEMORY_SIZE_ALIGNED(520), ATOMIC_MEMORY_SIZE_ALIGNED(1678) /*,
 ATOMIC_MEMORY_SIZE_ALIGNED(3200)*/};

/**
 * Allocation of a memory block.
 *
 * Returns a pointer to a memory block out of a cache or a new created block
 * if local and global caches are empty. The returned pointer can be used to
 * fill the usable data in the memory block (via \c ._tag_ptr._ptr->memory).
 * After that the pointer can be used to push the memory block to a global
 * stack via function call of \c atomic_memory_push(). Alternatively the
 * memory block can be freed with \c atomic_memory_free() if it is no longer
 * used.
 *
 * The tag of the returned pointer is increased, if the memory block comes out
 * of a cache. This prevents the ABA-problem if the returned pointer is only
 * used once for each function from \c atomic_memory_cache (one
 * \c atomic_memory_push() followed by one \c atomic_memory_pop() followed by
 * one \c atomic_memory_free).
 *
 * @param[in]  size       enum which gives the length in bytes of the new
 *                        memory block (see array \a atomic_memory_sizes)
 * @return \a atomic_memory_block_tag_ptr to a new memory block on success,
 *         \a atomic_memory_block_tag_ptr with field \c _ptr set to NULL if
 *         not enough space in \a atomic_memory_buffer is available
 */
union atomic_memory_block_tag_ptr atomic_memory_alloc(
		enum atomic_memory_size size)
				__attribute__((warn_unused_result))__attribute__((hot));

/**
 * Pushes a memory block in a ready to consume stack.
 *
 *
 */
void atomic_memory_push(union atomic_memory_block_tag_ptr block);

union atomic_memory_block_tag_ptr atomic_memory_pop()
		__attribute__((warn_unused_result));

void atomic_memory_free(union atomic_memory_block_tag_ptr block);

#endif /* LIBIOTRACE_ATOMIC_MEMORY_CACHE_H */
