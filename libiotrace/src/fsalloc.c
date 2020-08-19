#include <dlfcn.h>
#include <errno.h>
#include <string.h>
#include <assert.h>

#include "libiotrace_config.h"

#include "atomic_memory_cache.h"

#define STATIC_CALLOC_BUFFER_SIZE 1024

/* must be static because calloc has to return zero initialized memory
 * and static variables are zero initialized */
static char static_calloc_buffer[STATIC_CALLOC_BUFFER_SIZE];
static char *static_calloc_buffer_pos = static_calloc_buffer;

void* (*__real_malloc)(size_t size) = NULL;
void (*__real_free)(void *ptr) = NULL;
//void* (*__real_calloc)(size_t nmemb, size_t size) = NULL;
//void* (*__real_realloc)(void *ptr, size_t size) = NULL;
#ifdef HAVE_REALLOCARRAY
void* (*__real_reallocarray)(void *ptr, size_t nmemb, size_t size) = NULL;
#endif

static void fsalloc_init() ATTRIBUTE_CONSTRUCTOR;

static inline void* get_mem_ptr(union atomic_memory_block_tag_ptr block)__attribute__((always_inline));
static inline enum atomic_memory_size get_mem_size(size_t size)__attribute__((always_inline));

char fsalloc_init_done = 0;

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

//		dlerror(); /* clear old error conditions */
//		__real_realloc = dlsym(RTLD_NEXT, "realloc");
//		char *dlsym_dlerror_realloc = dlerror();
//		assert(NULL == dlsym_dlerror_realloc);

		fsalloc_init_done = 1;
	}
}

void* malloc(size_t size) {
	union atomic_memory_block_tag_ptr block;
	struct atomic_memory_block *block_ptr;
	int real_size;
	enum atomic_memory_size block_size;

	block_size = get_mem_size(size);
	if (atomic_memory_size_count == block_size) {
		if (!fsalloc_init_done) {
			fsalloc_init();
		}

		if (__builtin_add_overflow(size, sizeof(struct atomic_memory_block),
				&real_size)) {
			errno = ENOMEM;
			return NULL;
		}
		block_ptr = __real_malloc(real_size);
		block_ptr->_size = atomic_memory_size_count;
		return block_ptr->memory;
	} else {
		block = atomic_memory_alloc(block_size);
		return get_mem_ptr(block);
	}
}

void* calloc(size_t nmemb, size_t size) {
	size_t real_size;
	char *old_value;
	char *new_value;
	void *ret;

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

	if (NULL == (void*) __real_malloc) {
		/* dlsym compiled with pthread uses calloc: first call of dlsym calls
		 * calloc before initialization of pointer to real calloc is done.
		 * That will start initialization via alloc_init() which calls dlsym
		 * which calls calloc which calls alloc_init() and so on.
		 *
		 * To prevent that a call of the wrapper of calloc with uninitialized
		 * pointer to real calloc must return memory from a static allocated
		 * area. So no recursive function calls will happen. This means also
		 * that calls of calloc before initialization via ctor are not logged. */

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

		return old_value;
	}

	ret = malloc(real_size);
	if (NULL == ret) {
		return ret;
	}
	return memset(ret, 0, real_size);
}

void* realloc(void *ptr, size_t size) {
	struct atomic_memory_block *block;
	void *ret;

	if (NULL == ptr) {
		return malloc(size);
	}

	if (0 == size) {
		free(ptr);
		return NULL;
	}

	block = (struct atomic_memory_block*) ((char*) ptr
			- sizeof(struct atomic_memory_block));

	ret = malloc(size);
	if (NULL == ret) {
		return ret;
	}

	if (size
			> atomic_memory_sizes[block->_size]
					- sizeof(struct atomic_memory_block)) {
		size = atomic_memory_sizes[block->_size]
				- sizeof(struct atomic_memory_block);
	}
	memcpy(ret, block->memory, size);

	if (atomic_memory_size_count == block->_size) {
		if (!fsalloc_init_done) {
			fsalloc_init();
		}

		__real_free(ptr);
		return ret;
	} else {
		atomic_memory_free(block->_next);
		return ret;
	}
}

void free(void *ptr) {
	struct atomic_memory_block *block;

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

		__real_free(&block);
	} else {
		atomic_memory_free(block->_next);
	}
}

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
	switch (size) {
	case 16:
		return atomic_memory_size_16;
		/* 8498689 function_data":{"size":16}}
		 7852957 function_data":{"size":8}}
		 7160032 function_data":{"size":12}}
		 5703916 function_data":{"size":24}}
		 5595432 function_data":{"size":32}}
		 5243898 function_data":{"size":4}}
		 5145887 function_data":{"size":152}}
		 5028626 function_data":{"size":20}}
		 3482313 function_data":{"size":28}}
		 3326830 function_data":{"size":40}}
		 2913915 function_data":{"size":36}}
		 2491322 function_data":{"size":88}}
		 2135248 function_data":{"size":44}}
		 2022163 function_data":{"size":58}}
		 1996998 function_data":{"size":48}}
		 1448418 function_data":{"size":232}}
		 1367466 function_data":{"size":52}}
		 1169029 function_data":{"size":56}}
		 1062700 function_data":{"size":31}}
		 901786 function_data":{"size":64}} */
	default:
		return atomic_memory_size_count;
	}
}
