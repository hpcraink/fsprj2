#ifndef LIBIOTRACE_INCLUDE_FUNCTION_H
#define LIBIOTRACE_INCLUDE_FUNCTION_H

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

#endif /* LIBIOTRACE_INCLUDE_FUNCTION_H */