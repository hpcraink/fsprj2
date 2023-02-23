#ifndef LIBIOTRACE_ALLOC_H
#define LIBIOTRACE_ALLOC_H

#include "libiotrace_config.h"

#include <stdlib.h>
#if defined(HAVE_BRK) || defined(HAVE_SBRK)
#  include <unistd.h>
#endif
#include "wrapper_defines.h"

#define STATIC_CALLOC_BUFFER_SIZE 1024

/* Function pointers for  alloc functions */

BEGIN_C_DECLS

REAL_TYPE void* REAL(malloc)(size_t size) REAL_INIT;
REAL_TYPE void REAL(free)(void *ptr) REAL_INIT;
REAL_TYPE void* REAL(calloc)(size_t nmemb, size_t size) REAL_INIT;
REAL_TYPE void* REAL(realloc)(void *ptr, size_t size) REAL_INIT;
#ifdef HAVE_REALLOCARRAY
REAL_TYPE void* REAL(reallocarray)(void *ptr, size_t nmemb, size_t size) REAL_INIT;
#endif
REAL_TYPE int REAL(posix_memalign)(void **memptr, size_t alignment, size_t size) REAL_INIT;
#ifdef HAVE_BRK
REAL_TYPE int REAL(brk)(void *addr) REAL_INIT;
#endif
#ifdef HAVE_SBRK
REAL_TYPE void* REAL(sbrk)(intptr_t increment) REAL_INIT;
#endif

#ifndef IO_LIB_STATIC
void alloc_init(void) ATTRIBUTE_CONSTRUCTOR;
#endif

char toggle_alloc_wrapper(const char *line, const char toggle);

END_C_DECLS

#endif /* LIBIOTRACE_ALLOC_H */
