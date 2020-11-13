#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <limits.h>
#include <assert.h>
#include <math.h>
#include "posix_io.h"

/*
 * To add a new data-type for generating the struct and the json-cstring seven lines
 * have to be added. The positions for adding a new data-type are marked with:
 * insert new line for new data-type here
 */

/* All intern variables are named with prefix "json_struct_" to prevent shadowing.
 * All functions are also named with prefix "json_struct_" to prevent conflicts. */

/* values for #define JSON_STRUCT */
#define JSON_STRUCT_DATA_TYPE   1 /* generate struct */
#define JSON_STRUCT_PRINT       2 /* generate print-function to print struct as json */
#define JSON_STRUCT_BYTES_COUNT 3 /* generate function to evaluate max size of json-string */
#define JSON_STRUCT_SIZEOF      4 /* generate function to evaluate size for JSON_STRUCT_COPY */
#define JSON_STRUCT_COPY        5 /* generate function to deep copy struct (with VOID_P elements) */
#define JSON_STRUCT_FREE        6 /* generate function to free malloc'ed memory */
#define JSON_STRUCT_PUSH_COUNT  7 /* generate function to evaluate size for HTTP Posts */
#define JSON_STRUCT_PUSH        8 /* generate function to generate POST request*/

/* #defines for error handling */
#ifndef JSON_STRUCT_ERROR
#  define JSON_STRUCT_ERROR
#  define JSON_STRUCT_ENUM_ERROR(value) CALL_REAL_POSIX_SYNC(fprintf)(stderr, "Unknown value \"%d\" of enum in function %s.\n", value, __func__); \
                                        /* ToDo: __func__ dependencies (like in posix_io.c) */ \
                                        assert(0);
#  define JSON_STRUCT_SIZE_ERROR(ret, size) if (ret >= size) { \
                                                CALL_REAL_POSIX_SYNC(fprintf)(stderr, "Output buffer in function %s not big enough.\n", __func__); \
                                                assert(0); \
                                            }
#endif

/* macros for setting VOID_P elements */
#define JSON_STRUCT_SET_VOID_P(struct_name, element, substruct_type, value) struct_name.void_p_enum_##element = \
                                                                                void_p_enum_##element##_##substruct_type; \
                                                                            struct_name.element = (void*) (&value);
#define JSON_STRUCT_SET_VOID_P_NULL(struct_name, element) struct_name.element = NULL;

/* macros for setting malloced string array */
#define JSON_STRUCT_SET_MALLOC_STRING_ARRAY(struct_name, array_name, malloced_array, start, size) struct_name.size_##array_name = size; \
                                                                                                  struct_name.start_##array_name = start; \
                                                                                                  struct_name.array_name = malloced_array;
#define JSON_STRUCT_SET_MALLOC_STRING_ARRAY_NULL(struct_name, array_name) struct_name.array_name = NULL;

/* macros for setting malloced ptr array */
#define JSON_STRUCT_SET_MALLOC_PTR_ARRAY(struct_name, array_name, malloced_array, start, size) struct_name.size_##array_name = size; \
                                                                                               struct_name.start_##array_name = start; \
                                                                                               struct_name.array_name = malloced_array;
#define JSON_STRUCT_SET_MALLOC_PTR_ARRAY_NULL(struct_name, array_name) struct_name.array_name = NULL;

/* macros for setting int array */
#define JSON_STRUCT_SET_INT_ARRAY(struct_name, array_name, array, size) struct_name.size_##array_name = size; \
                                                                        struct_name.array_name = array;
#define JSON_STRUCT_SET_INT_ARRAY_NULL(struct_name, array_name) struct_name.array_name = NULL;

/* macros for setting struct array */
#define JSON_STRUCT_SET_STRUCT_ARRAY(struct_name, array_name, array, size) struct_name.size_##array_name = size; \
                                                                           struct_name.array_name = array;
#define JSON_STRUCT_SET_STRUCT_ARRAY_NULL(struct_name, array_name) struct_name.array_name = NULL;

/* macros for setting key value array */
#define JSON_STRUCT_INIT_KEY_VALUE_ARRAY(struct_name, array_name, keys_array, values_array) struct_name.size_##array_name = 0; \
                                                                                            struct_name.keys_##array_name = keys_array; \
                                                                                            struct_name.values_##array_name = values_array;
#define JSON_STRUCT_SET_KEY_VALUE_ARRAY_NULL(struct_name, array_name) struct_name.size_##array_name = 0; \
                                                                      struct_name.keys_##array_name = NULL; \
                                                                      struct_name.values_##array_name = NULL;
#define JSON_STRUCT_ADD_KEY_VALUE(struct_name, array_name, key, value) struct_name.keys_##array_name[struct_name.size_##array_name] = key; \
                                                                       struct_name.values_##array_name[struct_name.size_##array_name] = value; \
                                                                       struct_name.size_##array_name++;

/* #define's for escaping cstrings (used in print_cstring()-function) */
#ifndef JSON_STRUCT_ESCAPE_SLASH
#  define JSON_STRUCT_ESCAPE_SLASH 0
#endif

#ifdef JSON_STRUCT

#undef JSON_STRUCT_ELEMENT_SIZE
#undef JSON_STRUCT_TYPE_SIZE_DEC

#undef JSON_STRUCT_ELEMENT
#undef JSON_STRUCT_SNPRINTF

#undef JSON_STRUCT_ENUM_START
#undef JSON_STRUCT_ENUM_ELEMENT
#undef JSON_STRUCT_ENUM_END

#undef JSON_STRUCT_ARRAY_BITFIELD_START
#undef JSON_STRUCT_ARRAY_BITFIELD_ELEMENT
#undef JSON_STRUCT_ARRAY_BITFIELD_END

#undef JSON_STRUCT_START
#undef JSON_STRUCT_END

#undef JSON_STRUCT_VOID_P_START
#undef JSON_STRUCT_VOID_P_ELEMENT
#undef JSON_STRUCT_VOID_P_END

#undef JSON_STRUCT_STRUCT_ARRAY

#undef JSON_STRUCT_STRUCT_P
#undef JSON_STRUCT_STRUCT
#undef JSON_STRUCT_ARRAY_BITFIELD
#undef JSON_STRUCT_ENUM
#undef JSON_STRUCT_INT
#undef JSON_STRUCT_PID_T
#undef JSON_STRUCT_CSTRING
#undef JSON_STRUCT_CSTRING_P
#undef JSON_STRUCT_CSTRING_P_CONST
#undef JSON_STRUCT_CLOCK_T
#undef JSON_STRUCT_FILE_P
#undef JSON_STRUCT_ENUM_START
#undef JSON_STRUCT_LONG_INT
#undef JSON_STRUCT_SIZE_T
#undef JSON_STRUCT_SSIZE_T
#undef JSON_STRUCT_OFF_T
#undef JSON_STRUCT_U_INT64_T
#undef JSON_STRUCT_VOID_P
#undef JSON_STRUCT_VOID_P_CONST
#undef JSON_STRUCT_FD_SET_P
#ifdef HAVE_DLMOPEN
#  undef JSON_STRUCT_LMID_T
#endif
#undef JSON_STRUCT_SHORT
#undef JSON_STRUCT_DEV_T
#undef JSON_STRUCT_INO_T
#undef JSON_STRUCT_MALLOC_STRING_ARRAY
#undef JSON_STRUCT_MALLOC_PTR_ARRAY
#undef JSON_STRUCT_INT_ARRAY
#undef JSON_STRUCT_SA_FAMILY_T
#undef JSON_STRUCT_KEY_VALUE_ARRAY
/* insert new line for new data-type here */

/* ----------------------------------------------------------------------------------------------------------------------- */
#if JSON_STRUCT == JSON_STRUCT_DATA_TYPE
/* ----------------------------------------------------------------------------------------------------------------------- */
/*
 * Macros for struct declaration
 * */

#  define JSON_STRUCT_ENUM_START(name) enum name {
#  define JSON_STRUCT_ENUM_ELEMENT(name) name,
#  define JSON_STRUCT_ENUM_END };

#  define JSON_STRUCT_ARRAY_BITFIELD_START(name) struct name {
#  define JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(name) unsigned int name : 1;
#  define JSON_STRUCT_ARRAY_BITFIELD_END };

#  define JSON_STRUCT_VOID_P_START(name) void *name; enum {
#  define JSON_STRUCT_VOID_P_ELEMENT(name, element) void_p_enum_##name##_##element,
#  define JSON_STRUCT_VOID_P_END(name) } void_p_enum_##name;

#  define JSON_STRUCT_START(name) struct name {
#  define JSON_STRUCT_END };

#  define JSON_STRUCT_STRUCT_ARRAY(type, name, max_length) struct type **name; int size_##name;

