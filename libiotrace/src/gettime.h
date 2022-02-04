/**
 * Headers for timing functions
 *
 * Copyright (c) 2021     Hochschule Esslingen, University of Applied Science
 */
#include "libiotrace_config.h"

BEGIN_C_DECLS

#ifdef HAVE_SYS_TYPES_H
#  include <sys/types.h>
#endif

/**
 * Returns the 64 bit time in nanosecond resolution since start of the computer
 * @ret   time in ns
 */
uint64_t gettime();

END_C_DECLS
