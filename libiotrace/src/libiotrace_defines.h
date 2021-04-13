#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <limits.h>
#include <math.h>

#include "error.h"

/*
 * To add a new data-type for generating the struct and the json-cstring seven lines
 * have to be added. The positions for adding a new data-type are marked with:
 * insert new line for new data-type here
 */

/* All intern variables are named with prefix "libiotrace_struct_" to prevent shadowing.
 * All functions are also named with prefix "libiotrace_struct_" to prevent conflicts. */

/* values for #define LIBIOTRACE_STRUCT */
#define LIBIOTRACE_STRUCT_DATA_TYPE         1 /* generate struct */
#define LIBIOTRACE_STRUCT_PRINT             2 /* generate print-function to print struct as json */
#define LIBIOTRACE_STRUCT_BYTES_COUNT       3 /* generate function to evaluate max size of json-string */
#define LIBIOTRACE_STRUCT_SIZEOF            4 /* generate function to evaluate size for LIBIOTRACE_STRUCT_COPY */
#define LIBIOTRACE_STRUCT_COPY              5 /* generate function to deep copy struct (with VOID_P elements) */
#define LIBIOTRACE_STRUCT_FREE              6 /* generate function to free malloc'ed memory */
#define LIBIOTRACE_STRUCT_PUSH_BYTES_COUNT  7 /* generate function to evaluate size for HTTP Posts */
#define LIBIOTRACE_STRUCT_PUSH              8 /* generate function to generate POST request*/

/* #defines for error handling */
#ifndef LIBIOTRACE_STRUCT_ERROR
#  define LIBIOTRACE_STRUCT_ERROR
#  define LIBIOTRACE_STRUCT_ENUM_ERROR(value) LIBIOTRACE_ERROR("unknown value \"%d\" of enum", value);
#  define LIBIOTRACE_STRUCT_SIZE_ERROR(ret, size) if (ret >= size) { \
                                                LIBIOTRACE_ERROR("output buffer not big enough"); \
                                            }
#endif

/* macros for setting VOID_P elements */
#define LIBIOTRACE_STRUCT_SET_VOID_P(struct_name, element, substruct_type, value) struct_name.void_p_enum_##element = \
                                                                                void_p_enum_##element##_##substruct_type; \
                                                                            struct_name.element = (void*) (&value);
#define LIBIOTRACE_STRUCT_SET_VOID_P_NULL(struct_name, element) struct_name.element = NULL;

/* macros for setting malloced string array */
#define LIBIOTRACE_STRUCT_SET_MALLOC_STRING_ARRAY(struct_name, array_name, malloced_array, start, size) struct_name.size_##array_name = size; \
                                                                                                  struct_name.start_##array_name = start; \
                                                                                                  struct_name.array_name = malloced_array;
#define LIBIOTRACE_STRUCT_SET_MALLOC_STRING_ARRAY_NULL(struct_name, array_name) struct_name.array_name = NULL;

/* macros for setting malloced ptr array */
#define LIBIOTRACE_STRUCT_SET_MALLOC_PTR_ARRAY(struct_name, array_name, malloced_array, start, size) struct_name.size_##array_name = size; \
                                                                                               struct_name.start_##array_name = start; \
                                                                                               struct_name.array_name = malloced_array;
#define LIBIOTRACE_STRUCT_SET_MALLOC_PTR_ARRAY_NULL(struct_name, array_name) struct_name.array_name = NULL;

/* macros for setting int array */
#define LIBIOTRACE_STRUCT_SET_INT_ARRAY(struct_name, array_name, array, size) struct_name.size_##array_name = size; \
                                                                        struct_name.array_name = array;
#define LIBIOTRACE_STRUCT_SET_INT_ARRAY_NULL(struct_name, array_name) struct_name.array_name = NULL;

/* macros for setting struct array */
#define LIBIOTRACE_STRUCT_SET_STRUCT_ARRAY(struct_name, array_name, array, size) struct_name.size_##array_name = size; \
                                                                           struct_name.array_name = array;
#define LIBIOTRACE_STRUCT_SET_STRUCT_ARRAY_NULL(struct_name, array_name) struct_name.array_name = NULL;

/* macros for setting key value array */
#define LIBIOTRACE_STRUCT_INIT_KEY_VALUE_ARRAY(struct_name, array_name, keys_array, values_array) struct_name.size_##array_name = 0; \
                                                                                            struct_name.keys_##array_name = keys_array; \
                                                                                            struct_name.values_##array_name = values_array;
#define LIBIOTRACE_STRUCT_SET_KEY_VALUE_ARRAY_NULL(struct_name, array_name) struct_name.size_##array_name = 0; \
                                                                      struct_name.keys_##array_name = NULL; \
                                                                      struct_name.values_##array_name = NULL;
#define LIBIOTRACE_STRUCT_ADD_KEY_VALUE(struct_name, array_name, key, value) struct_name.keys_##array_name[struct_name.size_##array_name] = key; \
                                                                       struct_name.values_##array_name[struct_name.size_##array_name] = value; \
                                                                       struct_name.size_##array_name++;

/* #define's for escaping cstrings (used in print_cstring()-function) */
#ifndef LIBIOTRACE_STRUCT_ESCAPE_SLASH
#  define LIBIOTRACE_STRUCT_ESCAPE_SLASH 0
#endif

#define COUNT_DEC_AS_CHAR(type) ceil(log10(pow(2, sizeof(type) * CHAR_BIT)))

#ifdef LIBIOTRACE_STRUCT

#undef LIBIOTRACE_STRUCT_ELEMENT_SIZE
#undef LIBIOTRACE_STRUCT_TYPE_SIZE_DEC

#undef LIBIOTRACE_STRUCT_ELEMENT
#undef LIBIOTRACE_STRUCT_SNPRINTF

#undef LIBIOTRACE_STRUCT_ENUM_START
#undef LIBIOTRACE_STRUCT_ENUM_ELEMENT
#undef LIBIOTRACE_STRUCT_ENUM_END

#undef LIBIOTRACE_STRUCT_ARRAY_BITFIELD_START
#undef LIBIOTRACE_STRUCT_ARRAY_BITFIELD_ELEMENT
#undef LIBIOTRACE_STRUCT_ARRAY_BITFIELD_END

#undef LIBIOTRACE_STRUCT_START
#undef LIBIOTRACE_STRUCT_END

#undef LIBIOTRACE_STRUCT_VOID_P_START
#undef LIBIOTRACE_STRUCT_VOID_P_ELEMENT
#undef LIBIOTRACE_STRUCT_VOID_P_END

#undef LIBIOTRACE_STRUCT_STRUCT_ARRAY

#undef LIBIOTRACE_STRUCT_STRUCT_P
#undef LIBIOTRACE_STRUCT_STRUCT
#undef LIBIOTRACE_STRUCT_ARRAY_BITFIELD
#undef LIBIOTRACE_STRUCT_ENUM
#undef LIBIOTRACE_STRUCT_INT
#undef LIBIOTRACE_STRUCT_CHAR
#undef LIBIOTRACE_STRUCT_PID_T
#undef LIBIOTRACE_STRUCT_CSTRING
#undef LIBIOTRACE_STRUCT_CSTRING_P
#undef LIBIOTRACE_STRUCT_CSTRING_P_CONST
#undef LIBIOTRACE_STRUCT_CLOCK_T
#undef LIBIOTRACE_STRUCT_FILE_P
#undef LIBIOTRACE_STRUCT_ENUM_START
#undef LIBIOTRACE_STRUCT_LONG_INT
#undef LIBIOTRACE_STRUCT_SIZE_T
#undef LIBIOTRACE_STRUCT_SSIZE_T
#undef LIBIOTRACE_STRUCT_OFF_T
#undef LIBIOTRACE_STRUCT_U_INT64_T
#undef LIBIOTRACE_STRUCT_VOID_P
#undef LIBIOTRACE_STRUCT_VOID_P_CONST
#undef LIBIOTRACE_STRUCT_FD_SET_P
#if defined(HAVE_DLMOPEN) && defined(WITH_DL_IO)
#  undef LIBIOTRACE_STRUCT_LMID_T
#endif
#undef LIBIOTRACE_STRUCT_SHORT
#undef LIBIOTRACE_STRUCT_DEV_T
#undef LIBIOTRACE_STRUCT_INO_T
#undef LIBIOTRACE_STRUCT_MALLOC_STRING_ARRAY
#undef LIBIOTRACE_STRUCT_MALLOC_PTR_ARRAY
#undef LIBIOTRACE_STRUCT_INT_ARRAY
#undef LIBIOTRACE_STRUCT_SA_FAMILY_T
#undef LIBIOTRACE_STRUCT_KEY_VALUE_ARRAY
/* insert new line for new data-type here */

/* ----------------------------------------------------------------------------------------------------------------------- */
#if LIBIOTRACE_STRUCT == LIBIOTRACE_STRUCT_DATA_TYPE
/* ----------------------------------------------------------------------------------------------------------------------- */
/*
 * Macros for struct declaration
 * */

#  define LIBIOTRACE_STRUCT_ENUM_START(name) enum name {
#  define LIBIOTRACE_STRUCT_ENUM_ELEMENT(name) name,
#  define LIBIOTRACE_STRUCT_ENUM_END };

#  define LIBIOTRACE_STRUCT_ARRAY_BITFIELD_START(name) struct name {
#  define LIBIOTRACE_STRUCT_ARRAY_BITFIELD_ELEMENT(name) unsigned int name : 1;
#  define LIBIOTRACE_STRUCT_ARRAY_BITFIELD_END };

#  define LIBIOTRACE_STRUCT_VOID_P_START(name) void *name; enum {
#  define LIBIOTRACE_STRUCT_VOID_P_ELEMENT(name, element) void_p_enum_##name##_##element,
#  define LIBIOTRACE_STRUCT_VOID_P_END(name) } void_p_enum_##name;

#  define LIBIOTRACE_STRUCT_START(name) struct name {
#  define LIBIOTRACE_STRUCT_END };

#  define LIBIOTRACE_STRUCT_STRUCT_ARRAY(type, name, max_length) struct type **name; int size_##name;

#  define LIBIOTRACE_STRUCT_STRUCT_P(type, name) struct type *name;
#  define LIBIOTRACE_STRUCT_STRUCT(type, name) struct type name;
#  define LIBIOTRACE_STRUCT_ARRAY_BITFIELD(type, name) struct type name;
#  define LIBIOTRACE_STRUCT_ENUM(type, name) enum type name;
#  define LIBIOTRACE_STRUCT_INT(name) int name;
#  define LIBIOTRACE_STRUCT_CHAR(name) char name;
#  define LIBIOTRACE_STRUCT_PID_T(name) pid_t name;
#  define LIBIOTRACE_STRUCT_CSTRING(name, length) char name[length];
#  define LIBIOTRACE_STRUCT_CSTRING_P(name, max_length) char *name;
#  define LIBIOTRACE_STRUCT_CSTRING_P_CONST(name, max_length) const char *name;
#  define LIBIOTRACE_STRUCT_CLOCK_T(name) clock_t name;
#  define LIBIOTRACE_STRUCT_FILE_P(name) FILE *name;
#  define LIBIOTRACE_STRUCT_LONG_INT(name) long int name;
#  define LIBIOTRACE_STRUCT_SIZE_T(name) size_t name;
#  define LIBIOTRACE_STRUCT_SSIZE_T(name) ssize_t name;
#  if HAVE_OFF64_T
#    define LIBIOTRACE_STRUCT_OFF_T(name) off64_t name;
#  else
#    define LIBIOTRACE_STRUCT_OFF_T(name) off_t name;
#  endif
#  define LIBIOTRACE_STRUCT_U_INT64_T(name) u_int64_t name;
#  define LIBIOTRACE_STRUCT_VOID_P(name) void *name;
#  define LIBIOTRACE_STRUCT_VOID_P_CONST(name) const void *name;
#  define LIBIOTRACE_STRUCT_FD_SET_P(name) fd_set *name;
#  if defined(HAVE_DLMOPEN) && defined(WITH_DL_IO)
#    define LIBIOTRACE_STRUCT_LMID_T(name) Lmid_t name;
#  endif
#  define LIBIOTRACE_STRUCT_SHORT(name) short name;
#  define LIBIOTRACE_STRUCT_DEV_T(name) dev_t name;
#  define LIBIOTRACE_STRUCT_INO_T(name) ino_t name;
#  define LIBIOTRACE_STRUCT_MALLOC_STRING_ARRAY(name, max_size, max_length_per_element) int start_##name; int size_##name; char ** name;
#  define LIBIOTRACE_STRUCT_MALLOC_PTR_ARRAY(name, max_size) int start_##name; int size_##name; void ** name;
#  define LIBIOTRACE_STRUCT_INT_ARRAY(name, max_size) int size_##name; int * name;
#  define LIBIOTRACE_STRUCT_SA_FAMILY_T(name) sa_family_t name;
#  define LIBIOTRACE_STRUCT_KEY_VALUE_ARRAY(name, max_size, max_length_per_cstring) int size_##name; char ** keys_##name; char ** values_##name;
/* insert new line for new data-type here */

