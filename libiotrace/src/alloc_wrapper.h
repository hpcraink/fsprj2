#include "wrapper_name.h"

WRAPPER_NAME(malloc)
WRAPPER_NAME(free)
WRAPPER_NAME(calloc)
WRAPPER_NAME(realloc)
#ifdef HAVE_REALLOCARRAY
WRAPPER_NAME(reallocarray)
#endif