#  define JSON_STRUCT_STRUCT_P(type, name) struct type *name;
#  define JSON_STRUCT_STRUCT(type, name) struct type name;
#  define JSON_STRUCT_ARRAY_BITFIELD(type, name) struct type name;
#  define JSON_STRUCT_ENUM(type, name) enum type name;
#  define JSON_STRUCT_INT(name) int name;
#  define JSON_STRUCT_PID_T(name) pid_t name;
#  define JSON_STRUCT_CSTRING(name, length) char name[length];
#  define JSON_STRUCT_CSTRING_P(name, max_length) char *name;
#  define JSON_STRUCT_CSTRING_P_CONST(name, max_length) const char *name;
#  define JSON_STRUCT_CLOCK_T(name) clock_t name;
#  define JSON_STRUCT_FILE_P(name) FILE *name;
#  define JSON_STRUCT_LONG_INT(name) long int name;
#  define JSON_STRUCT_SIZE_T(name) size_t name;
#  define JSON_STRUCT_SSIZE_T(name) ssize_t name;
#  if HAVE_OFF64_T
#    define JSON_STRUCT_OFF_T(name) off64_t name;
#  else
#    define JSON_STRUCT_OFF_T(name) off_t name;
#  endif
#  define JSON_STRUCT_U_INT64_T(name) u_int64_t name;
#  define JSON_STRUCT_VOID_P(name) void *name;
#  define JSON_STRUCT_VOID_P_CONST(name) const void *name;
#  define JSON_STRUCT_FD_SET_P(name) fd_set *name;
#  ifdef HAVE_DLMOPEN
#    define JSON_STRUCT_LMID_T(name) Lmid_t name;
#  endif
#  define JSON_STRUCT_SHORT(name) short name;
#  define JSON_STRUCT_DEV_T(name) dev_t name;
#  define JSON_STRUCT_INO_T(name) ino_t name;
#  define JSON_STRUCT_MALLOC_STRING_ARRAY(name, max_size, max_length_per_element) int start_##name; int size_##name; char ** name;
#  define JSON_STRUCT_MALLOC_PTR_ARRAY(name, max_size) int start_##name; int size_##name; void ** name;
#  define JSON_STRUCT_INT_ARRAY(name, max_size) int size_##name; int * name;
#  define JSON_STRUCT_SA_FAMILY_T(name) sa_family_t name;
#  define JSON_STRUCT_KEY_VALUE_ARRAY(name, max_size, max_length_per_cstring) int size_##name; char ** keys_##name; char ** values_##name;
/* insert new line for new data-type here */

/* ----------------------------------------------------------------------------------------------------------------------- */
#elif JSON_STRUCT == JSON_STRUCT_PRINT
/* ----------------------------------------------------------------------------------------------------------------------- */
/*
 * Macros for serializing structs to json
 *
 * following functions are available after include of the macros
 *
 * int json_struct_print_<name of struct>(char* buf, size_t size, struct name *data)
 * int json_struct_print_array_<name of array>(char* buf, size_t size, struct name *data)
 * int json_struct_print_enum_<name of enum>(char* buf, size_t size, struct name *data)
 *
 * for printing struct as json-cstring. The size argument specifies the maximum number
 * of characters to produce.
 * */

#define JSON_STRUCT_ESCAPE_PRINT_2BYTES(character) if (json_struct_size - json_struct_ret > 2) { \
                                                       *json_struct_buf++ = '\\'; \
                                                       *json_struct_buf++ = character; \
                                                       json_struct_src++; \
                                                       json_struct_ret += 2; \
                                                   } else { \
                                                       json_struct_size = 0; /* buffer not big enough: break while */ \
                                                   }

int json_struct_print_cstring(char* json_struct_buf, size_t json_struct_size, const char* json_struct_src) {
    const char *json_struct_hex = "0123456789abcdef";
    int json_struct_ret = 0;

    if (json_struct_size > 2) {
        *json_struct_buf++ = '\"';
        json_struct_ret++;
    } else {
        return json_struct_ret;
    }

    if (NULL != json_struct_src) {
        /* ToDo: comment */
        while (*json_struct_src != '\0' && json_struct_size - json_struct_ret > 2) {
            if ((unsigned char)*json_struct_src >= ' ') {
                switch (*json_struct_src) {
                case '\"':
                case '\\':
#if JSON_STRUCT_ESCAPE_SLASH
                case '/':
#endif
                    JSON_STRUCT_ESCAPE_PRINT_2BYTES(*json_struct_src)
                    break;
                default:
                    *json_struct_buf++ = *json_struct_src++;
                    json_struct_ret++;
                }
            } else {
                switch (*json_struct_src) {
                case '\b':
                    JSON_STRUCT_ESCAPE_PRINT_2BYTES('b')
                    break;
                case '\f':
                    JSON_STRUCT_ESCAPE_PRINT_2BYTES('f')
                    break;
                case '\n':
                    JSON_STRUCT_ESCAPE_PRINT_2BYTES('n')
                    break;
                case '\r':
                    JSON_STRUCT_ESCAPE_PRINT_2BYTES('r')
                    break;
                case '\t':
                    JSON_STRUCT_ESCAPE_PRINT_2BYTES('t')
                    break;
                default:
                    /* '\a' and '\v' are not part of the JSON escape definition and
                     * have to be encoded as \u,
                     * also all other values < U+0020 (space ' ') have to be encoded
                     * as \u */
                    if (json_struct_size - json_struct_ret > 7) {
                        *json_struct_buf++ = '\\';
                        *json_struct_buf++ = 'u';
                        *json_struct_buf++ = '0';
                        *json_struct_buf++ = '0';
                        *json_struct_buf++ = json_struct_hex[((unsigned char)*json_struct_src >> 4)];
                        *json_struct_buf++ = json_struct_hex[(*json_struct_src) & 0x0f];
                        json_struct_src++;
                        json_struct_ret += 6;
                    } else {
                        json_struct_size = 0; /* buffer not big enough: break while */ \
                    }
                }
            }
        }
    }

    *json_struct_buf++ = '\"';
    json_struct_ret++;
    *json_struct_buf = '\0';
    return json_struct_ret;
}

int json_struct_write(char* json_struct_buf, size_t json_struct_size, const char* json_struct_src) {
    int json_struct_ret = 0;

    while (*json_struct_src != '\0' && json_struct_size - json_struct_ret > 1) {
        *json_struct_buf++ = *json_struct_src++;
        json_struct_ret++;
    }

    *json_struct_buf = '\0';
    return json_struct_ret;
}

#  define JSON_STRUCT_SNPRINTF(...) json_struct_ret = snprintf(json_struct_buf, json_struct_size, __VA_ARGS__); \
                                    JSON_STRUCT_SIZE_ERROR(json_struct_ret, json_struct_size) /* don't write more characters then size of buffer */ \
                                    json_struct_buf += json_struct_ret;  /* set pointer to end of written characters */ \
                                    json_struct_size -= json_struct_ret; /* resize buffer size */

#  define JSON_STRUCT_WRITE(value) json_struct_ret = json_struct_write(json_struct_buf, json_struct_size, value); \
                                   JSON_STRUCT_SIZE_ERROR(json_struct_ret, json_struct_size) /* don't write more characters then size of buffer */ \
                                   json_struct_buf += json_struct_ret;  /* set pointer to end of written characters */ \
                                   json_struct_size -= json_struct_ret; /* resize buffer size */

#  define JSON_STRUCT_QUOT(key) "\""#key"\""

#  define JSON_STRUCT_TYPE(name, function) json_struct_hasElements = 1; \
                                           JSON_STRUCT_WRITE(JSON_STRUCT_QUOT(name)":") \
                                           json_struct_ret = function(json_struct_buf, json_struct_size, \
                                                               &json_struct_data->name); \
                                           json_struct_buf += json_struct_ret;  /* set pointer to end of written characters */ \
                                           json_struct_size -= json_struct_ret; /* resize buffer size */
#  define JSON_STRUCT_ELEMENT(key, template, ...) json_struct_hasElements = 1; \
                                                  JSON_STRUCT_SNPRINTF(JSON_STRUCT_QUOT(key)":"#template",", __VA_ARGS__)
#  define JSON_STRUCT_ESCAPE(name) json_struct_hasElements = 1; \
                                   JSON_STRUCT_WRITE(JSON_STRUCT_QUOT(name)":") \
                                   json_struct_ret = json_struct_print_cstring(json_struct_buf, json_struct_size, \
                                                       json_struct_data->name); \
                                   JSON_STRUCT_SIZE_ERROR(json_struct_ret, json_struct_size) \
                                   json_struct_buf += json_struct_ret;  /* set pointer to end of written characters */ \
                                   json_struct_size -= json_struct_ret; /* resize buffer size */ \
                                   JSON_STRUCT_WRITE(",")

#  define JSON_STRUCT_ENUM_START(name) int json_struct_print_enum_##name(char* json_struct_buf, \
                                             size_t json_struct_size, enum name *json_struct_data) { \
                                         int json_struct_ret = 0; \
                                         int json_struct_start_size = json_struct_size; \
                                         switch (*json_struct_data) {
#  define JSON_STRUCT_ENUM_ELEMENT(name) case name: \
                                           JSON_STRUCT_WRITE(JSON_STRUCT_QUOT(name)",") \
                                           break;
#  define JSON_STRUCT_ENUM_END default: \
                                 JSON_STRUCT_ENUM_ERROR(*json_struct_data) \
                               } return json_struct_start_size - json_struct_size;}

#  define JSON_STRUCT_ARRAY_BITFIELD_START(name) int json_struct_print_array_##name(char* json_struct_buf, \
                                                       size_t json_struct_size, struct name *json_struct_data) { \
                                                   char json_struct_hasElements = 0; \
                                                   int json_struct_ret = 0; \
                                                   int json_struct_start_size = json_struct_size; \
                                                   JSON_STRUCT_WRITE("[")
#  define JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(name) if (json_struct_data->name) { \
                                                     json_struct_hasElements = 1; \
                                                     JSON_STRUCT_WRITE(JSON_STRUCT_QUOT(name)",") \
                                                   }