/* ----------------------------------------------------------------------------------------------------------------------- */
#elif LIBIOTRACE_STRUCT == LIBIOTRACE_STRUCT_PRINT
/* ----------------------------------------------------------------------------------------------------------------------- */
/*
 * Macros for serializing structs to json
 *
 * following functions are available after include of the macros
 *
 * int libiotrace_struct_print_<name of struct>(char* buf, size_t size, struct name *data)
 * int libiotrace_struct_print_array_<name of array>(char* buf, size_t size, struct name *data)
 * int libiotrace_struct_print_enum_<name of enum>(char* buf, size_t size, struct name *data)
 *
 * for printing struct as json-cstring. The size argument specifies the maximum number
 * of characters to produce.
 * */

#define LIBIOTRACE_STRUCT_ESCAPE_PRINT_2BYTES(character) if (libiotrace_struct_size - libiotrace_struct_ret > 2) { \
                                                       *libiotrace_struct_buf++ = '\\'; \
                                                       *libiotrace_struct_buf++ = character; \
                                                       libiotrace_struct_src++; \
                                                       libiotrace_struct_ret += 2; \
                                                   } else { \
                                                       libiotrace_struct_size = 0; /* buffer not big enough: break while */ \
                                                   }

int libiotrace_struct_print_cstring(char* libiotrace_struct_buf, size_t libiotrace_struct_size, const char* libiotrace_struct_src) {
    const char *libiotrace_struct_hex = "0123456789abcdef";
    int libiotrace_struct_ret = 0;

    if (libiotrace_struct_size > 2) {
        *libiotrace_struct_buf++ = '\"';
        libiotrace_struct_ret++;
    } else {
        return libiotrace_struct_ret;
    }

    if (NULL != libiotrace_struct_src) {
        /* ToDo: comment */
        while (*libiotrace_struct_src != '\0' && libiotrace_struct_size - libiotrace_struct_ret > 2) {
            if ((unsigned char)*libiotrace_struct_src >= ' ') {
                switch (*libiotrace_struct_src) {
                case '\"':
                case '\\':
#if LIBIOTRACE_STRUCT_ESCAPE_SLASH
                case '/':
#endif
                    LIBIOTRACE_STRUCT_ESCAPE_PRINT_2BYTES(*libiotrace_struct_src)
                    break;
                default:
                    *libiotrace_struct_buf++ = *libiotrace_struct_src++;
                    libiotrace_struct_ret++;
                }
            } else {
                switch (*libiotrace_struct_src) {
                case '\b':
                    LIBIOTRACE_STRUCT_ESCAPE_PRINT_2BYTES('b')
                    break;
                case '\f':
                    LIBIOTRACE_STRUCT_ESCAPE_PRINT_2BYTES('f')
                    break;
                case '\n':
                    LIBIOTRACE_STRUCT_ESCAPE_PRINT_2BYTES('n')
                    break;
                case '\r':
                    LIBIOTRACE_STRUCT_ESCAPE_PRINT_2BYTES('r')
                    break;
                case '\t':
                    LIBIOTRACE_STRUCT_ESCAPE_PRINT_2BYTES('t')
                    break;
                default:
                    /* '\a' and '\v' are not part of the JSON escape definition and
                     * have to be encoded as \u,
                     * also all other values < U+0020 (space ' ') have to be encoded
                     * as \u */
                    if (libiotrace_struct_size - libiotrace_struct_ret > 7) {
                        *libiotrace_struct_buf++ = '\\';
                        *libiotrace_struct_buf++ = 'u';
                        *libiotrace_struct_buf++ = '0';
                        *libiotrace_struct_buf++ = '0';
                        *libiotrace_struct_buf++ = libiotrace_struct_hex[((unsigned char)*libiotrace_struct_src >> 4)];
                        *libiotrace_struct_buf++ = libiotrace_struct_hex[(*libiotrace_struct_src) & 0x0f];
                        libiotrace_struct_src++;
                        libiotrace_struct_ret += 6;
                    } else {
                        libiotrace_struct_size = 0; /* buffer not big enough: break while */ \
                    }
                }
            }
        }
    }

    *libiotrace_struct_buf++ = '\"';
    libiotrace_struct_ret++;
    *libiotrace_struct_buf = '\0';
    return libiotrace_struct_ret;
}

int libiotrace_struct_write(char* libiotrace_struct_buf, size_t libiotrace_struct_size, const char* libiotrace_struct_src) {
    int libiotrace_struct_ret = 0;

    while (*libiotrace_struct_src != '\0' && libiotrace_struct_size - libiotrace_struct_ret > 1) {
        *libiotrace_struct_buf++ = *libiotrace_struct_src++;
        libiotrace_struct_ret++;
    }

    *libiotrace_struct_buf = '\0';
    return libiotrace_struct_ret;
}

#  define LIBIOTRACE_STRUCT_SNPRINTF(...) libiotrace_struct_ret = snprintf(libiotrace_struct_buf, libiotrace_struct_size, __VA_ARGS__); \
                                    LIBIOTRACE_STRUCT_SIZE_ERROR(libiotrace_struct_ret, libiotrace_struct_size) /* don't write more characters then size of buffer */ \
                                    libiotrace_struct_buf += libiotrace_struct_ret;  /* set pointer to end of written characters */ \
                                    libiotrace_struct_size -= libiotrace_struct_ret; /* resize buffer size */

#  define LIBIOTRACE_STRUCT_WRITE(value) libiotrace_struct_ret = libiotrace_struct_write(libiotrace_struct_buf, libiotrace_struct_size, value); \
                                   LIBIOTRACE_STRUCT_SIZE_ERROR(libiotrace_struct_ret, libiotrace_struct_size) /* don't write more characters then size of buffer */ \
                                   libiotrace_struct_buf += libiotrace_struct_ret;  /* set pointer to end of written characters */ \
                                   libiotrace_struct_size -= libiotrace_struct_ret; /* resize buffer size */

#  define LIBIOTRACE_STRUCT_QUOT(key) "\""#key"\""

#  define LIBIOTRACE_STRUCT_TYPE(name, function) libiotrace_struct_hasElements = 1; \
                                           LIBIOTRACE_STRUCT_WRITE(LIBIOTRACE_STRUCT_QUOT(name)":") \
                                           libiotrace_struct_ret = function(libiotrace_struct_buf, libiotrace_struct_size, \
                                                               &libiotrace_struct_data->name); \
                                           libiotrace_struct_buf += libiotrace_struct_ret;  /* set pointer to end of written characters */ \
                                           libiotrace_struct_size -= libiotrace_struct_ret; /* resize buffer size */
#  define LIBIOTRACE_STRUCT_ELEMENT(key, template, ...) libiotrace_struct_hasElements = 1; \
                                                  LIBIOTRACE_STRUCT_SNPRINTF(LIBIOTRACE_STRUCT_QUOT(key)":"#template",", __VA_ARGS__)
#  define LIBIOTRACE_STRUCT_ESCAPE(name) libiotrace_struct_hasElements = 1; \
                                   LIBIOTRACE_STRUCT_WRITE(LIBIOTRACE_STRUCT_QUOT(name)":") \
                                   libiotrace_struct_ret = libiotrace_struct_print_cstring(libiotrace_struct_buf, libiotrace_struct_size, \
                                                       libiotrace_struct_data->name); \
                                   LIBIOTRACE_STRUCT_SIZE_ERROR(libiotrace_struct_ret, libiotrace_struct_size) \
                                   libiotrace_struct_buf += libiotrace_struct_ret;  /* set pointer to end of written characters */ \
                                   libiotrace_struct_size -= libiotrace_struct_ret; /* resize buffer size */ \
                                   LIBIOTRACE_STRUCT_WRITE(",")

#  define LIBIOTRACE_STRUCT_ENUM_START(name) int libiotrace_struct_print_enum_##name(char* libiotrace_struct_buf, \
                                             size_t libiotrace_struct_size, enum name *libiotrace_struct_data) { \
                                         int libiotrace_struct_ret = 0; \
                                         int libiotrace_struct_start_size = libiotrace_struct_size; \
                                         switch (*libiotrace_struct_data) {
#  define LIBIOTRACE_STRUCT_ENUM_ELEMENT(name) case name: \
                                           LIBIOTRACE_STRUCT_WRITE(LIBIOTRACE_STRUCT_QUOT(name)",") \
                                           break;
#  define LIBIOTRACE_STRUCT_ENUM_END default: \
                                 LIBIOTRACE_STRUCT_ENUM_ERROR(*libiotrace_struct_data) \
                               } return libiotrace_struct_start_size - libiotrace_struct_size;}

#  define LIBIOTRACE_STRUCT_ARRAY_BITFIELD_START(name) int libiotrace_struct_print_array_##name(char* libiotrace_struct_buf, \
                                                       size_t libiotrace_struct_size, struct name *libiotrace_struct_data) { \
                                                   char libiotrace_struct_hasElements = 0; \
                                                   int libiotrace_struct_ret = 0; \
                                                   int libiotrace_struct_start_size = libiotrace_struct_size; \
                                                   LIBIOTRACE_STRUCT_WRITE("[")
#  define LIBIOTRACE_STRUCT_ARRAY_BITFIELD_ELEMENT(name) if (libiotrace_struct_data->name) { \
                                                     libiotrace_struct_hasElements = 1; \
                                                     LIBIOTRACE_STRUCT_WRITE(LIBIOTRACE_STRUCT_QUOT(name)",") \
                                                   }
#  define LIBIOTRACE_STRUCT_ARRAY_BITFIELD_END if (libiotrace_struct_hasElements) { \
                                           libiotrace_struct_buf--;  /* remove last comma */ \
                                           libiotrace_struct_size++; /* and resize buffer size */ \
                                         } \
                                         LIBIOTRACE_STRUCT_WRITE("],") \
                                         return libiotrace_struct_start_size - libiotrace_struct_size;}

#  define LIBIOTRACE_STRUCT_START(name) int libiotrace_struct_print_##name(char* libiotrace_struct_buf, size_t libiotrace_struct_size, \
                                        struct name *libiotrace_struct_data) { \
                                    char libiotrace_struct_hasElements = 0; \
                                    int libiotrace_struct_ret = 0; \
                                    int libiotrace_struct_start_size = libiotrace_struct_size; \
                                    LIBIOTRACE_STRUCT_WRITE("{")
#  define LIBIOTRACE_STRUCT_END if (libiotrace_struct_hasElements) { \
                            libiotrace_struct_buf--;  /* remove last comma */ \
                            libiotrace_struct_size++; /* and resize buffer size */ \
                          } \
                          LIBIOTRACE_STRUCT_WRITE("}") \
                          return libiotrace_struct_start_size - libiotrace_struct_size;}

