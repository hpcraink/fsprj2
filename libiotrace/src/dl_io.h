#ifndef LIBIOTRACE_LD_IO_H
#define LIBIOTRACE_LD_IO_H

#include "libiotrace_config.h"

#include <dlfcn.h>
#include "wrapper_defines.h"

/* Function pointers for dl functions */

BEGIN_C_DECLS

REAL_TYPE void * REAL(dlopen)(const char *filename, int flags) REAL_INIT;
#ifdef HAVE_DLMOPEN
REAL_TYPE void * REAL(dlmopen)(Lmid_t lmid, const char *filename, int flags) REAL_INIT;
#endif

#ifndef IO_LIB_STATIC
void dl_io_init() ATTRIBUTE_CONSTRUCTOR;
#endif

END_C_DECLS

#endif /* LIBIOTRACE_LD_IO_H */
