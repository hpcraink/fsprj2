#include "libiotrace_functions.h"

#ifdef WITH_POSIX_IO
#  include "posix_io.h"
#endif
#ifdef WITH_POSIX_AIO
#  include "posix_aio.h"
#endif
#ifdef WITH_DL_IO
#  include "dl_io.h"
#endif
#ifdef WITH_MPI_IO
#  include "mpi_io.h"
#endif
#ifdef WITH_ALLOC
#  include "alloc.h"
#endif

#undef LIBIOTRACE_STRUCT
#include "libiotrace_structs.h"
#define LIBIOTRACE_STRUCT LIBIOTRACE_STRUCT_PRINT
#include "libiotrace_structs.h"
#undef LIBIOTRACE_STRUCT
#define LIBIOTRACE_STRUCT LIBIOTRACE_STRUCT_BYTES_COUNT
#include "libiotrace_structs.h"
#undef LIBIOTRACE_STRUCT
#define LIBIOTRACE_STRUCT LIBIOTRACE_STRUCT_SIZEOF
#include "libiotrace_structs.h"
#undef LIBIOTRACE_STRUCT
#define LIBIOTRACE_STRUCT LIBIOTRACE_STRUCT_COPY
#include "libiotrace_structs.h"
#undef LIBIOTRACE_STRUCT
#define LIBIOTRACE_STRUCT LIBIOTRACE_STRUCT_FREE
#include "libiotrace_structs.h"
#undef LIBIOTRACE_STRUCT
#define LIBIOTRACE_STRUCT LIBIOTRACE_STRUCT_PUSH_BYTES_COUNT
#include "libiotrace_structs.h"
#undef LIBIOTRACE_STRUCT
#define LIBIOTRACE_STRUCT LIBIOTRACE_STRUCT_PUSH
#include "libiotrace_structs.h"
