#include "wrapper_name.h"

WRAPPER_NAME(malloc)
WRAPPER_NAME(free)
WRAPPER_NAME(calloc)
WRAPPER_NAME(realloc)
#ifdef HAVE_REALLOCARRAY
WRAPPER_NAME(reallocarray)
#endif
#ifdef HAVE_BRK
WRAPPER_NAME(brk)
#endif
#ifdef HAVE_SBRK
WRAPPER_NAME(sbrk)
#endif
