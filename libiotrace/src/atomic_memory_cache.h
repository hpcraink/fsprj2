#ifndef LIBIOTRACE_ATOMIC_MEMORY_CACHE_H
#define LIBIOTRACE_ATOMIC_MEMORY_CACHE_H

#include <stddef.h>
#include <inttypes.h>

#define ATOMIC_MEMORY_BUFFER_SIZE 1000 * 1000 * 1000

/* for atomic operations on atomic_memory_block_tag_ptr and compiler optimizations */
#define ATOMIC_MEMORY_ALIGNMENT 16
#define ATOMIC_MEMORY_SIZE_ALIGNED(x) (((x + sizeof(struct atomic_memory_block)) \
                                        + ATOMIC_MEMORY_ALIGNMENT - 1) \
                                       & ~(ATOMIC_MEMORY_ALIGNMENT - 1))

enum atomic_memory_size {
	atomic_memory_size_56,
	atomic_memory_size_110,
	atomic_memory_size_520,
	atomic_memory_size_1678,
	/* each new enum needs an corresponding entry in atomic_memory_sizes */
	atomic_memory_size_count
};

union atomic_memory_block_tag_ptr {
	struct {
		struct atomic_memory_block *_ptr;
		uint64_t _tag;
	} _tag_ptr;
	unsigned __int128 _integral_type;
} __attribute__ ((aligned (ATOMIC_MEMORY_ALIGNMENT)));

struct atomic_memory_block {
	union atomic_memory_block_tag_ptr _next;
	enum atomic_memory_size _size;
	char memory[]; //with at least _size array length (optional padding for alignment)
};

static const size_t atomic_memory_sizes[atomic_memory_size_count] = {
		ATOMIC_MEMORY_SIZE_ALIGNED(56), ATOMIC_MEMORY_SIZE_ALIGNED(110),
		ATOMIC_MEMORY_SIZE_ALIGNED(520), ATOMIC_MEMORY_SIZE_ALIGNED(1678) };

union atomic_memory_block_tag_ptr atomic_memory_alloc(
		enum atomic_memory_size size)
				__attribute__((warn_unused_result))__attribute__((hot));

void atomic_memory_push(union atomic_memory_block_tag_ptr block);

union atomic_memory_block_tag_ptr atomic_memory_pop()
		__attribute__((warn_unused_result));

void atomic_memory_free(union atomic_memory_block_tag_ptr block);

#endif /* LIBIOTRACE_ATOMIC_MEMORY_CACHE_H */
