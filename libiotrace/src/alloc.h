#ifndef LIBIOTRACE_ALLOC_H
#define LIBIOTRACE_ALLOC_H

#include "libiotrace_config.h"

#include <stdlib.h>
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

#ifndef IO_LIB_STATIC
void alloc_init(void) ATTRIBUTE_CONSTRUCTOR;
#endif

char toggle_alloc_wrapper(const char *line, const char toggle);

END_C_DECLS

#endif /* LIBIOTRACE_ALLOC_H */
