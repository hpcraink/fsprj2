#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <limits.h>
#include <assert.h>
#include <math.h>
#include "posix_io.h"

/*
 * To add a new data-type for generating the struct and the json-cstring six lines
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

/* #defines for error handling */
#ifndef JSON_STRUCT_ERROR
#  define JSON_STRUCT_ERROR
#  define JSON_STRUCT_ENUM_ERROR(value) CALL_REAL(fprintf)(stderr, "Unknown value \"%d\" of enum in function %s.\n", value, __func__); \
                                        /* ToDo: __func__ dependencies (like in posix_io.c) */ \
                                        assert(0);
#  define JSON_STRUCT_SIZE_ERROR(ret, size) if (ret >= size) { \
                                                CALL_REAL(fprintf)(stderr, "Output buffer in function %s not big enough.\n", __func__); \
                                                assert(0); \
                                            }
#endif

/* macros for setting VOID_P elements */
#define JSON_STRUCT_SET_VOID_P(struct_name, element, substruct_type, value) struct_name.void_p_enum_##element = \
                                                                                void_p_enum_##element##_##substruct_type; \
                                                                            struct_name.element = (void*) (&value);
#define JSON_STRUCT_SET_VOID_P_NULL(struct_name, element) struct_name.element = NULL;

/* #define's for escaping cstrings (used in print_cstring()-function) */
#ifndef JSON_STRUCT_ESCAPE_SLASH
#  define JSON_STRUCT_ESCAPE_SLASH 0
#endif

#ifdef JSON_STRUCT

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
/* insert new line for new data-type here */

#if JSON_STRUCT == JSON_STRUCT_DATA_TYPE

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
/* insert new line for new data-type here */

#elif JSON_STRUCT == JSON_STRUCT_PRINT

/*
 * generate functions
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
                                       int json_struct_count_name; \
                                       for (json_struct_count_name = 0; json_struct_count_name < FD_SETSIZE; json_struct_count_name++) { \
                                         if (FD_ISSET(json_struct_count_name, json_struct_data->name)) { \
                                           JSON_STRUCT_SNPRINTF("%d,", json_struct_count_name) \
                                         } \
                                       } \
                                       if (json_struct_count_name > 0) { \
                                         json_struct_buf--;  /* remove last comma */ \
                                         json_struct_size++; /* and resize buffer size */ \
                                       } \
                                       JSON_STRUCT_WRITE("],") \
                                     }
#  ifdef HAVE_DLMOPEN
#    define JSON_STRUCT_LMID_T(name) JSON_STRUCT_ELEMENT(name, %ld, json_struct_data->name)
#  endif
/* insert new line for new data-type here */

#elif JSON_STRUCT == JSON_STRUCT_BYTES_COUNT

/*
 * generate functions
 *
 * int json_struct_max_size_<name of struct>()
 * int json_struct_max_size_array_<name of struct>()
 * int json_struct_max_size_enum_<name of struct>()
 *
 * for evaluating max size of json-string. The returned size is without trailing
 * null character.
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
/* insert new line for new data-type here */

#elif JSON_STRUCT == JSON_STRUCT_SIZEOF

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
/* insert new line for new data-type here */

#elif JSON_STRUCT == JSON_STRUCT_COPY

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
/* insert new line for new data-type here */

#else
#  error "Undefined JSON_STRUCT value."
#endif

#endif
