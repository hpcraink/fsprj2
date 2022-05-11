/**
 * Headers for timing functions
 *
 * Copyright (c) 2021     Hochschule Esslingen, University of Applied Science
 */
#include "libiotrace_config.h"

BEGIN_C_DECLS

#ifdef HAVE_STDINT_H
#  include <stdint.h>
#endif

/**
 * Returns the 64 bit time in nanosecond resolution since start of the computer
 * @ret   time in ns
 */
uint64_t gettime(void);

END_C_DECLS
