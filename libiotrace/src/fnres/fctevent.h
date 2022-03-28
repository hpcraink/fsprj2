/**
 * Function event (fctevent):
 *   Takes in data from wrapper calls (posix/mpi; are called "function events") for the purpose
 *   of creating a mapping from all types of file handles (fildes, streams, etc.) to filenames
 */
#ifndef LIBIOTRACE_FCTEVENT_H
#define LIBIOTRACE_FCTEVENT_H

#include <stddef.h>     /* size_t */


/* -- Forward type declarations -- */
struct basic;           /* Note: #include "../libiotrace_include_struct.h" causes "Declaration of 'struct basic' will not be visible outside of this function" when incl. in "wrapper_defines.h" */


/* -- Constants -- */
/* Note: All files which aren't actual files have '_' as pre- & suffix */
#define FNAME_SPECIFIER_NAF             "_ NOT-A-FILE _"
#define FNAME_SPECIFIER_STD             "_ STD-IO _"
#define FNAME_SPECIFIER_PSEUDO          "_ PSEUDO-FILE _"
#define FNAME_SPECIFIER_MEMMAP          "_ MEM-MAPPING _"
#define FNAME_SPECIFIER_NOTFOUND        "_ NOT FOUND _"            /* May indicate error in module OR e.g., using invalid fildes (errno 9 Bad file descriptor) */
#define FNAME_SPECIFIER_UNSUPPORTED_FCT "__ UNKNOWN (NOT-YET-SUPPORTED) __"
#define FNAME_SPECIFIER_UNHANDELED_FCT  "__ UNKNOWN (UNHANDLED-FCT) __"


/* -- Function prototypes -- */
void fnres_init(long fnmap_max_size);
void fnres_fin(void);
// void fnres_reset_on_fork(void);

void fnres_trace_fctevent(struct basic *fctevent);

#endif /* LIBIOTRACE_FCTEVENT_H */