#  define JSON_STRUCT_ARRAY_BITFIELD_END if (json_struct_hasElements) { \
                                           json_struct_buf--;  /* remove last comma */ \
                                           json_struct_size++; /* and resize buffer size */ \
                                         } \
                                         JSON_STRUCT_WRITE("],") \
                                         return json_struct_start_size - json_struct_size;}

#  define JSON_STRUCT_START(name) int json_struct_print_##name(char* json_struct_buf, size_t json_struct_size, \
                                        struct name *json_struct_data) { \
                                    char json_struct_hasElements = 0; \
                                    int json_struct_ret = 0; \
                                    int json_struct_start_size = json_struct_size; \
                                    JSON_STRUCT_WRITE("{")
#  define JSON_STRUCT_END if (json_struct_hasElements) { \
                            json_struct_buf--;  /* remove last comma */ \
                            json_struct_size++; /* and resize buffer size */ \
                          } \
                          JSON_STRUCT_WRITE("}") \
                          return json_struct_start_size - json_struct_size;}

#  define JSON_STRUCT_VOID_P_START(name) if (NULL != json_struct_data->name) { \
                                           json_struct_hasElements = 1; \
                                           JSON_STRUCT_WRITE(JSON_STRUCT_QUOT(name)":") \
                                           switch (json_struct_data->void_p_enum_##name) {
#  define JSON_STRUCT_VOID_P_ELEMENT(name, element) case void_p_enum_##name##_##element: \
                                                      json_struct_ret = json_struct_print_##element(json_struct_buf, \
                                                                          json_struct_size, (struct element*) \
                                                                          json_struct_data->name); \
                                                      json_struct_buf += json_struct_ret;  /* set pointer to end of written characters */ \
                                                      json_struct_size -= json_struct_ret; /* resize buffer size */\
                                                      break;
#  define JSON_STRUCT_VOID_P_END(name)   default: \
                                           JSON_STRUCT_ENUM_ERROR(json_struct_data->void_p_enum_##name) \
                                         } \
                                         JSON_STRUCT_WRITE(",") \
                                       }

#  define JSON_STRUCT_STRUCT_ARRAY(type, name, max_length) if (NULL != json_struct_data->name) { \
                                                             json_struct_hasElements = 1; \
                                                             JSON_STRUCT_WRITE(JSON_STRUCT_QUOT(name)":[") \
                                                             int json_struct_count_##name; \
                                                             for (json_struct_count_##name = 0; \
                                                                  json_struct_count_##name < json_struct_data->size_##name; \
                                                                  json_struct_count_##name++) { \
                                                               json_struct_ret = json_struct_print_##type(json_struct_buf, \
                                                                                                          json_struct_size, \
                                                                                                          *((json_struct_data->name) + json_struct_count_##name)); \
                                                               json_struct_buf += json_struct_ret;  /* set pointer to end of written characters */ \
                                                               json_struct_size -= json_struct_ret; /* resize buffer size */ \
                                                               JSON_STRUCT_WRITE(",") \
                                                             } \
                                                             if (json_struct_count_##name > 0) { \
                                                               json_struct_buf--;  /* remove last comma */ \
                                                               json_struct_size++; /* and resize buffer size */ \
                                                             } \
                                                             JSON_STRUCT_WRITE("],") \
                                                           }

#  define JSON_STRUCT_STRUCT_P(type, name) if (NULL != json_struct_data->name) { \
                                             json_struct_hasElements = 1; \
                                             JSON_STRUCT_WRITE(JSON_STRUCT_QUOT(name)":") \
                                             json_struct_ret = json_struct_print_##type(json_struct_buf, json_struct_size, \
                                                                                        json_struct_data->name); \
                                             json_struct_buf += json_struct_ret;  /* set pointer to end of written characters */ \
                                             json_struct_size -= json_struct_ret; /* resize buffer size */ \
                                             JSON_STRUCT_WRITE(",") \
                                           }
#  define JSON_STRUCT_STRUCT(type, name) JSON_STRUCT_TYPE(name, json_struct_print_##type) \
                                         JSON_STRUCT_WRITE(",")