#  define LIBIOTRACE_STRUCT_VOID_P_START(name) if (NULL != libiotrace_struct_data->name) { \
                                           libiotrace_struct_hasElements = 1; \
                                           LIBIOTRACE_STRUCT_WRITE(LIBIOTRACE_STRUCT_QUOT(name)":") \
                                           switch (libiotrace_struct_data->void_p_enum_##name) {
#  define LIBIOTRACE_STRUCT_VOID_P_ELEMENT(name, element) case void_p_enum_##name##_##element: \
                                                      libiotrace_struct_ret = libiotrace_struct_print_##element(libiotrace_struct_buf, \
                                                                          libiotrace_struct_size, (struct element*) \
                                                                          libiotrace_struct_data->name); \
                                                      libiotrace_struct_buf += libiotrace_struct_ret;  /* set pointer to end of written characters */ \
                                                      libiotrace_struct_size -= libiotrace_struct_ret; /* resize buffer size */\
                                                      break;
#  define LIBIOTRACE_STRUCT_VOID_P_END(name)   default: \
                                           LIBIOTRACE_STRUCT_ENUM_ERROR(libiotrace_struct_data->void_p_enum_##name) \
                                         } \
                                         LIBIOTRACE_STRUCT_WRITE(",") \
                                       }

#  define LIBIOTRACE_STRUCT_STRUCT_ARRAY(type, name, max_length) if (NULL != libiotrace_struct_data->name) { \
                                                             libiotrace_struct_hasElements = 1; \
                                                             LIBIOTRACE_STRUCT_WRITE(LIBIOTRACE_STRUCT_QUOT(name)":[") \
                                                             int libiotrace_struct_count_##name; \
                                                             for (libiotrace_struct_count_##name = 0; \
                                                                  libiotrace_struct_count_##name < libiotrace_struct_data->size_##name; \
                                                                  libiotrace_struct_count_##name++) { \
                                                               libiotrace_struct_ret = libiotrace_struct_print_##type(libiotrace_struct_buf, \
                                                                                                          libiotrace_struct_size, \
                                                                                                          *((libiotrace_struct_data->name) + libiotrace_struct_count_##name)); \
                                                               libiotrace_struct_buf += libiotrace_struct_ret;  /* set pointer to end of written characters */ \
                                                               libiotrace_struct_size -= libiotrace_struct_ret; /* resize buffer size */ \
                                                               LIBIOTRACE_STRUCT_WRITE(",") \
                                                             } \
                                                             if (libiotrace_struct_count_##name > 0) { \
                                                               libiotrace_struct_buf--;  /* remove last comma */ \
                                                               libiotrace_struct_size++; /* and resize buffer size */ \
                                                             } \
                                                             LIBIOTRACE_STRUCT_WRITE("],") \
                                                           }

#  define LIBIOTRACE_STRUCT_STRUCT_P(type, name) if (NULL != libiotrace_struct_data->name) { \
                                             libiotrace_struct_hasElements = 1; \
                                             LIBIOTRACE_STRUCT_WRITE(LIBIOTRACE_STRUCT_QUOT(name)":") \
                                             libiotrace_struct_ret = libiotrace_struct_print_##type(libiotrace_struct_buf, libiotrace_struct_size, \
                                                                                        libiotrace_struct_data->name); \
                                             libiotrace_struct_buf += libiotrace_struct_ret;  /* set pointer to end of written characters */ \
                                             libiotrace_struct_size -= libiotrace_struct_ret; /* resize buffer size */ \
                                             LIBIOTRACE_STRUCT_WRITE(",") \
                                           }
#  define LIBIOTRACE_STRUCT_STRUCT(type, name) LIBIOTRACE_STRUCT_TYPE(name, libiotrace_struct_print_##type) \
                                         LIBIOTRACE_STRUCT_WRITE(",")
