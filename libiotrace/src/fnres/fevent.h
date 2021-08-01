//
// Takes in data from wrapper calls ("function events") for the purpose
// of creating a mapping from all types of ids -> filenames
//

#ifndef LIBIOTRACE_FEVENT_H
#define LIBIOTRACE_FEVENT_H

struct basic;       // #include "../libiotrace_include_struct.h" causes "Declaration of 'struct basic' will not be visible outside of this function" when incl. in "wrapper_defines.h"

void add_fevent_to_trace(struct basic *data);


// Macro for compiling lib w/ or w/o this component
#ifdef WITH_FILENAME_RESOLUTION
#define ADD_FEVENT_TO_TRACE(var) add_fevent_to_trace(var);
#else
#define ADD_FEVENT_TO_TRACE(var)
#endif

#endif /* LIBIOTRACE_FEVENT_H */