#  define JSON_STRUCT_ARRAY_BITFIELD(type, name) JSON_STRUCT_TYPE(name, json_struct_print_array_##type)
#  define JSON_STRUCT_ENUM(type, name) JSON_STRUCT_TYPE(name, json_struct_print_enum_##type)
#  define JSON_STRUCT_INT(name) JSON_STRUCT_ELEMENT(name, %d, json_struct_data->name)
#  define JSON_STRUCT_PID_T(name) JSON_STRUCT_ELEMENT(name, %u, json_struct_data->name)
#  define JSON_STRUCT_CSTRING(name, length) JSON_STRUCT_ESCAPE(name)
#  define JSON_STRUCT_CSTRING_P(name, max_length) JSON_STRUCT_ESCAPE(name)
#  define JSON_STRUCT_CSTRING_P_CONST(name, max_length) JSON_STRUCT_ESCAPE(name)
#  define JSON_STRUCT_CLOCK_T(name) JSON_STRUCT_ELEMENT(name, %lu, json_struct_data->name)
#  define JSON_STRUCT_FILE_P(name) JSON_STRUCT_ELEMENT(name, "%p", json_struct_data->name)
#  define JSON_STRUCT_LONG_INT(name) JSON_STRUCT_ELEMENT(name, %ld, json_struct_data->name)
#  define JSON_STRUCT_SIZE_T(name) JSON_STRUCT_ELEMENT(name, %ld, json_struct_data->name)
#  define JSON_STRUCT_SSIZE_T(name) JSON_STRUCT_ELEMENT(name, %ld, json_struct_data->name)
#  define JSON_STRUCT_OFF_T(name) JSON_STRUCT_ELEMENT(name, %ld, json_struct_data->name)
#  define JSON_STRUCT_U_INT64_T(name) JSON_STRUCT_ELEMENT(name, %lu, json_struct_data->name)
#  define JSON_STRUCT_VOID_P(name) JSON_STRUCT_ELEMENT(name, "%p", json_struct_data->name)
#  define JSON_STRUCT_VOID_P_CONST(name) JSON_STRUCT_ELEMENT(name, "%p", json_struct_data->name)
#  define JSON_STRUCT_FD_SET_P(name) if (NULL != json_struct_data->name) { \
                                       json_struct_hasElements = 1; \
                                       JSON_STRUCT_WRITE(JSON_STRUCT_QUOT(name)":[") \
                                       int json_struct_count_##name; \
                                       for (json_struct_count_##name = 0; json_struct_count_##name < FD_SETSIZE; json_struct_count_##name++) { \
                                         if (FD_ISSET(json_struct_count_##name, json_struct_data->name)) { \
                                           JSON_STRUCT_SNPRINTF("%d,", json_struct_count_##name) \
                                         } \
                                       } \
                                       if (json_struct_count_##name > 0) { \
                                         json_struct_buf--;  /* remove last comma */ \
                                         json_struct_size++; /* and resize buffer size */ \
                                       } \
                                       JSON_STRUCT_WRITE("],") \
                                     }
#  ifdef HAVE_DLMOPEN
#    define JSON_STRUCT_LMID_T(name) JSON_STRUCT_ELEMENT(name, %ld, json_struct_data->name)
#  endif
#  define JSON_STRUCT_SHORT(name) JSON_STRUCT_ELEMENT(name, %d, json_struct_data->name)
#  define JSON_STRUCT_DEV_T(name) JSON_STRUCT_ELEMENT(name, %lu, json_struct_data->name)
#  define JSON_STRUCT_INO_T(name) JSON_STRUCT_ELEMENT(name, %lu, json_struct_data->name)
#  define JSON_STRUCT_MALLOC_STRING_ARRAY(name, max_size, max_length_per_element) if (NULL != json_struct_data->name) { \
                                                                                    json_struct_hasElements = 1; \
                                                                                    JSON_STRUCT_WRITE(JSON_STRUCT_QUOT(name)":[") \
                                                                                    int json_struct_count_##name; \
                                                                                    for (json_struct_count_##name = json_struct_data->start_##name; \
                                                                                         json_struct_count_##name < json_struct_data->size_##name; \
                                                                                         json_struct_count_##name++) { \
                                                                                      json_struct_ret = json_struct_print_cstring(json_struct_buf, json_struct_size, \
                                                                                                                                  json_struct_data->name[json_struct_count_##name]); \
                                                                                      JSON_STRUCT_SIZE_ERROR(json_struct_ret, json_struct_size) \
                                                                                      json_struct_buf += json_struct_ret;  /* set pointer to end of written characters */ \
                                                                                      json_struct_size -= json_struct_ret; /* resize buffer size */ \
                                                                                      JSON_STRUCT_WRITE(",") \
                                                                                    } \
                                                                                    if (json_struct_count_##name > 0) { \
                                                                                      json_struct_buf--;  /* remove last comma */ \
                                                                                      json_struct_size++; /* and resize buffer size */ \
                                                                                    } \
                                                                                    JSON_STRUCT_WRITE("],") \
                                                                                  }
#  define JSON_STRUCT_MALLOC_PTR_ARRAY(name, max_size) if (NULL != json_struct_data->name) { \
                                                         json_struct_hasElements = 1; \
                                                         JSON_STRUCT_WRITE(JSON_STRUCT_QUOT(name)":[") \
                                                         int json_struct_count_##name; \
                                                         for (json_struct_count_##name = json_struct_data->start_##name; \
                                                              json_struct_count_##name < json_struct_data->size_##name; \
                                                              json_struct_count_##name++) { \
                                                           JSON_STRUCT_SNPRINTF("\"%p\",", json_struct_data->name[json_struct_count_##name]) \
                                                         } \
                                                         if (json_struct_count_##name > 0) { \
                                                           json_struct_buf--;  /* remove last comma */ \
                                                           json_struct_size++; /* and resize buffer size */ \
                                                         } \
                                                         JSON_STRUCT_WRITE("],") \
                                                       }
#  define JSON_STRUCT_INT_ARRAY(name, max_size) if (NULL != json_struct_data->name) { \
                                                  json_struct_hasElements = 1; \
                                                  JSON_STRUCT_WRITE(JSON_STRUCT_QUOT(name)":[") \
                                                  int json_struct_count_##name; \
                                                  for (json_struct_count_##name = 0; \
                                                       json_struct_count_##name < json_struct_data->size_##name; \
                                                       json_struct_count_##name++) { \
                                                    JSON_STRUCT_SNPRINTF("%d,", json_struct_data->name[json_struct_count_##name]) \
                                                  } \
                                                  if (json_struct_count_##name > 0) { \
                                                    json_struct_buf--;  /* remove last comma */ \
                                                    json_struct_size++; /* and resize buffer size */ \
                                                  } \
                                                  JSON_STRUCT_WRITE("],") \
                                                }
#  define JSON_STRUCT_SA_FAMILY_T(name) JSON_STRUCT_ELEMENT(name, %hu, json_struct_data->name)
#  define JSON_STRUCT_KEY_VALUE_ARRAY(name, max_size, max_length_per_cstring) if (NULL != json_struct_data->keys_##name) { \
                                                                                json_struct_hasElements = 1; \
                                                                                JSON_STRUCT_WRITE(JSON_STRUCT_QUOT(name)":{") \
                                                                                int json_struct_count_##name; \
                                                                                for (json_struct_count_##name = 0; \
                                                                                     json_struct_count_##name < json_struct_data->size_##name; \
                                                                                     json_struct_count_##name++) { \
                                                                                  json_struct_ret = json_struct_print_cstring(json_struct_buf, json_struct_size, \
                                                                                                      json_struct_data->keys_##name[json_struct_count_##name]); \
                                                                                  JSON_STRUCT_SIZE_ERROR(json_struct_ret, json_struct_size) \
                                                                                  json_struct_buf += json_struct_ret;  /* set pointer to end of written characters */ \
                                                                                  json_struct_size -= json_struct_ret; /* resize buffer size */ \
                                                                                  JSON_STRUCT_WRITE(":") \
                                                                                  json_struct_ret = json_struct_print_cstring(json_struct_buf, json_struct_size, \
                                                                                                      json_struct_data->values_##name[json_struct_count_##name]); \
                                                                                  JSON_STRUCT_SIZE_ERROR(json_struct_ret, json_struct_size) \
                                                                                  json_struct_buf += json_struct_ret;  /* set pointer to end of written characters */ \
                                                                                  json_struct_size -= json_struct_ret; /* resize buffer size */ \
                                                                                  JSON_STRUCT_WRITE(",") \
                                                                                } \
                                                                                if (json_struct_count_##name > 0) { \
                                                                                  json_struct_buf--;  /* remove last comma */ \
                                                                                  json_struct_size++; /* and resize buffer size */ \
                                                                                } \
                                                                                JSON_STRUCT_WRITE("},") \
                                                                              }
/* insert new line for new data-type here */

/* ----------------------------------------------------------------------------------------------------------------------- */
#elif JSON_STRUCT == JSON_STRUCT_BYTES_COUNT
/* ----------------------------------------------------------------------------------------------------------------------- */
/*
 * Macros for evaluating the maximum length of a serialized struct
 *
 * following functions are available after include of the macros
 *
 * int json_struct_max_size_<name of struct>()
 * int json_struct_max_size_array_<name of struct>()
 * int json_struct_max_size_enum_<name of struct>()
 *
 * for evaluating max size of json-string. The returned size is without trailing
 * null character.
 * These functions can be used to create a big enough buffer for functions included
 * with JSON_STRUCT == JSON_STRUCT_PRINT.
 * */

/* compiler replaces following line with constants */
#  define JSON_STRUCT_TYPE_SIZE_DEC(type) ceil(log10(pow(2, sizeof(type) * CHAR_BIT)))
#  define JSON_STRUCT_TYPE_SIZE_HEX(type) ((sizeof(type) * 2) + 2) /* +2 for "0x"-prefix */
#  define JSON_STRUCT_ELEMENT_SIZE(name, sizeValue) json_struct_hasElements = 1; \
                                                    json_struct_size += sizeof(#name) \
                                                                        - 1  /* trailing null character */ \
                                                                        + sizeValue \
                                                                        + 4; /* quotation marks (for key), colon and comma */

#  define JSON_STRUCT_ENUM_START(name) int json_struct_max_size_enum_##name() { \
                                         size_t json_struct_size_value = 0;
#  define JSON_STRUCT_ENUM_ELEMENT(name) if (sizeof(#name) > json_struct_size_value) \
                                           json_struct_size_value = sizeof(#name); /* get greatest possible value */
#  define JSON_STRUCT_ENUM_END return json_struct_size_value \
                                      - 1   /* trailing null character */ \
                                      + 2;} /* quotation marks (for value) */

#  define JSON_STRUCT_ARRAY_BITFIELD_START(name) int json_struct_max_size_array_##name() { \
                                                   char json_struct_hasElements = 0; \
                                                   size_t json_struct_size = 1; /* open parentheses */
#  define JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(name) json_struct_hasElements = 1; \
                                                   json_struct_size += sizeof(#name) /* add each possible value */ \
                                                                       - 1  /* trailing null character */ \
                                                                       + 3; /* quotation marks and comma */
#  define JSON_STRUCT_ARRAY_BITFIELD_END if (json_struct_hasElements) json_struct_size--; /* remove last comma */ \
                                         return json_struct_size + 1;} /* close parentheses */

#  define JSON_STRUCT_START(name) int json_struct_max_size_##name() { \
                                    char json_struct_hasElements = 0; \
                                    int json_struct_size_void_p; \
                                    int json_struct_size_void_p_tmp; \
                                    size_t json_struct_size = 1; /* open parentheses */
#  define JSON_STRUCT_END if (json_struct_hasElements) json_struct_size--; /* remove last comma */ \
                          json_struct_size++; /* close parentheses */ \
                          return json_struct_size;}

#  define JSON_STRUCT_VOID_P_START(name) json_struct_hasElements = 1; \
                                         json_struct_size += sizeof(#name) \
                                                             - 1  /* trailing null character */ \
                                                             + 4; /* quotation marks (for key), colon and comma */ \
                                         json_struct_size_void_p = 0;
#  define JSON_STRUCT_VOID_P_ELEMENT(name, element) json_struct_size_void_p_tmp = json_struct_max_size_##element(); \
                                                    if(json_struct_size_void_p_tmp > json_struct_size_void_p) \
                                                      json_struct_size_void_p = json_struct_size_void_p_tmp;
#  define JSON_STRUCT_VOID_P_END(name) json_struct_size += json_struct_size_void_p;

#  define JSON_STRUCT_STRUCT_ARRAY(type, name, max_length) JSON_STRUCT_ELEMENT_SIZE(name, (json_struct_max_size_##type() \
                                                                                           + 1) /* +1 for comma */ \
                                                                                          * max_length \
                                                                                          - 1   /* for last comma */ \
                                                                                          + 2)  /* for brackets [] */

#  define JSON_STRUCT_STRUCT_P(type, name) JSON_STRUCT_ELEMENT_SIZE(name, json_struct_max_size_##type())
#  define JSON_STRUCT_STRUCT(type, name) JSON_STRUCT_ELEMENT_SIZE(name, json_struct_max_size_##type())
#  define JSON_STRUCT_ARRAY_BITFIELD(type, name) JSON_STRUCT_ELEMENT_SIZE(name, json_struct_max_size_array_##type())
#  define JSON_STRUCT_ENUM(type, name) JSON_STRUCT_ELEMENT_SIZE(name, json_struct_max_size_enum_##type())
#  define JSON_STRUCT_INT(name) JSON_STRUCT_ELEMENT_SIZE(name, JSON_STRUCT_TYPE_SIZE_DEC(int) \
                                                               + 1) /* for sign (-) */
#  define JSON_STRUCT_PID_T(name) JSON_STRUCT_ELEMENT_SIZE(name, JSON_STRUCT_TYPE_SIZE_DEC(pid_t))
#  define JSON_STRUCT_CSTRING(name,length) JSON_STRUCT_ELEMENT_SIZE(name, (length - 1) /* -1 trailing null character */ \
                                                                          * 6  /* *6 for escaping (\u00ff) */ \
                                                                          + 2) /* +2 quotation marks (for value) */
#  define JSON_STRUCT_CSTRING_P(name, max_length) JSON_STRUCT_CSTRING(name, max_length)
#  define JSON_STRUCT_CSTRING_P_CONST(name, max_length) JSON_STRUCT_CSTRING(name, max_length)
#  define JSON_STRUCT_CLOCK_T(name) JSON_STRUCT_ELEMENT_SIZE(name, JSON_STRUCT_TYPE_SIZE_DEC(clock_t))
#  define JSON_STRUCT_FILE_P(name) JSON_STRUCT_ELEMENT_SIZE(name, JSON_STRUCT_TYPE_SIZE_HEX(FILE*) \
                                                                  + 2) /* quotation marks (for value) */
#  define JSON_STRUCT_LONG_INT(name) JSON_STRUCT_ELEMENT_SIZE(name, JSON_STRUCT_TYPE_SIZE_DEC(long int) \
                                                                    + 1) /* for sign (-) */