#  define LIBIOTRACE_STRUCT_ARRAY_BITFIELD(type, name) LIBIOTRACE_STRUCT_TYPE(name, libiotrace_struct_print_array_##type)
#  define LIBIOTRACE_STRUCT_ENUM(type, name) LIBIOTRACE_STRUCT_TYPE(name, libiotrace_struct_print_enum_##type)
#  define LIBIOTRACE_STRUCT_INT(name) LIBIOTRACE_STRUCT_ELEMENT(name, %d, libiotrace_struct_data->name)
#  define LIBIOTRACE_STRUCT_CHAR(name) LIBIOTRACE_STRUCT_ELEMENT(name, %d, libiotrace_struct_data->name)
#  define LIBIOTRACE_STRUCT_PID_T(name) LIBIOTRACE_STRUCT_ELEMENT(name, %u, libiotrace_struct_data->name)
#  define LIBIOTRACE_STRUCT_CSTRING(name, length) LIBIOTRACE_STRUCT_ESCAPE(name)
#  define LIBIOTRACE_STRUCT_CSTRING_P(name, max_length) LIBIOTRACE_STRUCT_ESCAPE(name)
#  define LIBIOTRACE_STRUCT_CSTRING_P_CONST(name, max_length) LIBIOTRACE_STRUCT_ESCAPE(name)
#  define LIBIOTRACE_STRUCT_CLOCK_T(name) LIBIOTRACE_STRUCT_ELEMENT(name, %lu, libiotrace_struct_data->name)
#  define LIBIOTRACE_STRUCT_FILE_P(name) LIBIOTRACE_STRUCT_ELEMENT(name, "%p", libiotrace_struct_data->name)
#  define LIBIOTRACE_STRUCT_LONG_INT(name) LIBIOTRACE_STRUCT_ELEMENT(name, %ld, libiotrace_struct_data->name)
#  define LIBIOTRACE_STRUCT_SIZE_T(name) LIBIOTRACE_STRUCT_ELEMENT(name, %ld, libiotrace_struct_data->name)
#  define LIBIOTRACE_STRUCT_SSIZE_T(name) LIBIOTRACE_STRUCT_ELEMENT(name, %ld, libiotrace_struct_data->name)
#  define LIBIOTRACE_STRUCT_OFF_T(name) LIBIOTRACE_STRUCT_ELEMENT(name, %ld, libiotrace_struct_data->name)
#  define LIBIOTRACE_STRUCT_U_INT64_T(name) LIBIOTRACE_STRUCT_ELEMENT(name, %lu, libiotrace_struct_data->name)
#  define LIBIOTRACE_STRUCT_VOID_P(name) LIBIOTRACE_STRUCT_ELEMENT(name, "%p", libiotrace_struct_data->name)
#  define LIBIOTRACE_STRUCT_VOID_P_CONST(name) LIBIOTRACE_STRUCT_ELEMENT(name, "%p", libiotrace_struct_data->name)
#  define LIBIOTRACE_STRUCT_FD_SET_P(name) if (NULL != libiotrace_struct_data->name) { \
                                       libiotrace_struct_hasElements = 1; \
                                       LIBIOTRACE_STRUCT_WRITE(LIBIOTRACE_STRUCT_QUOT(name)":[") \
                                       int libiotrace_struct_count_##name; \
                                       for (libiotrace_struct_count_##name = 0; libiotrace_struct_count_##name < FD_SETSIZE; libiotrace_struct_count_##name++) { \
                                         if (FD_ISSET(libiotrace_struct_count_##name, libiotrace_struct_data->name)) { \
                                           LIBIOTRACE_STRUCT_SNPRINTF("%d,", libiotrace_struct_count_##name) \
                                         } \
                                       } \
                                       if (libiotrace_struct_count_##name > 0) { \
                                         libiotrace_struct_buf--;  /* remove last comma */ \
                                         libiotrace_struct_size++; /* and resize buffer size */ \
                                       } \
                                       LIBIOTRACE_STRUCT_WRITE("],") \
                                     }
#  if defined(HAVE_DLMOPEN) && defined(WITH_DL_IO)
#    define LIBIOTRACE_STRUCT_LMID_T(name) LIBIOTRACE_STRUCT_ELEMENT(name, %ld, libiotrace_struct_data->name)
#  endif
#  define LIBIOTRACE_STRUCT_SHORT(name) LIBIOTRACE_STRUCT_ELEMENT(name, %d, libiotrace_struct_data->name)
#  define LIBIOTRACE_STRUCT_DEV_T(name) LIBIOTRACE_STRUCT_ELEMENT(name, %lu, libiotrace_struct_data->name)
#  define LIBIOTRACE_STRUCT_INO_T(name) LIBIOTRACE_STRUCT_ELEMENT(name, %lu, libiotrace_struct_data->name)
#  define LIBIOTRACE_STRUCT_MALLOC_STRING_ARRAY(name, max_size, max_length_per_element) if (NULL != libiotrace_struct_data->name) { \
                                                                                    libiotrace_struct_hasElements = 1; \
                                                                                    LIBIOTRACE_STRUCT_WRITE(LIBIOTRACE_STRUCT_QUOT(name)":[") \
                                                                                    int libiotrace_struct_count_##name; \
                                                                                    for (libiotrace_struct_count_##name = libiotrace_struct_data->start_##name; \
                                                                                         libiotrace_struct_count_##name < libiotrace_struct_data->size_##name; \
                                                                                         libiotrace_struct_count_##name++) { \
                                                                                      libiotrace_struct_ret = libiotrace_struct_print_cstring(libiotrace_struct_buf, libiotrace_struct_size, \
                                                                                                                                  libiotrace_struct_data->name[libiotrace_struct_count_##name]); \
                                                                                      LIBIOTRACE_STRUCT_SIZE_ERROR(libiotrace_struct_ret, libiotrace_struct_size) \
                                                                                      libiotrace_struct_buf += libiotrace_struct_ret;  /* set pointer to end of written characters */ \
                                                                                      libiotrace_struct_size -= libiotrace_struct_ret; /* resize buffer size */ \
                                                                                      LIBIOTRACE_STRUCT_WRITE(",") \
                                                                                    } \
                                                                                    if (libiotrace_struct_count_##name > 0) { \
                                                                                      libiotrace_struct_buf--;  /* remove last comma */ \
                                                                                      libiotrace_struct_size++; /* and resize buffer size */ \
                                                                                    } \
                                                                                    LIBIOTRACE_STRUCT_WRITE("],") \
                                                                                  }
#  define LIBIOTRACE_STRUCT_MALLOC_PTR_ARRAY(name, max_size) if (NULL != libiotrace_struct_data->name) { \
                                                         libiotrace_struct_hasElements = 1; \
                                                         LIBIOTRACE_STRUCT_WRITE(LIBIOTRACE_STRUCT_QUOT(name)":[") \
                                                         int libiotrace_struct_count_##name; \
                                                         for (libiotrace_struct_count_##name = libiotrace_struct_data->start_##name; \
                                                              libiotrace_struct_count_##name < libiotrace_struct_data->size_##name; \
                                                              libiotrace_struct_count_##name++) { \
                                                           LIBIOTRACE_STRUCT_SNPRINTF("\"%p\",", libiotrace_struct_data->name[libiotrace_struct_count_##name]) \
                                                         } \
                                                         if (libiotrace_struct_count_##name > 0) { \
                                                           libiotrace_struct_buf--;  /* remove last comma */ \
                                                           libiotrace_struct_size++; /* and resize buffer size */ \
                                                         } \
                                                         LIBIOTRACE_STRUCT_WRITE("],") \
                                                       }
#  define LIBIOTRACE_STRUCT_INT_ARRAY(name, max_size) if (NULL != libiotrace_struct_data->name) { \
                                                  libiotrace_struct_hasElements = 1; \
                                                  LIBIOTRACE_STRUCT_WRITE(LIBIOTRACE_STRUCT_QUOT(name)":[") \
                                                  int libiotrace_struct_count_##name; \
                                                  for (libiotrace_struct_count_##name = 0; \
                                                       libiotrace_struct_count_##name < libiotrace_struct_data->size_##name; \
                                                       libiotrace_struct_count_##name++) { \
                                                    LIBIOTRACE_STRUCT_SNPRINTF("%d,", libiotrace_struct_data->name[libiotrace_struct_count_##name]) \
                                                  } \
                                                  if (libiotrace_struct_count_##name > 0) { \
                                                    libiotrace_struct_buf--;  /* remove last comma */ \
                                                    libiotrace_struct_size++; /* and resize buffer size */ \
                                                  } \
                                                  LIBIOTRACE_STRUCT_WRITE("],") \
                                                }
#  define LIBIOTRACE_STRUCT_SA_FAMILY_T(name) LIBIOTRACE_STRUCT_ELEMENT(name, %hu, libiotrace_struct_data->name)
#  define LIBIOTRACE_STRUCT_KEY_VALUE_ARRAY(name, max_size, max_length_per_cstring) if (NULL != libiotrace_struct_data->keys_##name) { \
                                                                                libiotrace_struct_hasElements = 1; \
                                                                                LIBIOTRACE_STRUCT_WRITE(LIBIOTRACE_STRUCT_QUOT(name)":{") \
                                                                                int libiotrace_struct_count_##name; \
                                                                                for (libiotrace_struct_count_##name = 0; \
                                                                                     libiotrace_struct_count_##name < libiotrace_struct_data->size_##name; \
                                                                                     libiotrace_struct_count_##name++) { \
                                                                                  libiotrace_struct_ret = libiotrace_struct_print_cstring(libiotrace_struct_buf, libiotrace_struct_size, \
                                                                                                      libiotrace_struct_data->keys_##name[libiotrace_struct_count_##name]); \
                                                                                  LIBIOTRACE_STRUCT_SIZE_ERROR(libiotrace_struct_ret, libiotrace_struct_size) \
                                                                                  libiotrace_struct_buf += libiotrace_struct_ret;  /* set pointer to end of written characters */ \
                                                                                  libiotrace_struct_size -= libiotrace_struct_ret; /* resize buffer size */ \
                                                                                  LIBIOTRACE_STRUCT_WRITE(":") \
                                                                                  libiotrace_struct_ret = libiotrace_struct_print_cstring(libiotrace_struct_buf, libiotrace_struct_size, \
                                                                                                      libiotrace_struct_data->values_##name[libiotrace_struct_count_##name]); \
                                                                                  LIBIOTRACE_STRUCT_SIZE_ERROR(libiotrace_struct_ret, libiotrace_struct_size) \
                                                                                  libiotrace_struct_buf += libiotrace_struct_ret;  /* set pointer to end of written characters */ \
                                                                                  libiotrace_struct_size -= libiotrace_struct_ret; /* resize buffer size */ \
                                                                                  LIBIOTRACE_STRUCT_WRITE(",") \
                                                                                } \
                                                                                if (libiotrace_struct_count_##name > 0) { \
                                                                                  libiotrace_struct_buf--;  /* remove last comma */ \
                                                                                  libiotrace_struct_size++; /* and resize buffer size */ \
                                                                                } \
                                                                                LIBIOTRACE_STRUCT_WRITE("},") \
                                                                              }
/* insert new line for new data-type here */

/* ----------------------------------------------------------------------------------------------------------------------- */
#elif LIBIOTRACE_STRUCT == LIBIOTRACE_STRUCT_BYTES_COUNT
/* ----------------------------------------------------------------------------------------------------------------------- */
/*
 * Macros for evaluating the maximum length of a serialized struct
 *
 * following functions are available after include of the macros
 *
 * int libiotrace_struct_max_size_<name of struct>()
 * int libiotrace_struct_max_size_array_<name of struct>()
 * int libiotrace_struct_max_size_enum_<name of struct>()
 *
 * for evaluating max size of json-string. The returned size is without trailing
 * null character.
 * These functions can be used to create a big enough buffer for functions included
 * with LIBIOTRACE_STRUCT == LIBIOTRACE_STRUCT_PRINT.
 * */

/* compiler replaces following line with constants */
#  define LIBIOTRACE_STRUCT_TYPE_SIZE_DEC(type) COUNT_DEC_AS_CHAR(type)
#  define LIBIOTRACE_STRUCT_TYPE_SIZE_HEX(type) ((sizeof(type) * 2) + 2) /* +2 for "0x"-prefix */
#  define LIBIOTRACE_STRUCT_ELEMENT_SIZE(name, sizeValue) libiotrace_struct_hasElements = 1; \
                                                    libiotrace_struct_size += sizeof(#name) \
                                                                        - 1  /* trailing null character */ \
                                                                        + sizeValue \
                                                                        + 4; /* quotation marks (for key), colon and comma */

#  define LIBIOTRACE_STRUCT_ENUM_START(name) int libiotrace_struct_max_size_enum_##name() { \
                                         size_t libiotrace_struct_size_value = 0;
#  define LIBIOTRACE_STRUCT_ENUM_ELEMENT(name) if (sizeof(#name) > libiotrace_struct_size_value) \
                                           libiotrace_struct_size_value = sizeof(#name); /* get greatest possible value */
#  define LIBIOTRACE_STRUCT_ENUM_END return libiotrace_struct_size_value \
                                      - 1   /* trailing null character */ \
                                      + 2;} /* quotation marks (for value) */

#  define LIBIOTRACE_STRUCT_ARRAY_BITFIELD_START(name) int libiotrace_struct_max_size_array_##name() { \
                                                   char libiotrace_struct_hasElements = 0; \
                                                   size_t libiotrace_struct_size = 1; /* open parentheses */
#  define LIBIOTRACE_STRUCT_ARRAY_BITFIELD_ELEMENT(name) libiotrace_struct_hasElements = 1; \
                                                   libiotrace_struct_size += sizeof(#name) /* add each possible value */ \
                                                                       - 1  /* trailing null character */ \
                                                                       + 3; /* quotation marks and comma */
#  define LIBIOTRACE_STRUCT_ARRAY_BITFIELD_END if (libiotrace_struct_hasElements) libiotrace_struct_size--; /* remove last comma */ \
                                         return libiotrace_struct_size + 1;} /* close parentheses */

#  define LIBIOTRACE_STRUCT_START(name) int libiotrace_struct_max_size_##name() { \
                                    char libiotrace_struct_hasElements = 0; \
                                    int libiotrace_struct_size_void_p; \
                                    int libiotrace_struct_size_void_p_tmp; \
                                    size_t libiotrace_struct_size = 1; /* open parentheses */
#  define LIBIOTRACE_STRUCT_END if (libiotrace_struct_hasElements) libiotrace_struct_size--; /* remove last comma */ \
                          libiotrace_struct_size++; /* close parentheses */ \
                          return libiotrace_struct_size;}

#  define LIBIOTRACE_STRUCT_VOID_P_START(name) libiotrace_struct_hasElements = 1; \
                                         libiotrace_struct_size += sizeof(#name) \
                                                             - 1  /* trailing null character */ \
                                                             + 4; /* quotation marks (for key), colon and comma */ \
                                         libiotrace_struct_size_void_p = 0;
#  define LIBIOTRACE_STRUCT_VOID_P_ELEMENT(name, element) libiotrace_struct_size_void_p_tmp = libiotrace_struct_max_size_##element(); \
                                                    if(libiotrace_struct_size_void_p_tmp > libiotrace_struct_size_void_p) \
                                                      libiotrace_struct_size_void_p = libiotrace_struct_size_void_p_tmp;
#  define LIBIOTRACE_STRUCT_VOID_P_END(name) libiotrace_struct_size += libiotrace_struct_size_void_p;

#  define LIBIOTRACE_STRUCT_STRUCT_ARRAY(type, name, max_length) LIBIOTRACE_STRUCT_ELEMENT_SIZE(name, (libiotrace_struct_max_size_##type() \
                                                                                           + 1) /* +1 for comma */ \
                                                                                          * max_length \
                                                                                          - 1   /* for last comma */ \
                                                                                          + 2)  /* for brackets [] */

#  define LIBIOTRACE_STRUCT_STRUCT_P(type, name) LIBIOTRACE_STRUCT_ELEMENT_SIZE(name, libiotrace_struct_max_size_##type())
#  define LIBIOTRACE_STRUCT_STRUCT(type, name) LIBIOTRACE_STRUCT_ELEMENT_SIZE(name, libiotrace_struct_max_size_##type())
#  define LIBIOTRACE_STRUCT_ARRAY_BITFIELD(type, name) LIBIOTRACE_STRUCT_ELEMENT_SIZE(name, libiotrace_struct_max_size_array_##type())
#  define LIBIOTRACE_STRUCT_ENUM(type, name) LIBIOTRACE_STRUCT_ELEMENT_SIZE(name, libiotrace_struct_max_size_enum_##type())
#  define LIBIOTRACE_STRUCT_INT(name) LIBIOTRACE_STRUCT_ELEMENT_SIZE(name, LIBIOTRACE_STRUCT_TYPE_SIZE_DEC(int) \
                                                               + 1) /* for sign (-) */
#  define LIBIOTRACE_STRUCT_CHAR(name) LIBIOTRACE_STRUCT_ELEMENT_SIZE(name, LIBIOTRACE_STRUCT_TYPE_SIZE_DEC(int) \
                                                                + 1) /* for sign (-) */
#  define LIBIOTRACE_STRUCT_PID_T(name) LIBIOTRACE_STRUCT_ELEMENT_SIZE(name, LIBIOTRACE_STRUCT_TYPE_SIZE_DEC(pid_t))
#  define LIBIOTRACE_STRUCT_CSTRING(name,length) LIBIOTRACE_STRUCT_ELEMENT_SIZE(name, (length - 1) /* -1 trailing null character */ \
                                                                          * 6  /* *6 for escaping (\u00ff) */ \
                                                                          + 2) /* +2 quotation marks (for value) */
#  define LIBIOTRACE_STRUCT_CSTRING_P(name, max_length) LIBIOTRACE_STRUCT_CSTRING(name, max_length)
#  define LIBIOTRACE_STRUCT_CSTRING_P_CONST(name, max_length) LIBIOTRACE_STRUCT_CSTRING(name, max_length)
#  define LIBIOTRACE_STRUCT_CLOCK_T(name) LIBIOTRACE_STRUCT_ELEMENT_SIZE(name, LIBIOTRACE_STRUCT_TYPE_SIZE_DEC(clock_t))
#  define LIBIOTRACE_STRUCT_FILE_P(name) LIBIOTRACE_STRUCT_ELEMENT_SIZE(name, LIBIOTRACE_STRUCT_TYPE_SIZE_HEX(FILE*) \
                                                                  + 2) /* quotation marks (for value) */
#  define LIBIOTRACE_STRUCT_LONG_INT(name) LIBIOTRACE_STRUCT_ELEMENT_SIZE(name, LIBIOTRACE_STRUCT_TYPE_SIZE_DEC(long int) \
                                                                    + 1) /* for sign (-) */
#  define LIBIOTRACE_STRUCT_SIZE_T(name) LIBIOTRACE_STRUCT_ELEMENT_SIZE(name, LIBIOTRACE_STRUCT_TYPE_SIZE_DEC(size_t) \
                                                                  + 1) /* for sign (-) */
#  define LIBIOTRACE_STRUCT_SSIZE_T(name) LIBIOTRACE_STRUCT_ELEMENT_SIZE(name, LIBIOTRACE_STRUCT_TYPE_SIZE_DEC(ssize_t) \
                                                                   + 1) /* for sign (-) */
#  if HAVE_OFF64_T
#    define LIBIOTRACE_STRUCT_OFF_T(name) LIBIOTRACE_STRUCT_ELEMENT_SIZE(name, LIBIOTRACE_STRUCT_TYPE_SIZE_DEC(off64_t) \
                                                                   + 1) /* for sign (-) */
#  else
#    define LIBIOTRACE_STRUCT_OFF_T(name) LIBIOTRACE_STRUCT_ELEMENT_SIZE(name, LIBIOTRACE_STRUCT_TYPE_SIZE_DEC(off_t) \
                                                                   + 1) /* for sign (-) */
#  endif
#  define LIBIOTRACE_STRUCT_U_INT64_T(name) LIBIOTRACE_STRUCT_ELEMENT_SIZE(name, LIBIOTRACE_STRUCT_TYPE_SIZE_DEC(u_int64_t))
#  define LIBIOTRACE_STRUCT_VOID_P(name) LIBIOTRACE_STRUCT_ELEMENT_SIZE(name, LIBIOTRACE_STRUCT_TYPE_SIZE_HEX(void*) \
                                                                  + 2) /* quotation marks (for value) */
#  define LIBIOTRACE_STRUCT_VOID_P_CONST(name) LIBIOTRACE_STRUCT_ELEMENT_SIZE(name, LIBIOTRACE_STRUCT_TYPE_SIZE_HEX(void*) \
                                                                        + 2) /* quotation marks (for value) */
