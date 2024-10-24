/**
 * Function event (fctevent):
 *   Takes in data from wrapper calls (posix/mpi; are called "function events") for the purpose
 *   of creating a mapping from all types of file handles (fildes, streams, etc.) to filenames
 */
#ifndef LIBIOTRACE_FNRES_H
#define LIBIOTRACE_FNRES_H

#include <stddef.h>     /* size_t */


/* -- Forward type declarations -- */
struct basic;           /* Note: #include "../libiotrace_include_struct.h" causes "Declaration of 'struct basic' will not be visible outside of this function" when incl. in "wrapper_defines.h" */


/* -- Constants -- */
/* Note: All files which aren't actual files have '_' as pre- & suffix */
#define FNAME_SPECIFIER_NAF             "_ NOT-A-FILE _"
#define FNAME_SPECIFIER_STD             "_ STD-IO _"
#define FNAME_SPECIFIER_PSEUDO          "_ PSEUDO-FILE _"
#define FNAME_SPECIFIER_MMAP            "_ MEM-MAPPING _"
#define FNAME_SPECIFIER_NOTFOUND        "_ NOT FOUND _"            /* May indicate error in module OR e.g., using invalid fildes (errno 9 Bad file descriptor) */
#define FNAME_SPECIFIER_UNSUPPORTED_FCT "__ UNKNOWN (NOT-YET-SUPPORTED) __"
#define FNAME_SPECIFIER_UNHANDELED_FCT  "__ UNKNOWN (UNHANDLED-FCT) __"


/* -- Function prototypes -- */
/**
 * @brief                                 Initializes the filename resolution module
 *                                        NOTE: May terminate entire process if init fails
 *
 * @param[in] fnmap_max_size              Max # of file-handles which may be traced at any given point in time for current process
 * @return void
 */
void fnres_init(long fnmap_max_size);

/**
 * @brief                                 Finalizes the filename resolution module
 *
 * @return void
 */
void fnres_fin(void);


// void fnres_reset_on_fork(void);

/**
 * @brief                                 Traces passed io-event, i.e.,
 *                                          stores filename for new file-handles (e.g., for `open`) and
 *                                          sets associated filename on event (by looking up filename via handle)
 *
 * @param[in,out] ioevent_ptr             Will be updated to contain the traced filename
 * @return int                            `0` = successfully resolved filename, `-1` = couldn't resolve filename
 */
int fnres_trace_ioevent(struct basic *ioevent_ptr);

#endif /* LIBIOTRACE_FNRES_H */
