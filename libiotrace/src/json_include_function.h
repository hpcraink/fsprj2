#ifndef LIBIOTRACE_JSON_INCLUDE_FUNCTION_H
#define LIBIOTRACE_JSON_INCLUDE_FUNCTION_H

#undef JSON_STRUCT
#include "json_structs.h"
#define JSON_STRUCT JSON_STRUCT_PRINT
#include "json_structs.h"
#undef JSON_STRUCT
#define JSON_STRUCT JSON_STRUCT_BYTES_COUNT
#include "json_structs.h"
#undef JSON_STRUCT
#define JSON_STRUCT JSON_STRUCT_SIZEOF
#include "json_structs.h"
#undef JSON_STRUCT
#define JSON_STRUCT JSON_STRUCT_COPY
#include "json_structs.h"
#undef JSON_STRUCT
#define JSON_STRUCT JSON_STRUCT_FREE
#include "json_structs.h"
#undef JSON_STRUCT
#define JSON_STRUCT JSON_STRUCT_PUSH_COUNT
#include "json_structs.h"
#undef JSON_STRUCT
#define JSON_STRUCT JSON_STRUCT_PUSH
#include "json_structs.h"

#endif /* LIBIOTRACE_JSON_INCLUDE_FUNCTION_H */