#  define LIBIOTRACE_STRUCT_FD_SET_P(name) LIBIOTRACE_STRUCT_ELEMENT_SIZE(name, ((ceil(log10(FD_SETSIZE)) \
                                                                      + 1) /* for comma */ \
                                                                     * FD_SETSIZE) \
                                                                    - 1  /* for last comma */ \
                                                                    + 2) /* for brackets [] */
#  if defined(HAVE_DLMOPEN) && defined(WITH_DL_IO)
#    define LIBIOTRACE_STRUCT_LMID_T(name) LIBIOTRACE_STRUCT_ELEMENT_SIZE(name, LIBIOTRACE_STRUCT_TYPE_SIZE_DEC(Lmid_t) \
                                                                    + 1) /* for sign (-) */
#  endif
#  define LIBIOTRACE_STRUCT_SHORT(name) LIBIOTRACE_STRUCT_ELEMENT_SIZE(name, LIBIOTRACE_STRUCT_TYPE_SIZE_DEC(short) \
                                                                 + 1) /* for sign (-) */
#  define LIBIOTRACE_STRUCT_DEV_T(name) LIBIOTRACE_STRUCT_ELEMENT_SIZE(name, LIBIOTRACE_STRUCT_TYPE_SIZE_DEC(dev_t))
#  define LIBIOTRACE_STRUCT_INO_T(name) LIBIOTRACE_STRUCT_ELEMENT_SIZE(name, LIBIOTRACE_STRUCT_TYPE_SIZE_DEC(dev_t))
#  define LIBIOTRACE_STRUCT_MALLOC_STRING_ARRAY(name, max_size, max_length_per_element) LIBIOTRACE_STRUCT_ELEMENT_SIZE(name, ((max_length_per_element - 1) /* -1 trailing null character */ \
                                                                                                                  * 6  /* *6 for escaping (\u00ff) */ \
                                                                                                                  + 2  /* +2 quotation marks (for value) */ \
                                                                                                                  + 1) /* +1 for comma */ \
                                                                                                                 * max_size \
                                                                                                                 - 1   /* for last comma */ \
                                                                                                                 + 2)  /* for brackets [] */
#  define LIBIOTRACE_STRUCT_MALLOC_PTR_ARRAY(name, max_size) LIBIOTRACE_STRUCT_ELEMENT_SIZE(name, (LIBIOTRACE_STRUCT_TYPE_SIZE_HEX(void*) \
                                                                                       + 2  /* +2 quotation marks (for value) */ \
                                                                                       + 1) /* +1 for comma */ \
                                                                                      * max_size \
                                                                                      - 1   /* for last comma */ \
                                                                                      + 2)  /* for brackets [] */
#  define LIBIOTRACE_STRUCT_INT_ARRAY(name, max_size) LIBIOTRACE_STRUCT_ELEMENT_SIZE(name, (LIBIOTRACE_STRUCT_TYPE_SIZE_DEC(int) \
                                                                                + 1) /* +1 for comma */ \
                                                                               * max_size \
                                                                               - 1   /* for last comma */ \
                                                                               + 2)  /* for brackets [] */
#  define LIBIOTRACE_STRUCT_SA_FAMILY_T(name) LIBIOTRACE_STRUCT_ELEMENT_SIZE(name, LIBIOTRACE_STRUCT_TYPE_SIZE_DEC(sa_family_t))
#  define LIBIOTRACE_STRUCT_KEY_VALUE_ARRAY(name, max_size, max_length_per_cstring) LIBIOTRACE_STRUCT_ELEMENT_SIZE(name, (((max_length_per_cstring - 1) /* -1 trailing null character */ \
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
#elif LIBIOTRACE_STRUCT == LIBIOTRACE_STRUCT_SIZEOF
/* ----------------------------------------------------------------------------------------------------------------------- */
/*
 * Macros for evaluating the size needed for a deep copy of a struct
 *
 * following functions are available after include of the macros
 *
 * int libiotrace_struct_sizeof_<name of struct>(struct name *libiotrace_struct_data)
 *
 * for evaluating max size of struct.
 * These functions can be used to evaluate needed buffer size for deep
 * copy functions included with LIBIOTRACE_STRUCT == LIBIOTRACE_STRUCT_COPY.
 * */

#  define LIBIOTRACE_STRUCT_ENUM_START(name)
#  define LIBIOTRACE_STRUCT_ENUM_ELEMENT(name)
#  define LIBIOTRACE_STRUCT_ENUM_END

#  define LIBIOTRACE_STRUCT_ARRAY_BITFIELD_START(name)
#  define LIBIOTRACE_STRUCT_ARRAY_BITFIELD_ELEMENT(name)
#  define LIBIOTRACE_STRUCT_ARRAY_BITFIELD_END

#  define LIBIOTRACE_STRUCT_VOID_P_START(name) if (NULL != libiotrace_struct_data->name) { \
                                           switch (libiotrace_struct_data->void_p_enum_##name) {
#  define LIBIOTRACE_STRUCT_VOID_P_ELEMENT(name, element) case void_p_enum_##name##_##element: \
                                                      libiotrace_struct_size += libiotrace_struct_sizeof_##element( \
                                                                            (struct element*) libiotrace_struct_data->name); \
                                                      break;
#  define LIBIOTRACE_STRUCT_VOID_P_END(name) } }

#  define LIBIOTRACE_STRUCT_START(name) int libiotrace_struct_sizeof_##name(struct name *libiotrace_struct_data) { \
                                    size_t libiotrace_struct_size = sizeof(struct name);
#  define LIBIOTRACE_STRUCT_END return libiotrace_struct_size;}

#  define LIBIOTRACE_STRUCT_STRUCT_ARRAY(type, name, max_length) if (NULL != libiotrace_struct_data->name) { \
                                                             libiotrace_struct_size += sizeof(struct type *) \
                                                                                 * libiotrace_struct_data->size_##name; \
                                                             int libiotrace_struct_count_##name; \
                                                             for (libiotrace_struct_count_##name = 0; \
                                                                  libiotrace_struct_count_##name < libiotrace_struct_data->size_##name; \
                                                                  libiotrace_struct_count_##name++) { \
                                                               libiotrace_struct_size += libiotrace_struct_sizeof_##type( \
                                                                                     *((libiotrace_struct_data->name) + libiotrace_struct_count_##name)); \
                                                             } \
                                                           }

#  define LIBIOTRACE_STRUCT_STRUCT_P(type, name) if (NULL != libiotrace_struct_data->name) { \
                                             libiotrace_struct_size += libiotrace_struct_sizeof_##type( \
                                                                   libiotrace_struct_data->name); \
                                           }
#  define LIBIOTRACE_STRUCT_STRUCT(type, name)
#  define LIBIOTRACE_STRUCT_ARRAY_BITFIELD(type, name)
#  define LIBIOTRACE_STRUCT_ENUM(type, name)
#  define LIBIOTRACE_STRUCT_INT(name)
#  define LIBIOTRACE_STRUCT_CHAR(name)
#  define LIBIOTRACE_STRUCT_PID_T(name)
#  define LIBIOTRACE_STRUCT_CSTRING(name, length)
#  define LIBIOTRACE_STRUCT_CSTRING_P(name, max_length) if (NULL != libiotrace_struct_data->name) { \
                                                    libiotrace_struct_size += strnlen(libiotrace_struct_data->name, max_length) \
                                                                        + 1; /* +1 for trailing null character */ \
                                                  }
#  define LIBIOTRACE_STRUCT_CSTRING_P_CONST(name, max_length) LIBIOTRACE_STRUCT_CSTRING_P(name, max_length)
#  define LIBIOTRACE_STRUCT_CLOCK_T(name)
#  define LIBIOTRACE_STRUCT_FILE_P(name)
#  define LIBIOTRACE_STRUCT_LONG_INT(name)
#  define LIBIOTRACE_STRUCT_SIZE_T(name)
#  define LIBIOTRACE_STRUCT_SSIZE_T(name)
#  define LIBIOTRACE_STRUCT_OFF_T(name)
#  define LIBIOTRACE_STRUCT_U_INT64_T(name)
#  define LIBIOTRACE_STRUCT_VOID_P(name)
#  define LIBIOTRACE_STRUCT_VOID_P_CONST(name)
#  define LIBIOTRACE_STRUCT_FD_SET_P(name) if (NULL != libiotrace_struct_data->name) { \
                                       libiotrace_struct_size += sizeof(fd_set); \
                                     }
#  if defined(HAVE_DLMOPEN) && defined(WITH_DL_IO)
#    define LIBIOTRACE_STRUCT_LMID_T(name)
#  endif
#  define LIBIOTRACE_STRUCT_SHORT(name)
#  define LIBIOTRACE_STRUCT_DEV_T(name)
#  define LIBIOTRACE_STRUCT_INO_T(name)
#  define LIBIOTRACE_STRUCT_MALLOC_STRING_ARRAY(name, max_size, max_length_per_element) if (NULL != libiotrace_struct_data->name) { \
                                                                                    libiotrace_struct_size += sizeof(char *) * (libiotrace_struct_data->size_##name \
                                                                                                        - libiotrace_struct_data->start_##name); \
                                                                                    for (int libiotrace_struct_count_##name = libiotrace_struct_data->start_##name; \
                                                                                         libiotrace_struct_count_##name < libiotrace_struct_data->size_##name && \
                                                                                         libiotrace_struct_count_##name < max_size; \
                                                                                         libiotrace_struct_count_##name++) { \
                                                                                      libiotrace_struct_size += strnlen(libiotrace_struct_data->name[libiotrace_struct_count_##name], \
                                                                                                                  max_length_per_element) \
                                                                                                          + 1; /* +1 for trailing null character */ \
                                                                                    } \
                                                                                  }
#  define LIBIOTRACE_STRUCT_MALLOC_PTR_ARRAY(name, max_size) if (NULL != libiotrace_struct_data->name) { \
                                                         libiotrace_struct_size += sizeof(void *) * (libiotrace_struct_data->size_##name \
                                                                             - libiotrace_struct_data->start_##name); \
                                                       }
#  define LIBIOTRACE_STRUCT_INT_ARRAY(name, max_size) if (NULL != libiotrace_struct_data->name) { \
                                                  libiotrace_struct_size += sizeof(int) * libiotrace_struct_data->size_##name; \
                                                }
#  define LIBIOTRACE_STRUCT_SA_FAMILY_T(name)
#  define LIBIOTRACE_STRUCT_KEY_VALUE_ARRAY(name, max_size, max_length_per_cstring) if (NULL != libiotrace_struct_data->keys_##name) { \
                                                                                libiotrace_struct_size += sizeof(char *) * libiotrace_struct_data->size_##name \
                                                                                                    * 2; /* key and value arrays */ \
                                                                                for (int libiotrace_struct_count_##name = 0; \
                                                                                     libiotrace_struct_count_##name < libiotrace_struct_data->size_##name && \
                                                                                     libiotrace_struct_count_##name < max_size; \
                                                                                     libiotrace_struct_count_##name++) { \
                                                                                  libiotrace_struct_size += strnlen(libiotrace_struct_data->keys_##name[libiotrace_struct_count_##name], \
                                                                                                              max_length_per_cstring) \
                                                                                                      + 1; /* +1 for trailing null character */ \
                                                                                  libiotrace_struct_size += strnlen(libiotrace_struct_data->values_##name[libiotrace_struct_count_##name], \
                                                                                                              max_length_per_cstring) \
                                                                                                      + 1; /* +1 for trailing null character */ \
                                                                                } \
                                                                              }
/* insert new line for new data-type here */

/* ----------------------------------------------------------------------------------------------------------------------- */
#elif LIBIOTRACE_STRUCT == LIBIOTRACE_STRUCT_COPY
/* ----------------------------------------------------------------------------------------------------------------------- */
/*
 * Macros for deep copy a struct
 *
 * following functions are available after include of the macros
 *
 * void* libiotrace_struct_copy_<name of struct>(void *libiotrace_struct_buf, struct name *libiotrace_struct_data)
 *
 * for deep copy a struct.
 * Functions can be used to write collected data into a buffer.
 * */