#  define JSON_STRUCT_SIZE_T(name) JSON_STRUCT_ELEMENT_SIZE(name, JSON_STRUCT_TYPE_SIZE_DEC(size_t) \
                                                                  + 1) /* for sign (-) */
#  define JSON_STRUCT_SSIZE_T(name) JSON_STRUCT_ELEMENT_SIZE(name, JSON_STRUCT_TYPE_SIZE_DEC(ssize_t) \
                                                                   + 1) /* for sign (-) */
#  if HAVE_OFF64_T
#    define JSON_STRUCT_OFF_T(name) JSON_STRUCT_ELEMENT_SIZE(name, JSON_STRUCT_TYPE_SIZE_DEC(off64_t) \
                                                                   + 1) /* for sign (-) */
#  else
#    define JSON_STRUCT_OFF_T(name) JSON_STRUCT_ELEMENT_SIZE(name, JSON_STRUCT_TYPE_SIZE_DEC(off_t) \
                                                                   + 1) /* for sign (-) */
#  endif
#  define JSON_STRUCT_U_INT64_T(name) JSON_STRUCT_ELEMENT_SIZE(name, JSON_STRUCT_TYPE_SIZE_DEC(u_int64_t))
#  define JSON_STRUCT_VOID_P(name) JSON_STRUCT_ELEMENT_SIZE(name, JSON_STRUCT_TYPE_SIZE_HEX(void*) \
                                                                  + 2) /* quotation marks (for value) */
#  define JSON_STRUCT_VOID_P_CONST(name) JSON_STRUCT_ELEMENT_SIZE(name, JSON_STRUCT_TYPE_SIZE_HEX(void*) \
                                                                        + 2) /* quotation marks (for value) */
#  define JSON_STRUCT_FD_SET_P(name) JSON_STRUCT_ELEMENT_SIZE(name, ((ceil(log10(FD_SETSIZE)) \
                                                                      + 1) /* for comma */ \
                                                                     * FD_SETSIZE) \
                                                                    - 1  /* for last comma */ \
                                                                    + 2) /* for brackets [] */
#  ifdef HAVE_DLMOPEN
#    define JSON_STRUCT_LMID_T(name) JSON_STRUCT_ELEMENT_SIZE(name, JSON_STRUCT_TYPE_SIZE_DEC(Lmid_t) \
                                                                    + 1) /* for sign (-) */
#  endif
#  define JSON_STRUCT_SHORT(name) JSON_STRUCT_ELEMENT_SIZE(name, JSON_STRUCT_TYPE_SIZE_DEC(short) \
                                                                 + 1) /* for sign (-) */
#  define JSON_STRUCT_DEV_T(name) JSON_STRUCT_ELEMENT_SIZE(name, JSON_STRUCT_TYPE_SIZE_DEC(dev_t))
#  define JSON_STRUCT_INO_T(name) JSON_STRUCT_ELEMENT_SIZE(name, JSON_STRUCT_TYPE_SIZE_DEC(dev_t))
#  define JSON_STRUCT_MALLOC_STRING_ARRAY(name, max_size, max_length_per_element) JSON_STRUCT_ELEMENT_SIZE(name, ((max_length_per_element - 1) /* -1 trailing null character */ \
                                                                                                                  * 6  /* *6 for escaping (\u00ff) */ \
                                                                                                                  + 2  /* +2 quotation marks (for value) */ \
                                                                                                                  + 1) /* +1 for comma */ \
                                                                                                                 * max_size \
                                                                                                                 - 1   /* for last comma */ \
                                                                                                                 + 2)  /* for brackets [] */
#  define JSON_STRUCT_MALLOC_PTR_ARRAY(name, max_size) JSON_STRUCT_ELEMENT_SIZE(name, (JSON_STRUCT_TYPE_SIZE_HEX(void*) \
                                                                                       + 2  /* +2 quotation marks (for value) */ \
                                                                                       + 1) /* +1 for comma */ \
                                                                                      * max_size \
                                                                                      - 1   /* for last comma */ \
                                                                                      + 2)  /* for brackets [] */
#  define JSON_STRUCT_INT_ARRAY(name, max_size) JSON_STRUCT_ELEMENT_SIZE(name, (JSON_STRUCT_TYPE_SIZE_DEC(int) \
                                                                                + 1) /* +1 for comma */ \
                                                                               * max_size \
                                                                               - 1   /* for last comma */ \
                                                                               + 2)  /* for brackets [] */
#  define JSON_STRUCT_SA_FAMILY_T(name) JSON_STRUCT_ELEMENT_SIZE(name, JSON_STRUCT_TYPE_SIZE_DEC(sa_family_t))
#  define JSON_STRUCT_KEY_VALUE_ARRAY(name, max_size, max_length_per_cstring) JSON_STRUCT_ELEMENT_SIZE(name, (((max_length_per_cstring - 1) /* -1 trailing null character */ \
                                                                                                               * 6  /* *6 for escaping (\u00ff) */ \
                                                                                                               + 2) /* +2 quotation marks (for key or value) */ \
                                                                                                              * 2   /* *2 for key and value */ \
                                                                                                              + 1   /* +1 for colon */ \
                                                                                                              + 1)  /* +1 for comma */ \
                                                                                                             * max_size \
                                                                                                             - 1    /* for last comma */ \
                                                                                                             + 2)   /* for brackets {} */
/* insert new line for new data-type here */

/* ----------------------------------------------------------------------------------------------------------------------- */
#elif JSON_STRUCT == JSON_STRUCT_SIZEOF
/* ----------------------------------------------------------------------------------------------------------------------- */
/*
 * Macros for evaluating the size needed for a deep copy of a struct
 *
 * following functions are available after include of the macros
 *
 * int json_struct_sizeof_<name of struct>(struct name *json_struct_data)
 *
 * for evaluating max size of struct.
 * These functions can be used to evaluate needed buffer size for deep
 * copy functions included with JSON_STRUCT == JSON_STRUCT_COPY.
 * */

#  define JSON_STRUCT_ENUM_START(name)
#  define JSON_STRUCT_ENUM_ELEMENT(name)
#  define JSON_STRUCT_ENUM_END

#  define JSON_STRUCT_ARRAY_BITFIELD_START(name)
#  define JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(name)
#  define JSON_STRUCT_ARRAY_BITFIELD_END

#  define JSON_STRUCT_VOID_P_START(name) if (NULL != json_struct_data->name) { \
                                           switch (json_struct_data->void_p_enum_##name) {
#  define JSON_STRUCT_VOID_P_ELEMENT(name, element) case void_p_enum_##name##_##element: \
                                                      json_struct_size += json_struct_sizeof_##element( \
                                                                            (struct element*) json_struct_data->name); \
                                                      break;
#  define JSON_STRUCT_VOID_P_END(name) } }

#  define JSON_STRUCT_START(name) int json_struct_sizeof_##name(struct name *json_struct_data) { \
                                    size_t json_struct_size = sizeof(struct name);
#  define JSON_STRUCT_END return json_struct_size;}

#  define JSON_STRUCT_STRUCT_ARRAY(type, name, max_length) if (NULL != json_struct_data->name) { \
                                                             json_struct_size += sizeof(struct type *) \
                                                                                 * json_struct_data->size_##name; \
                                                             int json_struct_count_##name; \
                                                             for (json_struct_count_##name = 0; \
                                                                  json_struct_count_##name < json_struct_data->size_##name; \
                                                                  json_struct_count_##name++) { \
                                                               json_struct_size += json_struct_sizeof_##type( \
                                                                                     *((json_struct_data->name) + json_struct_count_##name)); \
                                                             } \
                                                           }

#  define JSON_STRUCT_STRUCT_P(type, name) if (NULL != json_struct_data->name) { \
                                             json_struct_size += json_struct_sizeof_##type( \
                                                                   json_struct_data->name); \
                                           }
#  define JSON_STRUCT_STRUCT(type, name)
#  define JSON_STRUCT_ARRAY_BITFIELD(type, name)
#  define JSON_STRUCT_ENUM(type, name)
#  define JSON_STRUCT_INT(name)
#  define JSON_STRUCT_PID_T(name)
#  define JSON_STRUCT_CSTRING(name, length)
#  define JSON_STRUCT_CSTRING_P(name, max_length) if (NULL != json_struct_data->name) { \
                                                    json_struct_size += strnlen(json_struct_data->name, max_length) \
                                                                        + 1; /* +1 for trailing null character */ \
                                                  }
#  define JSON_STRUCT_CSTRING_P_CONST(name, max_length) JSON_STRUCT_CSTRING_P(name, max_length)
#  define JSON_STRUCT_CLOCK_T(name)
#  define JSON_STRUCT_FILE_P(name)
#  define JSON_STRUCT_LONG_INT(name)
#  define JSON_STRUCT_SIZE_T(name)
#  define JSON_STRUCT_SSIZE_T(name)
#  define JSON_STRUCT_OFF_T(name)
#  define JSON_STRUCT_U_INT64_T(name)
#  define JSON_STRUCT_VOID_P(name)
#  define JSON_STRUCT_VOID_P_CONST(name)
#  define JSON_STRUCT_FD_SET_P(name) if (NULL != json_struct_data->name) { \
                                       json_struct_size += sizeof(fd_set); \
                                     }
