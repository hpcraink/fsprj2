#ifndef LIBIOTRACE_LD_IO_H
#define LIBIOTRACE_LD_IO_H

#include "libiotrace_config.h"

#include <dlfcn.h>
#include "wrapper_defines.h"

/* Function pointers for dl functions */

REAL_TYPE void * REAL(dlopen)(const char *filename, int flags) REAL_INIT;
#ifdef HAVE_DLMOPEN
REAL_TYPE void * REAL(dlmopen)(Lmid_t lmid, const char *filename, int flags) REAL_INIT;
#endif

#ifndef IO_LIB_STATIC
#undef DLSYM_INIT_DONE
#undef DLSYM_INIT_FUNCTION
#define DLSYM_INIT_DONE ld_io_init_done
#define DLSYM_INIT_FUNCTION ld_io_init
static char DLSYM_INIT_DONE = 0;
static void DLSYM_INIT_FUNCTION() ATTRIBUTE_CONSTRUCTOR;
/* Initialize pointers for dl functions. */
static void DLSYM_INIT_FUNCTION() {
	if (!DLSYM_INIT_DONE) {

		DLSYM(dlopen);
#ifdef HAVE_DLMOPEN
		DLSYM(dlmopen);
#endif

		DLSYM_INIT_DONE = 1;
	}
}
#endif

#endif /* LIBIOTRACE_LD_IO_H */