int libiotrace_struct_copy_cstring_p(char *libiotrace_struct_to, const char *libiotrace_struct_from, size_t libiotrace_struct_max_length) {
    int libiotrace_struct_i;
    for (libiotrace_struct_i = 1; libiotrace_struct_i < libiotrace_struct_max_length && *libiotrace_struct_from != '\0'; libiotrace_struct_i++) {
        *libiotrace_struct_to = *libiotrace_struct_from;
        libiotrace_struct_to++;
        libiotrace_struct_from++;
    }
    if (libiotrace_struct_i == libiotrace_struct_max_length && *libiotrace_struct_from != '\0') {
        /* it's ok to write on (libiotrace_struct_to + libiotrace_struct_max_length), because expected size in
         * libiotrace_struct_sizeof_##name(struct name *libiotrace_struct_data) is +1 for trailing null character */
        *libiotrace_struct_to = *libiotrace_struct_from;
        libiotrace_struct_to++;
        libiotrace_struct_i++;
    }
    *libiotrace_struct_to = '\0';

    return libiotrace_struct_i;
}

#  define LIBIOTRACE_STRUCT_ENUM_START(name)
#  define LIBIOTRACE_STRUCT_ENUM_ELEMENT(name)
#  define LIBIOTRACE_STRUCT_ENUM_END

#  define LIBIOTRACE_STRUCT_ARRAY_BITFIELD_START(name)
#  define LIBIOTRACE_STRUCT_ARRAY_BITFIELD_ELEMENT(name)
#  define LIBIOTRACE_STRUCT_ARRAY_BITFIELD_END

#  define LIBIOTRACE_STRUCT_VOID_P_START(name) if (NULL != libiotrace_struct_data->name) { \
                                           switch (libiotrace_struct_data->void_p_enum_##name) {
#  define LIBIOTRACE_STRUCT_VOID_P_ELEMENT(name, element) case void_p_enum_##name##_##element: \
                                                      libiotrace_struct_copy->name = libiotrace_struct_buf; \
                                                      libiotrace_struct_buf = libiotrace_struct_copy_##element(libiotrace_struct_buf, \
                                                                          (struct element*) libiotrace_struct_data->name); \
                                                      break;
#  define LIBIOTRACE_STRUCT_VOID_P_END(name) } }

#  define LIBIOTRACE_STRUCT_START(name) void* libiotrace_struct_copy_##name(void *libiotrace_struct_buf, struct name *libiotrace_struct_data) { \
                                    struct name *libiotrace_struct_copy = (struct name *)libiotrace_struct_buf; \
                                    int libiotrace_struct_ret; \
                                    memcpy(libiotrace_struct_buf, (void *) libiotrace_struct_data, sizeof(struct name)); \
                                    libiotrace_struct_buf += sizeof(struct name);
#  define LIBIOTRACE_STRUCT_END return libiotrace_struct_buf;}

#  define LIBIOTRACE_STRUCT_STRUCT_ARRAY(type, name, max_length) if (NULL != libiotrace_struct_data->name) { \
                                                             libiotrace_struct_copy->name = (struct type **)libiotrace_struct_buf; \
                                                             libiotrace_struct_buf += sizeof(struct type *) \
                                                                                * libiotrace_struct_data->size_##name; \
                                                             int libiotrace_struct_count_##name; \
                                                             for (libiotrace_struct_count_##name = 0; \
                                                                  libiotrace_struct_count_##name < libiotrace_struct_data->size_##name; \
                                                                  libiotrace_struct_count_##name++) { \
                                                               libiotrace_struct_copy->name[libiotrace_struct_count_##name] = (struct type *)libiotrace_struct_buf; \
                                                               libiotrace_struct_buf = libiotrace_struct_copy_##type(libiotrace_struct_buf, \
                                                                                                         *((libiotrace_struct_data->name) \
                                                                                                           + libiotrace_struct_count_##name)); \
                                                             } \
                                                           }

#  define LIBIOTRACE_STRUCT_STRUCT_P(type, name) if (NULL != libiotrace_struct_data->name) { \
                                             libiotrace_struct_copy->name = libiotrace_struct_buf; \
                                             libiotrace_struct_buf = libiotrace_struct_copy_##type(libiotrace_struct_buf, \
                                                                 libiotrace_struct_data->name); \
                                           }
#  define LIBIOTRACE_STRUCT_STRUCT(type, name)
#  define LIBIOTRACE_STRUCT_ARRAY_BITFIELD(type, name)
#  define LIBIOTRACE_STRUCT_ENUM(type, name)
#  define LIBIOTRACE_STRUCT_INT(name)
#  define LIBIOTRACE_STRUCT_CHAR(name)
#  define LIBIOTRACE_STRUCT_PID_T(name)
#  define LIBIOTRACE_STRUCT_CSTRING(name, length)
#  define LIBIOTRACE_STRUCT_CSTRING_P(name, max_length) if (NULL != libiotrace_struct_data->name) { \
                                                    libiotrace_struct_ret = libiotrace_struct_copy_cstring_p((char *)libiotrace_struct_buf, \
                                                                        libiotrace_struct_data->name, max_length); \
                                                    libiotrace_struct_copy->name = (char *)libiotrace_struct_buf; \
                                                    libiotrace_struct_buf += libiotrace_struct_ret; \
                                                  }
#  define LIBIOTRACE_STRUCT_CSTRING_P_CONST(name, max_length) LIBIOTRACE_STRUCT_CSTRING_P(name, max_length)
#  define LIBIOTRACE_STRUCT_CLOCK_T(name)
#  define LIBIOTRACE_STRUCT_FILE_P(name)
#  define LIBIOTRACE_STRUCT_LONG_INT(name)
#  define LIBIOTRACE_STRUCT_SIZE_T(name)
#  define LIBIOTRACE_STRUCT_SSIZE_T(name)
#  define LIBIOTRACE_STRUCT_OFF_T(name)
#  define LIBIOTRACE_STRUCT_U_INT64_T(name)
#  define LIBIOTRACE_STRUCT_VOID_P(name)
#  define LIBIOTRACE_STRUCT_VOID_P_CONST(name)
#  define LIBIOTRACE_STRUCT_FD_SET_P(name) if (NULL != libiotrace_struct_data->name) { \
                                       memcpy(libiotrace_struct_buf, (void *) libiotrace_struct_data->name, sizeof(fd_set)); \
                                       libiotrace_struct_copy->name = (fd_set *)libiotrace_struct_buf; \
                                       libiotrace_struct_buf += sizeof(fd_set); \
                                     }
