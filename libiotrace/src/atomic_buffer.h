/**
 * A partial lock free buffer.
 *
 * Allocates memory once with malloc() during initialization with
 * function atomic_buffer_create(). Subsequent allocations from pool
 * with function atomic_buffer_alloc() are satisfied using atomic
 * instructions. This ensures lock free behavior.
 *
 * Via atomic_buffer_alloc() allocated memory can be freed with
 * function atomic_buffer_free(). Freed memory isn't immediately
 * ready for reuse or returned to the OS. Instead it's marked as
 * free. If all from one buffer allocated memory is marked as free
 * the memory can be returned to the OS by a call to
 * atomic_buffer_destroy(). After that the buffer can no longer
 * used. But the buffer can be part of a new call to
 * atomic_buffer_create() to create a new buffer. Alternatively to
 * atomic_buffer_destroy() it's possible to call
 * atomic_buffer_reuse(). In that case the memory isn't returned to
 * the OS. Instead the buffer is reset to its initial state and can
 * be used as if freshly created.
 *
 * Example:
 *
 * #include <errno.h>
 * #include "atomic_buffer.h"
 *
 * struct atomic_buffer buf;
 * int ret;
 * void *mem;
 *
 * ret = atomic_buffer_create(&buf, 500);
 * if (-1 == ret) {
 *     if (ENOMEM == errno) {
 *         // Out of memory
 *     } else {
 *         // unknown error
 *     }
 * }
 *
 * mem = atomic_buffer_alloc(&buf, 100);
 * if (NULL == mem) {
 *     // not enough available memory in buf
 * }
 *
 * // do something usefull with mem
 *
 * atomic_buffer_free(&buf, mem);
 *
 * atomic_buffer_destroy(&buf, 0);
 *
 */

#ifndef LIBIOTRACE_ATOMIC_BUFFER_H
#define LIBIOTRACE_ATOMIC_BUFFER_H

#include <stddef.h>

#include "libiotrace_config.h"

#ifdef ATOMIC_BUFFER_TEST
    extern ATTRIBUTE_THREAD int *atomic_buffer_wait_pos;
#endif

BEGIN_C_DECLS

/**
 * Struct to hold an atomic buffer.
 *
 * Each buffer needs its own struct. Each struct has to be
 * initialized with atomic_buffer_create().
 * Don't change the value of a member of this struct outside
 * of the implementation (it will break the buffer).
 *
 * Each struct of type atomic_buffer should be created from
 * static memory. This ensures that stalled threads trying
 * to use such a struct after a atomic_buffer_destroy() are
 * working properly. If dynamic allocated memory is used,
 * a free to that memory should only be performed if it's
 * sure that no thread is trying to use the struct after
 * the free (meaning no thread is stalled during a use of
 * the struct and no new call to a function with the struct
 * as an parameter is possible).
 */
struct atomic_buffer {
	/**
	 * start address of the atomic buffer
	 * (set by a call to atomic_buffer_create())
	 */
	void *_start;
	/**
	 * end address of the atomic buffer
	 * (set by a call to atomic_buffer_create())
	 */
	void *_end;
	/**
	 * current start address of free memory in
	 * the atomic buffer (initially set by a call
	 * to atomic_buffer_create() and increased by
	 * calls to atomic_buffer_alloc())
	 */
	void *_current;
	/**
	 * length of (by calls to atomic_buffer_free())
	 * freed memory in the buffer (initially set
	 * by call to atomic_buffer_create() and
	 * increased by calls to atomic_buffer_free())
	 */
	void *_freed;
};

struct atomic_buffer_prefix {
	size_t length;
};

/**
 * Create and Initialize a new buffer.
 *
 * Allocates a buffer with "size" bytes length and initialize
 * the struct "buf" with it.
 * This function is not guarded against concurrent threads and
 * should therefore be called only once from one thread. (e.g.
 * use a mutex for creating the buffer)
 *
 * Each allocation via atomic_buffer_alloc() reserves an
 * additional prefix to handle the allocation. So the "size" of
 * the buffer should be
 * (needed memory) + (number of allocations) * sizeof(struct
 * atomic_buffer_prefix).
 *
 * It is possible to reuse a the struct "buf" if the struct was
 * destroyed with a call to atomic_buffer_destroy(). In this case
 * atomic_buffer_create() doesn't ensure that stalled threads
 * already using "buf" are working properly (see
 * atomic_buffer_destroy()).
 *
 * @param[out] buf        pointer to a struct for holding the new buffer
 * @param[in]  size       length in bytes of the new buffer
 * @return 0 on success, -1 if an error occurred (check errno for ENOMEM Out of memory)
 */
int atomic_buffer_create(struct atomic_buffer *buf, size_t size)
		__attribute__((nonnull));

/**
 * Atomically allocate memory from buffer.
 *
 * Allocates memory with "size" bytes length from buffer "buf".
 * This function works atomically on "buf" and can therefore be
 * called from concurrent threads.
 *
 * Struct "buf" has to be initialized via function
 * atomic_buffer_create().
 *
 * Each allocated memory area needs additional memory to save
 * informations about the allocation. So more than "size" bytes
 * are reserved from the buffer (additional memory per
 * allocation is sizeof(struct atomic_buffer_prefix)).
 *
 * @param[in,out] buf        buffer to allocate memory from
 * @param[in]     size       length in bytes to allocate
 * @return a pointer to the allocated memory. If less than
 *         "size" bytes are available in the buffer, NULL is
 *         returned.
 */