#  ifdef HAVE_DLMOPEN
#    define JSON_STRUCT_LMID_T(name)
#  endif
#  define JSON_STRUCT_SHORT(name)
#  define JSON_STRUCT_DEV_T(name)
#  define JSON_STRUCT_INO_T(name)
#  define JSON_STRUCT_MALLOC_STRING_ARRAY(name, max_size, max_length_per_element) if (NULL != json_struct_data->name) { \
                                                                                    json_struct_size += sizeof(char *) * (json_struct_data->size_##name \
                                                                                                        - json_struct_data->start_##name); \
                                                                                    for (int json_struct_count_##name = json_struct_data->start_##name; \
                                                                                         json_struct_count_##name < json_struct_data->size_##name && \
                                                                                         json_struct_count_##name < max_size; \
                                                                                         json_struct_count_##name++) { \
                                                                                      json_struct_size += strnlen(json_struct_data->name[json_struct_count_##name], \
                                                                                                                  max_length_per_element) \
                                                                                                          + 1; /* +1 for trailing null character */ \
                                                                                    } \
                                                                                  }
#  define JSON_STRUCT_MALLOC_PTR_ARRAY(name, max_size) if (NULL != json_struct_data->name) { \
                                                         json_struct_size += sizeof(void *) * (json_struct_data->size_##name \
                                                                             - json_struct_data->start_##name); \
                                                       }
#  define JSON_STRUCT_INT_ARRAY(name, max_size) if (NULL != json_struct_data->name) { \
                                                  json_struct_size += sizeof(int) * json_struct_data->size_##name; \
                                                }
#  define JSON_STRUCT_SA_FAMILY_T(name)
#  define JSON_STRUCT_KEY_VALUE_ARRAY(name, max_size, max_length_per_cstring) if (NULL != json_struct_data->keys_##name) { \
                                                                                json_struct_size += sizeof(char *) * json_struct_data->size_##name \
                                                                                                    * 2; /* key and value arrays */ \
                                                                                for (int json_struct_count_##name = 0; \
                                                                                     json_struct_count_##name < json_struct_data->size_##name && \
                                                                                     json_struct_count_##name < max_size; \
                                                                                     json_struct_count_##name++) { \
                                                                                  json_struct_size += strnlen(json_struct_data->keys_##name[json_struct_count_##name], \
                                                                                                              max_length_per_cstring) \
                                                                                                      + 1; /* +1 for trailing null character */ \
                                                                                  json_struct_size += strnlen(json_struct_data->values_##name[json_struct_count_##name], \
                                                                                                              max_length_per_cstring) \
                                                                                                      + 1; /* +1 for trailing null character */ \
                                                                                } \
                                                                              }
/* insert new line for new data-type here */

/* ----------------------------------------------------------------------------------------------------------------------- */
#elif JSON_STRUCT == JSON_STRUCT_COPY
/* ----------------------------------------------------------------------------------------------------------------------- */
/*
 * Macros for deep copy a struct
 *
 * following functions are available after include of the macros
 *
 * void* json_struct_copy_<name of struct>(void *json_struct_buf, struct name *json_struct_data)
 *
 * for deep copy a struct.
 * Functions can be used to write collected data into a buffer.
 * */

int json_struct_copy_cstring_p(char *json_struct_to, const char *json_struct_from, size_t json_struct_max_length) {
    int json_struct_i;
    for (json_struct_i = 1; json_struct_i < json_struct_max_length && *json_struct_from != '\0'; json_struct_i++) {
        *json_struct_to = *json_struct_from;
        json_struct_to++;
        json_struct_from++;
    }
    if (json_struct_i == json_struct_max_length && *json_struct_from != '\0') {
        /* it's ok to write on (json_struct_to + json_struct_max_length), because expected size in
         * json_struct_sizeof_##name(struct name *json_struct_data) is +1 for trailing null character */
        *json_struct_to = *json_struct_from;
        json_struct_to++;
        json_struct_i++;
    }
    *json_struct_to = '\0';

    return json_struct_i;
}

#  define JSON_STRUCT_ENUM_START(name)
#  define JSON_STRUCT_ENUM_ELEMENT(name)
#  define JSON_STRUCT_ENUM_END

#  define JSON_STRUCT_ARRAY_BITFIELD_START(name)
#  define JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(name)
#  define JSON_STRUCT_ARRAY_BITFIELD_END

#  define JSON_STRUCT_VOID_P_START(name) if (NULL != json_struct_data->name) { \
                                           switch (json_struct_data->void_p_enum_##name) {
#  define JSON_STRUCT_VOID_P_ELEMENT(name, element) case void_p_enum_##name##_##element: \
                                                      json_struct_copy->name = json_struct_buf; \
                                                      json_struct_buf = json_struct_copy_##element(json_struct_buf, \
                                                                          (struct element*) json_struct_data->name); \
                                                      break;
#  define JSON_STRUCT_VOID_P_END(name) } }

#  define JSON_STRUCT_START(name) void* json_struct_copy_##name(void *json_struct_buf, struct name *json_struct_data) { \
                                    struct name *json_struct_copy = (struct name *)json_struct_buf; \
                                    int json_struct_ret; \
                                    memcpy(json_struct_buf, (void *) json_struct_data, sizeof(struct name)); \
                                    json_struct_buf += sizeof(struct name);
#  define JSON_STRUCT_END return json_struct_buf;}

#  define JSON_STRUCT_STRUCT_ARRAY(type, name, max_length) if (NULL != json_struct_data->name) { \
                                                             json_struct_copy->name = (struct type **)json_struct_buf; \
                                                             json_struct_buf += sizeof(struct type *) \
                                                                                * json_struct_data->size_##name; \
                                                             int json_struct_count_##name; \
                                                             for (json_struct_count_##name = 0; \
                                                                  json_struct_count_##name < json_struct_data->size_##name; \
                                                                  json_struct_count_##name++) { \
                                                               json_struct_copy->name[json_struct_count_##name] = (struct type *)json_struct_buf; \
                                                               json_struct_buf = json_struct_copy_##type(json_struct_buf, \
                                                                                                         *((json_struct_data->name) \
                                                                                                           + json_struct_count_##name)); \
                                                             } \
                                                           }

#  define JSON_STRUCT_STRUCT_P(type, name) if (NULL != json_struct_data->name) { \
                                             json_struct_copy->name = json_struct_buf; \
                                             json_struct_buf = json_struct_copy_##type(json_struct_buf, \
                                                                 json_struct_data->name); \
                                           }
#  define JSON_STRUCT_STRUCT(type, name)
#  define JSON_STRUCT_ARRAY_BITFIELD(type, name)
#  define JSON_STRUCT_ENUM(type, name)
#  define JSON_STRUCT_INT(name)
#  define JSON_STRUCT_PID_T(name)
#  define JSON_STRUCT_CSTRING(name, length)
#  define JSON_STRUCT_CSTRING_P(name, max_length) if (NULL != json_struct_data->name) { \
                                                    json_struct_ret = json_struct_copy_cstring_p((char *)json_struct_buf, \
                                                                        json_struct_data->name, max_length); \
                                                    json_struct_copy->name = (char *)json_struct_buf; \
                                                    json_struct_buf += json_struct_ret; \
                                                  }
#  define JSON_STRUCT_CSTRING_P_CONST(name, max_length) JSON_STRUCT_CSTRING_P(name, max_length)
#  define JSON_STRUCT_CLOCK_T(name)
#  define JSON_STRUCT_FILE_P(name)
#  define JSON_STRUCT_LONG_INT(name)
#  define JSON_STRUCT_SIZE_T(name)
#  define JSON_STRUCT_SSIZE_T(name)
#  define JSON_STRUCT_OFF_T(name)
#  define JSON_STRUCT_U_INT64_T(name)
#  define JSON_STRUCT_VOID_P(name)
#  define JSON_STRUCT_VOID_P_CONST(name)
#  define JSON_STRUCT_FD_SET_P(name) if (NULL != json_struct_data->name) { \
                                       memcpy(json_struct_buf, (void *) json_struct_data->name, sizeof(fd_set)); \
                                       json_struct_copy->name = (fd_set *)json_struct_buf; \
                                       json_struct_buf += sizeof(fd_set); \
                                     }