#  if defined(HAVE_DLMOPEN) && defined(WITH_DL_IO)
#    define LIBIOTRACE_STRUCT_LMID_T(name)
#  endif
#  define LIBIOTRACE_STRUCT_SHORT(name)
#  define LIBIOTRACE_STRUCT_DEV_T(name)
#  define LIBIOTRACE_STRUCT_INO_T(name)
#  define LIBIOTRACE_STRUCT_MALLOC_STRING_ARRAY(name, max_size, max_length_per_element) if (NULL != libiotrace_struct_data->name) { \
                                                                                    libiotrace_struct_copy->name = (char **) libiotrace_struct_buf; \
                                                                                    libiotrace_struct_buf += sizeof(char *) * (libiotrace_struct_data->size_##name \
                                                                                                                         - libiotrace_struct_data->start_##name); \
                                                                                    for (int libiotrace_struct_count_##name = libiotrace_struct_data->start_##name; \
                                                                                         libiotrace_struct_count_##name < libiotrace_struct_data->size_##name && \
                                                                                         libiotrace_struct_count_##name - libiotrace_struct_data->start_##name < max_size; \
                                                                                         libiotrace_struct_count_##name++) { \
                                                                                      if (NULL != libiotrace_struct_data->name[libiotrace_struct_count_##name]) { \
                                                                                        libiotrace_struct_copy->name[libiotrace_struct_count_##name \
                                                                                                               - libiotrace_struct_data->start_##name] = (char *) libiotrace_struct_buf; \
                                                                                        libiotrace_struct_ret = libiotrace_struct_copy_cstring_p((char *)libiotrace_struct_buf, \
                                                                                                                                     libiotrace_struct_data->name[libiotrace_struct_count_##name], \
                                                                                                                                     max_length_per_element); \
                                                                                        libiotrace_struct_buf += libiotrace_struct_ret; \
                                                                                      } else { \
                                                                                        libiotrace_struct_copy->name[libiotrace_struct_count_##name] = NULL; \
                                                                                      } \
                                                                                    } \
                                                                                    libiotrace_struct_copy->start_##name = 0; \
                                                                                    libiotrace_struct_copy->size_##name = libiotrace_struct_data->size_##name \
                                                                                                                    - libiotrace_struct_data->start_##name; \
                                                                                    if (libiotrace_struct_copy->size_##name > max_size) { \
                                                                                      libiotrace_struct_copy->size_##name = max_size; \
                                                                                    } \
                                                                                  }
#  define LIBIOTRACE_STRUCT_MALLOC_PTR_ARRAY(name, max_size) if (NULL != libiotrace_struct_data->name) { \
                                                         libiotrace_struct_copy->name = (void *) libiotrace_struct_buf; \
                                                         libiotrace_struct_buf += sizeof(void *) * (libiotrace_struct_data->size_##name \
                                                                            - libiotrace_struct_data->start_##name); \
                                                         for (int libiotrace_struct_count_##name = libiotrace_struct_data->start_##name; \
                                                              libiotrace_struct_count_##name < libiotrace_struct_data->size_##name && \
                                                              libiotrace_struct_count_##name - libiotrace_struct_data->start_##name < max_size; \
                                                              libiotrace_struct_count_##name++) { \
                                                           if (NULL != libiotrace_struct_data->name[libiotrace_struct_count_##name]) { \
                                                             libiotrace_struct_copy->name[libiotrace_struct_count_##name \
                                                                                    - libiotrace_struct_data->start_##name] = (void *) libiotrace_struct_buf; \
                                                           } else { \
                                                             libiotrace_struct_copy->name[libiotrace_struct_count_##name] = NULL; \
                                                           } \
                                                         } \
                                                         libiotrace_struct_copy->start_##name = 0; \
                                                         libiotrace_struct_copy->size_##name = libiotrace_struct_data->size_##name \
                                                                                         - libiotrace_struct_data->start_##name; \
                                                         if (libiotrace_struct_copy->size_##name > max_size) { \
                                                           libiotrace_struct_copy->size_##name = max_size; \
                                                         } \
                                                       }
#  define LIBIOTRACE_STRUCT_INT_ARRAY(name, max_size) if (NULL != libiotrace_struct_data->name) { \
                                                  memcpy(libiotrace_struct_buf, (void *) libiotrace_struct_data->name, \
                                                         sizeof(int) * libiotrace_struct_copy->size_##name); \
                                                  libiotrace_struct_copy->name = (int *) libiotrace_struct_buf; \
                                                  libiotrace_struct_buf += sizeof(int) * libiotrace_struct_copy->size_##name; \
                                                }
#  define LIBIOTRACE_STRUCT_SA_FAMILY_T(name)
#  define LIBIOTRACE_STRUCT_KEY_VALUE_ARRAY(name, max_size, max_length_per_cstring) if (NULL != libiotrace_struct_data->keys_##name) { \
                                                                                libiotrace_struct_copy->keys_##name = (char **) libiotrace_struct_buf; \
                                                                                libiotrace_struct_buf += sizeof(char *) * libiotrace_struct_data->size_##name; \
                                                                                libiotrace_struct_copy->values_##name = (char **) libiotrace_struct_buf; \
                                                                                libiotrace_struct_buf += sizeof(char *) * libiotrace_struct_data->size_##name; \
                                                                                for (int libiotrace_struct_count_##name = 0; \
                                                                                     libiotrace_struct_count_##name < libiotrace_struct_data->size_##name && \
                                                                                     libiotrace_struct_count_##name < max_size; \
                                                                                     libiotrace_struct_count_##name++) { \
                                                                                  if (NULL != libiotrace_struct_data->keys_##name[libiotrace_struct_count_##name]) { \
                                                                                    libiotrace_struct_copy->keys_##name[libiotrace_struct_count_##name] = (char *) libiotrace_struct_buf; \
                                                                                    libiotrace_struct_ret = libiotrace_struct_copy_cstring_p((char *)libiotrace_struct_buf, \
                                                                                                                                 libiotrace_struct_data->keys_##name[libiotrace_struct_count_##name], \
                                                                                                                                 max_length_per_cstring); \
                                                                                    libiotrace_struct_buf += libiotrace_struct_ret; \
                                                                                  } else { \
                                                                                    libiotrace_struct_copy->keys_##name[libiotrace_struct_count_##name] = NULL; \
                                                                                  } \
                                                                                  if (NULL != libiotrace_struct_data->values_##name[libiotrace_struct_count_##name]) { \
                                                                                    libiotrace_struct_copy->values_##name[libiotrace_struct_count_##name] = (char *) libiotrace_struct_buf; \
                                                                                    libiotrace_struct_ret = libiotrace_struct_copy_cstring_p((char *)libiotrace_struct_buf, \
                                                                                                                                 libiotrace_struct_data->values_##name[libiotrace_struct_count_##name], \
                                                                                                                                 max_length_per_cstring); \
                                                                                    libiotrace_struct_buf += libiotrace_struct_ret; \
                                                                                  } else { \
                                                                                    libiotrace_struct_copy->values_##name[libiotrace_struct_count_##name] = NULL; \
                                                                                  } \
                                                                                } \
                                                                                if (libiotrace_struct_copy->size_##name > max_size) { \
                                                                                  libiotrace_struct_copy->size_##name = max_size; \
                                                                                } \
                                                                              }
/* insert new line for new data-type here */

/* ----------------------------------------------------------------------------------------------------------------------- */
#elif LIBIOTRACE_STRUCT == LIBIOTRACE_STRUCT_FREE
/* ----------------------------------------------------------------------------------------------------------------------- */
/*
 * Macros for freeing allocated memory
 *
 * following functions are available after include of the macros
 *
 * void libiotrace_struct_free_<name of struct>(struct name *libiotrace_struct_data)
 *
 * for freeing allocated memory.
 * Functions can be used to free memory after deep copying with functions included
 * with LIBIOTRACE_STRUCT == LIBIOTRACE_STRUCT_COPY.
 * */

#  define LIBIOTRACE_STRUCT_ENUM_START(name)
#  define LIBIOTRACE_STRUCT_ENUM_ELEMENT(name)
#  define LIBIOTRACE_STRUCT_ENUM_END

#  define LIBIOTRACE_STRUCT_ARRAY_BITFIELD_START(name)
#  define LIBIOTRACE_STRUCT_ARRAY_BITFIELD_ELEMENT(name)
#  define LIBIOTRACE_STRUCT_ARRAY_BITFIELD_END

#  define LIBIOTRACE_STRUCT_VOID_P_START(name) if (NULL != libiotrace_struct_data->name) { \
                                           switch (libiotrace_struct_data->void_p_enum_##name) {
#  define LIBIOTRACE_STRUCT_VOID_P_ELEMENT(name, element) case void_p_enum_##name##_##element: \
                                                      libiotrace_struct_free_##element((struct element*) libiotrace_struct_data->name); \
                                                      break;
#  define LIBIOTRACE_STRUCT_VOID_P_END(name) } }

#  define LIBIOTRACE_STRUCT_START(name) void libiotrace_struct_free_##name(struct name *libiotrace_struct_data) {
#  define LIBIOTRACE_STRUCT_END }

#  define LIBIOTRACE_STRUCT_STRUCT_ARRAY(type, name, max_length) if (NULL != libiotrace_struct_data->name) { \
                                                             int libiotrace_struct_count_##name; \
                                                             for (libiotrace_struct_count_##name = 0; \
                                                                  libiotrace_struct_count_##name < libiotrace_struct_data->size_##name; \
                                                                  libiotrace_struct_count_##name++) { \
                                                                 libiotrace_struct_free_##type(*((libiotrace_struct_data->name) \
                                                                                           + libiotrace_struct_count_##name)); \
                                                             } \
                                                           }

#  define LIBIOTRACE_STRUCT_STRUCT_P(type, name) if (NULL != libiotrace_struct_data->name) { \
                                             libiotrace_struct_free_##type(libiotrace_struct_data->name); \
                                           }
#  define LIBIOTRACE_STRUCT_STRUCT(type, name)
#  define LIBIOTRACE_STRUCT_ARRAY_BITFIELD(type, name)
#  define LIBIOTRACE_STRUCT_ENUM(type, name)
#  define LIBIOTRACE_STRUCT_INT(name)
#  define LIBIOTRACE_STRUCT_CHAR(name)
#  define LIBIOTRACE_STRUCT_PID_T(name)
#  define LIBIOTRACE_STRUCT_CSTRING(name, length)
#  define LIBIOTRACE_STRUCT_CSTRING_P(name, max_length)
#  define LIBIOTRACE_STRUCT_CSTRING_P_CONST(name, max_length)
#  define LIBIOTRACE_STRUCT_CLOCK_T(name)
#  define LIBIOTRACE_STRUCT_FILE_P(name)
#  define LIBIOTRACE_STRUCT_LONG_INT(name)
#  define LIBIOTRACE_STRUCT_SIZE_T(name)
#  define LIBIOTRACE_STRUCT_SSIZE_T(name)
#  define LIBIOTRACE_STRUCT_OFF_T(name)
#  define LIBIOTRACE_STRUCT_U_INT64_T(name)
#  define LIBIOTRACE_STRUCT_VOID_P(name)
#  define LIBIOTRACE_STRUCT_VOID_P_CONST(name)
#  define LIBIOTRACE_STRUCT_FD_SET_P(name)
#  if defined(HAVE_DLMOPEN) && defined(WITH_DL_IO)
#    define LIBIOTRACE_STRUCT_LMID_T(name)
#  endif
#  define LIBIOTRACE_STRUCT_SHORT(name)
#  define LIBIOTRACE_STRUCT_DEV_T(name)
#  define LIBIOTRACE_STRUCT_INO_T(name)
#  define LIBIOTRACE_STRUCT_MALLOC_STRING_ARRAY(name, max_size, max_length_per_element) if (NULL != libiotrace_struct_data->name) { \
                                                                                    CALL_REAL_ALLOC_SYNC(free)(libiotrace_struct_data->name); \
                                                                                    libiotrace_struct_data->name = NULL; \
                                                                                  }
#  define LIBIOTRACE_STRUCT_MALLOC_PTR_ARRAY(name, max_size) if (NULL != libiotrace_struct_data->name) { \
                                                         CALL_REAL_ALLOC_SYNC(free)(libiotrace_struct_data->name); \
                                                         libiotrace_struct_data->name = NULL; \
                                                       }
#  define LIBIOTRACE_STRUCT_INT_ARRAY(name, max_size)
#  define LIBIOTRACE_STRUCT_SA_FAMILY_T(name)
#  define LIBIOTRACE_STRUCT_KEY_VALUE_ARRAY(name, max_size, max_length_per_cstring)
/* insert new line for new data-type here */

/* ----------------------------------------------------------------------------------------------------------------------- */
#elif LIBIOTRACE_STRUCT == LIBIOTRACE_STRUCT_PUSH_BYTES_COUNT
/*
 * Macros for evaluating size of HTTP Posts
 *
 * following functions are available after include of the macros
 *
 * size_t libiotrace_struct_push_max_size_<name of struct>()
 *
 * */


#  define LIBIOTRACE_STRUCT_ELEMENT_SIZE(name, sizeValue) libiotrace_struct_size += prefix_length + sizeof(#name) \
                                                                        - 1  /* nullbyte entfernen von #name */ \
                                                                        + sizeValue \
                                                                        + 2; /* equal sign  and comma */
#  define LIBIOTRACE_STRUCT_TYPE_SIZE_DEC(type) COUNT_DEC_AS_CHAR(type) /* wie viele ascii character passen in numerischen Typ wie zb uint64_t */

#  define LIBIOTRACE_STRUCT_ENUM_START(name)
#  define LIBIOTRACE_STRUCT_ENUM_ELEMENT(name)
#  define LIBIOTRACE_STRUCT_ENUM_END

#  define LIBIOTRACE_STRUCT_ARRAY_BITFIELD_START(name)
#  define LIBIOTRACE_STRUCT_ARRAY_BITFIELD_ELEMENT(name)
#  define LIBIOTRACE_STRUCT_ARRAY_BITFIELD_END

// Nur fuer eingehaenkte Strukturen ####################################################################################
#  define LIBIOTRACE_STRUCT_VOID_P_START(name) libiotrace_struct_size_void_p = 0;
// LIBIOTRACE_STRUCT_VOID_P_ELEMENT ruft fuer jede Struktur die eingehaengt sein kann die Funktion zur Groessenermittlung auf und gibt dabei die Praefixlaenge(=name) mit
// Ziel: Groesste eingehaengte Struktur finden
#  define LIBIOTRACE_STRUCT_VOID_P_ELEMENT(name, element) libiotrace_struct_size_void_p_tmp = libiotrace_struct_push_max_size_##element(sizeof(#name) + prefix_length + 1 /*underscore*/); \
                                                    if(libiotrace_struct_size_void_p_tmp > libiotrace_struct_size_void_p) \
                                                      libiotrace_struct_size_void_p = libiotrace_struct_size_void_p_tmp;
#  define LIBIOTRACE_STRUCT_VOID_P_END(name) libiotrace_struct_size += libiotrace_struct_size_void_p;
// ####################################################################################################################

//Jede Struktur beginnt mit diesem Makro
#  define LIBIOTRACE_STRUCT_START(name) int libiotrace_struct_push_max_size_##name(size_t prefix_length) { /* prefix zb. file_type -- prefix nur fuer basic 0 */ \
                                    /* char libiotrace_struct_hasElements = 0; */ /* Merken ob in Struktur ueberhaupt was drin ist - nicht benoetigt weil letztes Element auch \n hat */ \
                                    int libiotrace_struct_size_void_p; /* Nur fuer eingehaengte Strukuten... Merken welche eingeh. Strkt. am groessten */ \
                                    int libiotrace_struct_size_void_p_tmp; \
                                    size_t libiotrace_struct_size = 0; /* Start with 0 -- no parentheses like json */
#  define LIBIOTRACE_STRUCT_END return libiotrace_struct_size;}

#  define LIBIOTRACE_STRUCT_STRUCT_ARRAY(type, name, max_length)

#  define LIBIOTRACE_STRUCT_STRUCT_P(type, name) libiotrace_struct_size += libiotrace_struct_push_max_size_##type(sizeof(#name) + prefix_length + 1 /*underscore*/);
#  define LIBIOTRACE_STRUCT_STRUCT(type, name) libiotrace_struct_size += libiotrace_struct_push_max_size_##type(sizeof(#name) + prefix_length + 1 /*underscore*/);
#  define LIBIOTRACE_STRUCT_ARRAY_BITFIELD(type, name)
#  define LIBIOTRACE_STRUCT_ENUM(type, name)
#  define LIBIOTRACE_STRUCT_INT(name) LIBIOTRACE_STRUCT_ELEMENT_SIZE(name, LIBIOTRACE_STRUCT_TYPE_SIZE_DEC(int))
#  define LIBIOTRACE_STRUCT_CHAR(name) LIBIOTRACE_STRUCT_ELEMENT_SIZE(name, LIBIOTRACE_STRUCT_TYPE_SIZE_DEC(char))
#  define LIBIOTRACE_STRUCT_PID_T(name) LIBIOTRACE_STRUCT_ELEMENT_SIZE(name, LIBIOTRACE_STRUCT_TYPE_SIZE_DEC(pid_t))
#  define LIBIOTRACE_STRUCT_CSTRING(name, length) LIBIOTRACE_STRUCT_ELEMENT_SIZE(name, length + 2) /* +2 for "" in Strings --> Influx 2.X */
#  define LIBIOTRACE_STRUCT_CSTRING_P(name, max_length) LIBIOTRACE_STRUCT_CSTRING(name, max_length)
#  define LIBIOTRACE_STRUCT_CSTRING_P_CONST(name, max_length)
#  define LIBIOTRACE_STRUCT_CLOCK_T(name)
#  define LIBIOTRACE_STRUCT_FILE_P(name)
#  define LIBIOTRACE_STRUCT_LONG_INT(name)
#  define LIBIOTRACE_STRUCT_SIZE_T(name) LIBIOTRACE_STRUCT_ELEMENT_SIZE(name, LIBIOTRACE_STRUCT_TYPE_SIZE_DEC(size_t))
#  define LIBIOTRACE_STRUCT_SSIZE_T(name)
#  define LIBIOTRACE_STRUCT_OFF_T(name)
#  define LIBIOTRACE_STRUCT_U_INT64_T(name) LIBIOTRACE_STRUCT_ELEMENT_SIZE(name, LIBIOTRACE_STRUCT_TYPE_SIZE_DEC(u_int64_t))
#  define LIBIOTRACE_STRUCT_VOID_P(name)
#  define LIBIOTRACE_STRUCT_VOID_P_CONST(name)
#  define LIBIOTRACE_STRUCT_FD_SET_P(name)
#  if defined(HAVE_DLMOPEN) && defined(WITH_DL_IO)
#    define LIBIOTRACE_STRUCT_LMID_T(name)
#  endif
#  define LIBIOTRACE_STRUCT_SHORT(name)

#  define LIBIOTRACE_STRUCT_DEV_T(name) LIBIOTRACE_STRUCT_ELEMENT_SIZE(name, LIBIOTRACE_STRUCT_TYPE_SIZE_DEC(dev_t))
#  define LIBIOTRACE_STRUCT_INO_T(name) LIBIOTRACE_STRUCT_ELEMENT_SIZE(name, LIBIOTRACE_STRUCT_TYPE_SIZE_DEC(ino_t))


#  define LIBIOTRACE_STRUCT_MALLOC_STRING_ARRAY(name, max_size, max_length_per_element)
#  define LIBIOTRACE_STRUCT_MALLOC_PTR_ARRAY(name, max_size)
#  define LIBIOTRACE_STRUCT_INT_ARRAY(name, max_size)
#  define LIBIOTRACE_STRUCT_SA_FAMILY_T(name)
#  define LIBIOTRACE_STRUCT_KEY_VALUE_ARRAY(name, max_size, max_length_per_cstring)

/* insert new line for new data-type here */
/* ----------------------------------------------------------------------------------------------------------------------- */
#elif LIBIOTRACE_STRUCT == LIBIOTRACE_STRUCT_PUSH
/*
 * Macros for generating HTTP Posts
 *
 * following functions are available after include of the macros
 *
 * int libiotrace_struct_push_<name of struct>(void* libiotrace_struct_buffer_to_post, size_t libiotrace_struct_length_of_buffer_to_post, struct name *libiotrace_struct_data);
 *
 * */
#  define LIBIOTRACE_STRUCT_ELEMENT(key, template, ...) if(*prefix=='\0') { \
                                                      LIBIOTRACE_STRUCT_SNPRINTF(#key"="#template",", __VA_ARGS__) \
                                                  } else { \
                                                      LIBIOTRACE_STRUCT_SNPRINTF("%s_"#key"="#template",", prefix, __VA_ARGS__)\
                                                  }

#  define LIBIOTRACE_STRUCT_SNPRINTF(...) libiotrace_struct_ret = snprintf(libiotrace_struct_buf, libiotrace_struct_size, __VA_ARGS__); \
                                    LIBIOTRACE_STRUCT_SIZE_ERROR(libiotrace_struct_ret, libiotrace_struct_size) /* don't write more characters then size of buffer */ \
                                    libiotrace_struct_buf += libiotrace_struct_ret;  /* set pointer to end of written characters */ \
                                    libiotrace_struct_size -= libiotrace_struct_ret; /* resize buffer size */


#  define LIBIOTRACE_STRUCT_SIZE_ERROR(ret, size) if (ret >= size) { \
                                                LIBIOTRACE_ERROR("output buffer not big enough"); \
                                            }


#  define LIBIOTRACE_STRUCT_ENUM_START(name)
#  define LIBIOTRACE_STRUCT_ENUM_ELEMENT(name)
#  define LIBIOTRACE_STRUCT_ENUM_END

#  define LIBIOTRACE_STRUCT_ARRAY_BITFIELD_START(name)
#  define LIBIOTRACE_STRUCT_ARRAY_BITFIELD_ELEMENT(name)
#  define LIBIOTRACE_STRUCT_ARRAY_BITFIELD_END

/*LIBIOTRACE_STRUCT_VOID_P_START(name) generiert prefix*/
#  define LIBIOTRACE_STRUCT_VOID_P_START(name) if (NULL != libiotrace_struct_data->name) { /* Ist z.B file_type eingehaengt? */ \
                                            char prefix_new [strlen(prefix) + sizeof(#name) +1]; /*+1 weil _ als Trennzeichen*/\
                                            if(*prefix == '\0') { \
                                              snprintf(prefix_new, sizeof(prefix_new),"%s", #name);\
                                            } else { \
                                              snprintf(prefix_new, sizeof(prefix_new),"%s_%s", prefix, #name);\
                                            }\
                                            switch (libiotrace_struct_data->void_p_enum_##name) { /* Welche Struktur ist eingehaengt? */
#  define LIBIOTRACE_STRUCT_VOID_P_ELEMENT(name, element) case void_p_enum_##name##_##element: /* Der entsprechende Case wird ausgewaehlt */ \
                                                      libiotrace_struct_ret = libiotrace_struct_push_##element(libiotrace_struct_buf, \
                                                                          libiotrace_struct_size, \
                                                                          (struct element*) libiotrace_struct_data->name, prefix_new); \
                                                      libiotrace_struct_buf += libiotrace_struct_ret;  /* set pointer to end of written characters */ \
                                                      libiotrace_struct_size -= libiotrace_struct_ret; /* resize buffer size */\
                                                      break;
#  define LIBIOTRACE_STRUCT_VOID_P_END(name)   default: \
                                           LIBIOTRACE_STRUCT_ENUM_ERROR(libiotrace_struct_data->void_p_enum_##name) \
                                         } \
                                       }

#  define LIBIOTRACE_STRUCT_START(name) int libiotrace_struct_push_##name(char* libiotrace_struct_buf, size_t libiotrace_struct_size, \
                                        struct name *libiotrace_struct_data, const char* prefix) { \
                                    int libiotrace_struct_ret = 0; \
                                    int libiotrace_struct_start_size = libiotrace_struct_size;
#  define LIBIOTRACE_STRUCT_END return libiotrace_struct_start_size - libiotrace_struct_size;}

#  define LIBIOTRACE_STRUCT_STRUCT_ARRAY(type, name, max_length)

#  define LIBIOTRACE_STRUCT_STRUCT_P(type, name) if (NULL != libiotrace_struct_data->name) { \
                                           char prefix_new_##name [strlen(prefix) + sizeof(#name) +1]; /*+1 for "_" */ \
                                           if(*prefix == '\0') { \
                                             snprintf(prefix_new_##name, sizeof(prefix_new_##name),"%s", #name); \
                                           } else { \
                                             snprintf(prefix_new_##name, sizeof(prefix_new_##name),"%s_%s", prefix, #name); \
                                           } \
                                           libiotrace_struct_ret = libiotrace_struct_push_##type(libiotrace_struct_buf, \
                                                               libiotrace_struct_size, \
                                                               libiotrace_struct_data->name, prefix_new_##name); \
                                           libiotrace_struct_buf += libiotrace_struct_ret;  /* set pointer to end of written characters */ \
                                           libiotrace_struct_size -= libiotrace_struct_ret; /* resize buffer size */ \
                                         }
#  define LIBIOTRACE_STRUCT_STRUCT(type, name) char prefix_new_##name [strlen(prefix) + sizeof(#name) +1]; /*+1 weil _ als Trennzeichen*/\
                                         if(*prefix == '\0') { \
                                           snprintf(prefix_new_##name, sizeof(prefix_new_##name),"%s", #name);\
                                         } else { \
                                           snprintf(prefix_new_##name, sizeof(prefix_new_##name),"%s_%s", prefix, #name);\
                                         }\
                                         libiotrace_struct_ret = libiotrace_struct_push_##type(libiotrace_struct_buf, \
                                                             libiotrace_struct_size, \
                                                             &(libiotrace_struct_data->name), prefix_new_##name); \
                                         libiotrace_struct_buf += libiotrace_struct_ret;  /* set pointer to end of written characters */ \
                                         libiotrace_struct_size -= libiotrace_struct_ret; /* resize buffer size */
#  define LIBIOTRACE_STRUCT_ARRAY_BITFIELD(type, name)
#  define LIBIOTRACE_STRUCT_ENUM(type, name)
#  define LIBIOTRACE_STRUCT_INT(name) LIBIOTRACE_STRUCT_ELEMENT(name, %d, libiotrace_struct_data->name)
#  define LIBIOTRACE_STRUCT_CHAR(name) LIBIOTRACE_STRUCT_ELEMENT(name, %d, libiotrace_struct_data->name)
#  define LIBIOTRACE_STRUCT_PID_T(name) LIBIOTRACE_STRUCT_ELEMENT(name, %u, libiotrace_struct_data->name)
#  define LIBIOTRACE_STRUCT_CSTRING(name, length) LIBIOTRACE_STRUCT_ELEMENT(name, "%s", libiotrace_struct_data->name)
#  define LIBIOTRACE_STRUCT_CSTRING_P(name, max_length) LIBIOTRACE_STRUCT_ELEMENT(name, "%s", libiotrace_struct_data->name)
#  define LIBIOTRACE_STRUCT_CSTRING_P_CONST(name, max_length)
#  define LIBIOTRACE_STRUCT_CLOCK_T(name)
#  define LIBIOTRACE_STRUCT_FILE_P(name)
#  define LIBIOTRACE_STRUCT_LONG_INT(name)
#  define LIBIOTRACE_STRUCT_SIZE_T(name) LIBIOTRACE_STRUCT_ELEMENT(name, %ld, libiotrace_struct_data->name)
#  define LIBIOTRACE_STRUCT_SSIZE_T(name)
#  define LIBIOTRACE_STRUCT_OFF_T(name)
#  define LIBIOTRACE_STRUCT_U_INT64_T(name) LIBIOTRACE_STRUCT_ELEMENT(name, %lu, libiotrace_struct_data->name) /*mpi_file->written_bytes*/
#  define LIBIOTRACE_STRUCT_VOID_P(name)
#  define LIBIOTRACE_STRUCT_VOID_P_CONST(name)
#  define LIBIOTRACE_STRUCT_FD_SET_P(name)
#  if defined(HAVE_DLMOPEN) && defined(WITH_DL_IO)
#    define LIBIOTRACE_STRUCT_LMID_T(name)
#  endif
#  define LIBIOTRACE_STRUCT_SHORT(name)
#  define LIBIOTRACE_STRUCT_DEV_T(name) LIBIOTRACE_STRUCT_ELEMENT(name, %lu, libiotrace_struct_data->name)
#  define LIBIOTRACE_STRUCT_INO_T(name) LIBIOTRACE_STRUCT_ELEMENT(name, %lu, libiotrace_struct_data->name)
#  define LIBIOTRACE_STRUCT_MALLOC_STRING_ARRAY(name, max_size, max_length_per_element)
#  define LIBIOTRACE_STRUCT_MALLOC_PTR_ARRAY(name, max_size)
#  define LIBIOTRACE_STRUCT_INT_ARRAY(name, max_size)
#  define LIBIOTRACE_STRUCT_SA_FAMILY_T(name)
#  define LIBIOTRACE_STRUCT_KEY_VALUE_ARRAY(name, max_size, max_length_per_cstring)

/* insert new line for new data-type here */
/* ----------------------------------------------------------------------------------------------------------------------- */
#else
/* ----------------------------------------------------------------------------------------------------------------------- */
#  error "Undefined LIBIOTRACE_STRUCT value."
#endif

#endif