void* atomic_buffer_alloc(struct atomic_buffer *buf, size_t size)
		__attribute__((nonnull));

/**
 * Atomically free memory from buffer.
 *
 * Marks previous with atomic_buffer_alloc() allocated memory as
 * free. This enables reuse of the memory with
 * atomic_buffer_reuse() or freeing the whole buffer with
 * atomic_buffer_destroy().
 *
 * "memory" must be achieved via call of the function
 * atomic_buffer_alloc(). Using other pointers results in
 * undefined behavior.
 *
 * Multiple calls of this function with the same "buf" and "memory"
 * results in freeing other memory areas (which are probably in
 * use from other threads) and therefore in undefined behavior.
 * So for each "memory" pointer in each buffer this function should
 * be called only once. It's possible to call atomic_buffer_alloc()
 * from a different thread than the corresponding
 * atomic_buffer_free() for the same memory area.
 *
 * @param[in,out] buf        buffer to free memory from
 * @param[in]     memory     pointer to via atomic_buffer_alloc() allocated memory
 */
void atomic_buffer_free(struct atomic_buffer *buf, void *memory)
		__attribute__((nonnull));

/**
 * Destroys an existing buffer.
 *
 * This function sets the buffer size to zero. So all threads which
 * are working with "buf" can't allocate memory from it. After that
 * the function waits until all allocated memory is freed with an
 * call to atomic_buffer_free(). Therefore this function is not lock
 * free. If a single thread doesn't free its memory it blocks
 * forever. The function checks in intervals if all allocated
 * memory is freed. After each check the function sleeps for
 * "sleep_nanoseconds" nanoseconds. If all allocations are freed the
 * underlying buffer is freed with an call to free().
 * At this point some threads could be stalled in
 * atomic_buffer_alloc() and trying to use "buf" in subsequent
 * instructions. They will fail because the available size is set to
 * zero. So a call to atomic_buffer_destroy() should only be done if
 * it's ensured that no thread is calling atomic_buffer_alloc() with
 * "buf" as an parameter at this moment or in the future.
 *
 * A destroyed buffer can not be reused with atomic_buffer_reuse(),
 * because its memory is freed.
 *
 * A destroyed buffer can be used to create a new one with
 * atomic_buffer_create(). For that it has to be ensured that no
 * stalled thread is already trying to allocate memory from "buf".
 * That's necessary because atomic_buffer_create() is not thread
 * safe and so concurrent access to "buf" will result in undefined
 * behavior.
 *
 * This function is not guarded against concurrent threads in
 * regard to free() and should therefore be called only once from
 * one thread. (e.g. use a mutex for destroying the buffer)
 *
 * @param[in,out] buf                  buffer to destroy
 * @param[in]     sleep_nanoseconds    interval to sleep between tries to destroy
 *                                     (values less than 0 are set to 0 and values
 *                                     greater than 999999999 are set to
 *                                     999999999)
 */
void atomic_buffer_destroy(struct atomic_buffer *buf,
		long sleep_nanoseconds) __attribute__((nonnull));

/**
 * Reuses an existing buffer.
 *
 * This function sets the buffer size to zero. So all threads which
 * are working with "buf" can't allocate memory from it. After that
 * the function waits until all allocated memory is freed with an
 * call to atomic_buffer_free(). Therefore this function is not lock
 * free. If a single thread doesn't free its memory it blocks
 * forever. The function checks in intervals if all allocated
 * memory is freed. After each check the function sleeps for
 * "sleep_nanoseconds" nanoseconds. If all allocations are freed the
 * size of available memory in "buf" is set to the initial size of
 * the buffer.
 * At this point some threads could be stalled in
 * atomic_buffer_alloc() and trying to use "buf" in subsequent
 * instructions. They will allocate memory from the reused buffer.
 *
 * A destroyed buffer can not be reused with atomic_buffer_reuse(),
 * because its memory is freed.
 *
 * This function is not guarded against concurrent threads in
 * regard to setting the available memory and should therefore be
 * called only once from one thread. (e.g. use a mutex for
 * reusing the buffer)
 *
 * @param[in,out] buf                  buffer to destroy
 * @param[in]     sleep_nanoseconds    interval to sleep between tries to destroy
 *                                     (values less than 0 are set to 0 and values
 *                                     greater than 999999999 are set to
 *                                     999999999)
 */
void atomic_buffer_reuse(struct atomic_buffer *buf,
		long sleep_nanoseconds) __attribute__((nonnull));

/**
 * Get number of free bytes of a buffer.
 *
 * Returns the number of free (not already allocated) bytes
 * in buffer "buf". Before the return value is read a other
 * thread can change the number of free bytes. Therefore this
 * function should only be used for debugging and not be part
 * of program logic.
 *
 * @param[in]     buf                  buffer to get information of
 * @return number of not allocated bytes in buffer "buf"
 */
size_t atomic_buffer_get_free_memory(struct atomic_buffer *buf)
		__attribute__((nonnull));

/**
 * Get number of freed bytes of a buffer.
 *
 * Returns the number of freed (not longer allocated) bytes
 * in buffer "buf". Before the return value is read a other
 * thread can change the number of freed bytes. Therefore
 * this function should only be used for debugging and not be
 * part of program logic.
 *
 * @param[in]     buf                  buffer to get information of
 * @return number of not longer allocated bytes in buffer "buf"
 */
size_t atomic_buffer_get_freed_memory(struct atomic_buffer *buf)
		__attribute__((nonnull));

END_C_DECLS

#endif /* LIBIOTRACE_ATOMIC_BUFFER_H */