#  ifdef HAVE_DLMOPEN
#    define JSON_STRUCT_LMID_T(name)
#  endif
#  define JSON_STRUCT_SHORT(name)
#  define JSON_STRUCT_DEV_T(name)
#  define JSON_STRUCT_INO_T(name)
#  define JSON_STRUCT_MALLOC_STRING_ARRAY(name, max_size, max_length_per_element) if (NULL != json_struct_data->name) { \
                                                                                    json_struct_copy->name = (char **) json_struct_buf; \
                                                                                    json_struct_buf += sizeof(char *) * (json_struct_data->size_##name \
                                                                                                                         - json_struct_data->start_##name); \
                                                                                    for (int json_struct_count_##name = json_struct_data->start_##name; \
                                                                                         json_struct_count_##name < json_struct_data->size_##name && \
                                                                                         json_struct_count_##name - json_struct_data->start_##name < max_size; \
                                                                                         json_struct_count_##name++) { \
                                                                                      if (NULL != json_struct_data->name[json_struct_count_##name]) { \
                                                                                        json_struct_copy->name[json_struct_count_##name \
                                                                                                               - json_struct_data->start_##name] = (char *) json_struct_buf; \
                                                                                        json_struct_ret = json_struct_copy_cstring_p((char *)json_struct_buf, \
                                                                                                                                     json_struct_data->name[json_struct_count_##name], \
                                                                                                                                     max_length_per_element); \
                                                                                        json_struct_buf += json_struct_ret; \
                                                                                      } else { \
                                                                                        json_struct_copy->name[json_struct_count_##name] = NULL; \
                                                                                      } \
                                                                                    } \
                                                                                    json_struct_copy->start_##name = 0; \
                                                                                    json_struct_copy->size_##name = json_struct_data->size_##name \
                                                                                                                    - json_struct_data->start_##name; \
                                                                                    if (json_struct_copy->size_##name > max_size) { \
                                                                                      json_struct_copy->size_##name = max_size; \
                                                                                    } \
                                                                                  }
#  define JSON_STRUCT_MALLOC_PTR_ARRAY(name, max_size) if (NULL != json_struct_data->name) { \
                                                         json_struct_copy->name = (void *) json_struct_buf; \
                                                         json_struct_buf += sizeof(void *) * (json_struct_data->size_##name \
                                                                            - json_struct_data->start_##name); \
                                                         for (int json_struct_count_##name = json_struct_data->start_##name; \
                                                              json_struct_count_##name < json_struct_data->size_##name && \
                                                              json_struct_count_##name - json_struct_data->start_##name < max_size; \
                                                              json_struct_count_##name++) { \
                                                           if (NULL != json_struct_data->name[json_struct_count_##name]) { \
                                                             json_struct_copy->name[json_struct_count_##name \
                                                                                    - json_struct_data->start_##name] = (void *) json_struct_buf; \
                                                           } else { \
                                                             json_struct_copy->name[json_struct_count_##name] = NULL; \
                                                           } \
                                                         } \
                                                         json_struct_copy->start_##name = 0; \
                                                         json_struct_copy->size_##name = json_struct_data->size_##name \
                                                                                         - json_struct_data->start_##name; \
                                                         if (json_struct_copy->size_##name > max_size) { \
                                                           json_struct_copy->size_##name = max_size; \
                                                         } \
                                                       }
#  define JSON_STRUCT_INT_ARRAY(name, max_size) if (NULL != json_struct_data->name) { \
                                                  memcpy(json_struct_buf, (void *) json_struct_data->name, \
                                                         sizeof(int) * json_struct_copy->size_##name); \
                                                  json_struct_copy->name = (int *) json_struct_buf; \
                                                  json_struct_buf += sizeof(int) * json_struct_copy->size_##name; \
                                                }
#  define JSON_STRUCT_SA_FAMILY_T(name)
#  define JSON_STRUCT_KEY_VALUE_ARRAY(name, max_size, max_length_per_cstring) if (NULL != json_struct_data->keys_##name) { \
                                                                                json_struct_copy->keys_##name = (char **) json_struct_buf; \
                                                                                json_struct_buf += sizeof(char *) * json_struct_data->size_##name; \
                                                                                json_struct_copy->values_##name = (char **) json_struct_buf; \
                                                                                json_struct_buf += sizeof(char *) * json_struct_data->size_##name; \
                                                                                for (int json_struct_count_##name = 0; \
                                                                                     json_struct_count_##name < json_struct_data->size_##name && \
                                                                                     json_struct_count_##name < max_size; \
                                                                                     json_struct_count_##name++) { \
                                                                                  if (NULL != json_struct_data->keys_##name[json_struct_count_##name]) { \
                                                                                    json_struct_copy->keys_##name[json_struct_count_##name] = (char *) json_struct_buf; \
                                                                                    json_struct_ret = json_struct_copy_cstring_p((char *)json_struct_buf, \
                                                                                                                                 json_struct_data->keys_##name[json_struct_count_##name], \
                                                                                                                                 max_length_per_cstring); \
                                                                                    json_struct_buf += json_struct_ret; \
                                                                                  } else { \
                                                                                    json_struct_copy->keys_##name[json_struct_count_##name] = NULL; \
                                                                                  } \
                                                                                  if (NULL != json_struct_data->values_##name[json_struct_count_##name]) { \
                                                                                    json_struct_copy->values_##name[json_struct_count_##name] = (char *) json_struct_buf; \
                                                                                    json_struct_ret = json_struct_copy_cstring_p((char *)json_struct_buf, \
                                                                                                                                 json_struct_data->values_##name[json_struct_count_##name], \
                                                                                                                                 max_length_per_cstring); \
                                                                                    json_struct_buf += json_struct_ret; \
                                                                                  } else { \
                                                                                    json_struct_copy->values_##name[json_struct_count_##name] = NULL; \
                                                                                  } \
                                                                                } \
                                                                                if (json_struct_copy->size_##name > max_size) { \
                                                                                  json_struct_copy->size_##name = max_size; \
                                                                                } \
                                                                              }
/* insert new line for new data-type here */

/* ----------------------------------------------------------------------------------------------------------------------- */
#elif JSON_STRUCT == JSON_STRUCT_FREE
/* ----------------------------------------------------------------------------------------------------------------------- */
/*
 * Macros for freeing allocated memory
 *
 * following functions are available after include of the macros
 *
 * void json_struct_free_<name of struct>(struct name *json_struct_data)
 *
 * for freeing allocated memory.
 * Functions can be used to free memory after deep copying with functions included
 * with JSON_STRUCT == JSON_STRUCT_COPY.
 * */

#  define JSON_STRUCT_ENUM_START(name)
#  define JSON_STRUCT_ENUM_ELEMENT(name)
#  define JSON_STRUCT_ENUM_END

#  define JSON_STRUCT_ARRAY_BITFIELD_START(name)
#  define JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(name)
#  define JSON_STRUCT_ARRAY_BITFIELD_END

#  define JSON_STRUCT_VOID_P_START(name) if (NULL != json_struct_data->name) { \
                                           switch (json_struct_data->void_p_enum_##name) {
#  define JSON_STRUCT_VOID_P_ELEMENT(name, element) case void_p_enum_##name##_##element: \
                                                      json_struct_free_##element((struct element*) json_struct_data->name); \
                                                      break;
#  define JSON_STRUCT_VOID_P_END(name) } }

#  define JSON_STRUCT_START(name) void json_struct_free_##name(struct name *json_struct_data) {
#  define JSON_STRUCT_END }

#  define JSON_STRUCT_STRUCT_ARRAY(type, name, max_length) if (NULL != json_struct_data->name) { \
                                                             int json_struct_count_##name; \
                                                             for (json_struct_count_##name = 0; \
                                                                  json_struct_count_##name < json_struct_data->size_##name; \
                                                                  json_struct_count_##name++) { \
                                                                 json_struct_free_##type(*((json_struct_data->name) \
                                                                                           + json_struct_count_##name)); \
                                                             } \
                                                           }

#  define JSON_STRUCT_STRUCT_P(type, name) if (NULL != json_struct_data->name) { \
                                             json_struct_free_##type(json_struct_data->name); \
                                           }
#  define JSON_STRUCT_STRUCT(type, name)
#  define JSON_STRUCT_ARRAY_BITFIELD(type, name)
#  define JSON_STRUCT_ENUM(type, name)
#  define JSON_STRUCT_INT(name)
#  define JSON_STRUCT_PID_T(name)
#  define JSON_STRUCT_CSTRING(name, length)
#  define JSON_STRUCT_CSTRING_P(name, max_length)
#  define JSON_STRUCT_CSTRING_P_CONST(name, max_length)
#  define JSON_STRUCT_CLOCK_T(name)
#  define JSON_STRUCT_FILE_P(name)
#  define JSON_STRUCT_LONG_INT(name)
#  define JSON_STRUCT_SIZE_T(name)
#  define JSON_STRUCT_SSIZE_T(name)
#  define JSON_STRUCT_OFF_T(name)
#  define JSON_STRUCT_U_INT64_T(name)
#  define JSON_STRUCT_VOID_P(name)
#  define JSON_STRUCT_VOID_P_CONST(name)
#  define JSON_STRUCT_FD_SET_P(name)
#  ifdef HAVE_DLMOPEN
#    define JSON_STRUCT_LMID_T(name)
#  endif
#  define JSON_STRUCT_SHORT(name)
#  define JSON_STRUCT_DEV_T(name)
#  define JSON_STRUCT_INO_T(name)
#  define JSON_STRUCT_MALLOC_STRING_ARRAY(name, max_size, max_length_per_element) if (NULL != json_struct_data->name) { \
                                                                                    free(json_struct_data->name); \
                                                                                    json_struct_data->name = NULL; \
                                                                                  }
#  define JSON_STRUCT_MALLOC_PTR_ARRAY(name, max_size) if (NULL != json_struct_data->name) { \
                                                         free(json_struct_data->name); \
                                                         json_struct_data->name = NULL; \
                                                       }
#  define JSON_STRUCT_INT_ARRAY(name, max_size)
#  define JSON_STRUCT_SA_FAMILY_T(name)
#  define JSON_STRUCT_KEY_VALUE_ARRAY(name, max_size, max_length_per_cstring)
/* insert new line for new data-type here */

/* ----------------------------------------------------------------------------------------------------------------------- */
#elif JSON_STRUCT == JSON_STRUCT_PUSH_COUNT
/*
 * Macros for evaluating size of HTTP Posts
 *
 * following functions are available after include of the macros
 *
 * size_t json_struct_push_max_size_<name of struct>()
 *
 * */

//char body[] = "some_metric 6.10\nsome_other_metric 7.14\n";   "some_metric"

#  define JSON_STRUCT_ELEMENT_SIZE(name, sizeValue) json_struct_size += prefix_length + sizeof(#name) \
                                                                        - 1  /* nullbyte entfernen von #name */ \
                                                                        + sizeValue \
                                                                        + 2; /* space and \n */
#  define JSON_STRUCT_TYPE_SIZE_DEC(type) ceil(log10(pow(2, sizeof(type) * CHAR_BIT))) /* wie viele ascii character passen in numerischen Typ wie zb uint64_t */

#  define JSON_STRUCT_ENUM_START(name)
#  define JSON_STRUCT_ENUM_ELEMENT(name)
#  define JSON_STRUCT_ENUM_END

#  define JSON_STRUCT_ARRAY_BITFIELD_START(name)
#  define JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(name)
#  define JSON_STRUCT_ARRAY_BITFIELD_END

// Nur fuer eingehaenkte Strukturen ####################################################################################
#  define JSON_STRUCT_VOID_P_START(name) json_struct_size_void_p = 0;
// JSON_STRUCT_VOID_P_ELEMENT ruft fuer jede Struktur die eingehaengt sein kann die Funktion zur Groessenermittlung auf und gibt dabei die Praefixlaenge(=name) mit
// Ziel: Groesste eingehaengte Struktur finden
#  define JSON_STRUCT_VOID_P_ELEMENT(name, element) json_struct_size_void_p_tmp = json_struct_push_max_size_##element(sizeof(#name) + prefix_length); \
                                                    if(json_struct_size_void_p_tmp > json_struct_size_void_p) \
                                                      json_struct_size_void_p = json_struct_size_void_p_tmp;
#  define JSON_STRUCT_VOID_P_END(name) json_struct_size += json_struct_size_void_p;
// ####################################################################################################################

//Jede Struktur beginnt mit diesem Makro
#  define JSON_STRUCT_START(name) int json_struct_push_max_size_##name(size_t prefix_length) { /* prefix zb. file_type -- prefix nur fuer basic 0 */ \
                                    /* char json_struct_hasElements = 0; /* Merken ob in Struktur ueberhaupt was drin ist - nicht benoetigt weil letztes Element auch \n hat */ \
                                    int json_struct_size_void_p; /* Nur fuer eingehaengte Strukuten... Merken welche eingeh. Strkt. am groessten */ \
                                    int json_struct_size_void_p_tmp; \
                                    size_t json_struct_size = 0; /* Start with 0 -- no parentheses like json */
#  define JSON_STRUCT_END return json_struct_size;}

#  define JSON_STRUCT_STRUCT_ARRAY(type, name, max_length)

#  define JSON_STRUCT_STRUCT_P(type, name)
#  define JSON_STRUCT_STRUCT(type, name)
#  define JSON_STRUCT_ARRAY_BITFIELD(type, name)
#  define JSON_STRUCT_ENUM(type, name)
#  define JSON_STRUCT_INT(name)
#  define JSON_STRUCT_PID_T(name)
#  define JSON_STRUCT_CSTRING(name, length)
#  define JSON_STRUCT_CSTRING_P(name, max_length)
#  define JSON_STRUCT_CSTRING_P_CONST(name, max_length)
#  define JSON_STRUCT_CLOCK_T(name)
#  define JSON_STRUCT_FILE_P(name)
#  define JSON_STRUCT_LONG_INT(name)
#  define JSON_STRUCT_SIZE_T(name) JSON_STRUCT_ELEMENT_SIZE(name, JSON_STRUCT_TYPE_SIZE_DEC(size_t))
#  define JSON_STRUCT_SSIZE_T(name)
#  define JSON_STRUCT_OFF_T(name)
#  define JSON_STRUCT_U_INT64_T(name) JSON_STRUCT_ELEMENT_SIZE(name, JSON_STRUCT_TYPE_SIZE_DEC(u_int64_t))
#  define JSON_STRUCT_VOID_P(name)
#  define JSON_STRUCT_VOID_P_CONST(name)
#  define JSON_STRUCT_FD_SET_P(name)
#  ifdef HAVE_DLMOPEN
#    define JSON_STRUCT_LMID_T(name)
#  endif
#  define JSON_STRUCT_SHORT(name)
#  define JSON_STRUCT_DEV_T(name)
#  define JSON_STRUCT_INO_T(name)
#  define JSON_STRUCT_MALLOC_STRING_ARRAY(name, max_size, max_length_per_element)
#  define JSON_STRUCT_MALLOC_PTR_ARRAY(name, max_size)
#  define JSON_STRUCT_INT_ARRAY(name, max_size)
#  define JSON_STRUCT_SA_FAMILY_T(name)
#  define JSON_STRUCT_KEY_VALUE_ARRAY(name, max_size, max_length_per_cstring)

/* insert new line for new data-type here */
/* ----------------------------------------------------------------------------------------------------------------------- */
#elif JSON_STRUCT == JSON_STRUCT_PUSH
/*
 * Macros for generating HTTP Posts
 *
 * following functions are available after include of the macros
 *
 * int json_struct_push_<name of struct>(void* json_struct_buffer_to_post, size_t json_struct_length_of_buffer_to_post, struct name *json_struct_data);
 *
 * */
#  define JSON_STRUCT_ELEMENT(key, template, ...) JSON_STRUCT_SNPRINTF(#key" "#template"\n", __VA_ARGS__)

#  define JSON_STRUCT_SNPRINTF(...) json_struct_ret = snprintf(json_struct_buf, json_struct_size, __VA_ARGS__); \
                                    JSON_STRUCT_SIZE_ERROR(json_struct_ret, json_struct_size) /* don't write more characters then size of buffer */ \
                                    json_struct_buf += json_struct_ret;  /* set pointer to end of written characters */ \
                                    json_struct_size -= json_struct_ret; /* resize buffer size */


#  define JSON_STRUCT_SIZE_ERROR(ret, size) if (ret >= size) { \
                                                CALL_REAL_POSIX_SYNC(fprintf)(stderr, "Output buffer in function %s not big enough.\n", __func__); \
                                                assert(0); \
                                            }


#  define JSON_STRUCT_ENUM_START(name)
#  define JSON_STRUCT_ENUM_ELEMENT(name)
#  define JSON_STRUCT_ENUM_END 

#  define JSON_STRUCT_ARRAY_BITFIELD_START(name)
#  define JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(name)
#  define JSON_STRUCT_ARRAY_BITFIELD_END

#  define JSON_STRUCT_VOID_P_START(name) 
#  define JSON_STRUCT_VOID_P_ELEMENT(name, element)
#  define JSON_STRUCT_VOID_P_END(name)

#  define JSON_STRUCT_START(name) int json_struct_push_##name(char* json_struct_buf, size_t json_struct_size, \
                                        struct name *json_struct_data) { \
                                    int json_struct_ret = 0; \
                                    int json_struct_start_size = json_struct_size;
#  define JSON_STRUCT_END return json_struct_start_size - json_struct_size;}

#  define JSON_STRUCT_STRUCT_ARRAY(type, name, max_length)

#  define JSON_STRUCT_STRUCT_P(type, name)
#  define JSON_STRUCT_STRUCT(type, name)
#  define JSON_STRUCT_ARRAY_BITFIELD(type, name)
#  define JSON_STRUCT_ENUM(type, name)
#  define JSON_STRUCT_INT(name)
#  define JSON_STRUCT_PID_T(name)
#  define JSON_STRUCT_CSTRING(name, length)
#  define JSON_STRUCT_CSTRING_P(name, max_length)
#  define JSON_STRUCT_CSTRING_P_CONST(name, max_length)
#  define JSON_STRUCT_CLOCK_T(name)
#  define JSON_STRUCT_FILE_P(name)
#  define JSON_STRUCT_LONG_INT(name)
#  define JSON_STRUCT_SIZE_T(name)
#  define JSON_STRUCT_SSIZE_T(name)
#  define JSON_STRUCT_OFF_T(name)
#  define JSON_STRUCT_U_INT64_T(name) JSON_STRUCT_ELEMENT(name, %lu, json_struct_data->name)
#  define JSON_STRUCT_VOID_P(name)
#  define JSON_STRUCT_VOID_P_CONST(name)
#  define JSON_STRUCT_FD_SET_P(name)
#  ifdef HAVE_DLMOPEN
#    define JSON_STRUCT_LMID_T(name)
#  endif
#  define JSON_STRUCT_SHORT(name)
#  define JSON_STRUCT_DEV_T(name)
#  define JSON_STRUCT_INO_T(name)
#  define JSON_STRUCT_MALLOC_STRING_ARRAY(name, max_size, max_length_per_element)
#  define JSON_STRUCT_MALLOC_PTR_ARRAY(name, max_size)
#  define JSON_STRUCT_INT_ARRAY(name, max_size)
#  define JSON_STRUCT_SA_FAMILY_T(name)
#  define JSON_STRUCT_KEY_VALUE_ARRAY(name, max_size, max_length_per_cstring)

/* insert new line for new data-type here */
/* ----------------------------------------------------------------------------------------------------------------------- */
#else
/* ----------------------------------------------------------------------------------------------------------------------- */
#  error "Undefined JSON_STRUCT value."
#endif

#endif
