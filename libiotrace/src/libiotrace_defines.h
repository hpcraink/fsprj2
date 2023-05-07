#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <limits.h>
#include <math.h>

#include "common/error.h"
#include "libiotrace_defines_utils.h"

/*
 * To add a new data-type for generating the struct and the json-cstring seven lines
 * have to be added. The positions for adding a new data-type are marked with:
 * insert new line for new data-type here
 */

/* All intern variables are named with prefix "libiotrace_struct_" to prevent shadowing.
 * All functions are also named with prefix "libiotrace_struct_" to prevent conflicts. */

/* Use only provided macros to write data into VOID_P elements and arrays. These macros
 * also set additional informations like count of elements. */

/* TODO: add prefix "__" to all only intern used variables (like void_p_enum_*, size_* and start_)
 * => change all references of these variables */

/* #defines for error handling */
#ifndef LIBIOTRACE_DEFINES_H
#  define LIBIOTRACE_DEFINES_H

/* values for #define LIBIOTRACE_STRUCT */
#  define LIBIOTRACE_STRUCT_DATA_TYPE         1 /* generate struct */
#  define LIBIOTRACE_STRUCT_PRINT             2 /* generate print-function to print struct as json */
#  define LIBIOTRACE_STRUCT_BYTES_COUNT       3 /* generate function to evaluate max size of json-string */
#  define LIBIOTRACE_STRUCT_SIZEOF            4 /* generate function to evaluate size for LIBIOTRACE_STRUCT_COPY */
#  define LIBIOTRACE_STRUCT_COPY              5 /* generate function to deep copy struct (with VOID_P elements) */
#  define LIBIOTRACE_STRUCT_FREE              6 /* generate function to free malloc'ed memory */
#  define LIBIOTRACE_STRUCT_PUSH_BYTES_COUNT  7 /* generate function to evaluate size for HTTP Posts */
#  define LIBIOTRACE_STRUCT_PUSH              8 /* generate function to generate POST request */

/* #defines for error handling */
#  define LIBIOTRACE_STRUCT_ENUM_ERROR(value) LOG_ERROR_AND_DIE("unknown value \"%u\" of enum", value);
#  define LIBIOTRACE_STRUCT_SIZE_ERROR(ret, size) if (ret >= size) { \
                                                LOG_ERROR_AND_DIE("output buffer not big enough"); \
                                            }

/* macros for setting VOID_P elements */
#define LIBIOTRACE_STRUCT_SET_VOID_P(struct_name, element, substruct_type, value) struct_name.__void_p_enum_##element = \
                                                                                __void_p_enum_##element##_##substruct_type; \
                                                                            struct_name.__##element = (void*) (&value);
#define LIBIOTRACE_STRUCT_SET_VOID_P_NULL(struct_name, element) struct_name.__##element = NULL;

/* macros for setting malloced string array */
#define LIBIOTRACE_STRUCT_SET_MALLOC_STRING_ARRAY(struct_name, array_name, malloced_array, start, size) struct_name.__size_##array_name = size; \
                                                                                                  struct_name.__start_##array_name = start; \
                                                                                                  struct_name.__##array_name = malloced_array;
#define LIBIOTRACE_STRUCT_SET_MALLOC_STRING_ARRAY_NULL(struct_name, array_name) struct_name.__##array_name = NULL;

/* macros for setting malloced ptr array */
#define LIBIOTRACE_STRUCT_SET_MALLOC_PTR_ARRAY(struct_name, array_name, malloced_array, start, size) struct_name.__size_##array_name = size; \
                                                                                               struct_name.__start_##array_name = start; \
                                                                                               struct_name.__##array_name = malloced_array;
#define LIBIOTRACE_STRUCT_SET_MALLOC_PTR_ARRAY_NULL(struct_name, array_name) struct_name.__##array_name = NULL;

/* macros for setting int array */
#define LIBIOTRACE_STRUCT_SET_INT_ARRAY(struct_name, array_name, array, size) struct_name.__size_##array_name = size; \
                                                                        struct_name.__##array_name = array;
#define LIBIOTRACE_STRUCT_SET_INT_ARRAY_NULL(struct_name, array_name) struct_name.__##array_name = NULL;

/* macros for setting struct array */
#define LIBIOTRACE_STRUCT_SET_STRUCT_ARRAY(struct_name, array_name, array, size) struct_name.__size_##array_name = size; \
                                                                           struct_name.__##array_name = array;
#define LIBIOTRACE_STRUCT_SET_STRUCT_ARRAY_NULL(struct_name, array_name) struct_name.__##array_name = NULL;

/* macros for setting key value array */
#define LIBIOTRACE_STRUCT_INIT_KEY_VALUE_ARRAY(struct_name, array_name, keys_array, values_array) struct_name.__size_##array_name = 0; \
                                                                                            struct_name.__##keys_##array_name = keys_array; \
                                                                                            struct_name.__##values_##array_name = values_array;
#define LIBIOTRACE_STRUCT_SET_KEY_VALUE_ARRAY_NULL(struct_name, array_name) struct_name.__size_##array_name = 0; \
                                                                      struct_name.__##keys_##array_name = NULL; \
                                                                      struct_name.__##values_##array_name = NULL;
#define LIBIOTRACE_STRUCT_ADD_KEY_VALUE(struct_name, array_name, key, value) struct_name.__##keys_##array_name[struct_name.__size_##array_name] = key; \
                                                                       struct_name.__##values_##array_name[struct_name.__size_##array_name] = value; \
                                                                       struct_name.__size_##array_name++;

#define COUNT_DEC_AS_CHAR(type) ceil(log10(pow(2, sizeof(type) * CHAR_BIT)))

#define LIBIOTRACE_STRUCT_QUOT(key) "\""#key"\""

#define LIBIOTRACE_STRUCT_SNPRINTF(...) libiotrace_struct_ret_int = snprintf(libiotrace_struct_buf, libiotrace_struct_size, __VA_ARGS__); \
                                    if (0 > libiotrace_struct_ret_int) { \
                                        LOG_ERROR_AND_DIE("snprintf returned value < 0"); \
                                    } \
                                    LIBIOTRACE_STRUCT_SIZE_ERROR((size_t)libiotrace_struct_ret_int, libiotrace_struct_size) /* don't write more characters then size of buffer */ \
                                    libiotrace_struct_buf += libiotrace_struct_ret_int;  /* set pointer to end of written characters */ \
                                    libiotrace_struct_size -= libiotrace_struct_ret_int; /* resize buffer size */

#define LIBIOTRACE_STRUCT_WRITE(value) libiotrace_struct_ret = libiotrace_struct_write(libiotrace_struct_buf, libiotrace_struct_size, value); \
                                   LIBIOTRACE_STRUCT_SIZE_ERROR(libiotrace_struct_ret, libiotrace_struct_size) /* don't write more characters then size of buffer */ \
                                   libiotrace_struct_buf += libiotrace_struct_ret;  /* set pointer to end of written characters */ \
                                   libiotrace_struct_size -= libiotrace_struct_ret; /* resize buffer size */

#endif /* LIBIOTRACE_DEFINES_H */

// ---------------------------------------------------------------------------------------------------
// all following defines are different depending on the value of LIBIOTRACE_STRUCT
// they can be included multiple times because they are outside of the LIBIOTRACE_DEFINES_H guard
// ---------------------------------------------------------------------------------------------------

#ifdef LIBIOTRACE_STRUCT

#undef LIBIOTRACE_STRUCT_ELEMENT_SIZE
#undef LIBIOTRACE_STRUCT_TYPE_SIZE_DEC
#undef LIBIOTRACE_STRUCT_TYPE

#undef LIBIOTRACE_STRUCT_ELEMENT

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

#  define LIBIOTRACE_STRUCT_VOID_P_START(name) void *__##name; enum {
#  define LIBIOTRACE_STRUCT_VOID_P_ELEMENT(name, element) __void_p_enum_##name##_##element,
#  define LIBIOTRACE_STRUCT_VOID_P_END(name) } __void_p_enum_##name;

#  define LIBIOTRACE_STRUCT_START(name) struct name {
#  define LIBIOTRACE_STRUCT_END };

#  define LIBIOTRACE_STRUCT_STRUCT_ARRAY(type, name, max_length) struct type **__##name; size_t __size_##name;

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
#  ifdef HAVE_OFF64_T
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
#  define LIBIOTRACE_STRUCT_MALLOC_STRING_ARRAY(name, max_size, max_length_per_element) size_t __start_##name; size_t __size_##name; char **__##name;
#  define LIBIOTRACE_STRUCT_MALLOC_PTR_ARRAY(name, max_size) size_t __start_##name; size_t __size_##name; void **__##name;
#  define LIBIOTRACE_STRUCT_INT_ARRAY(name, max_size) size_t __size_##name; int *__##name;
#  define LIBIOTRACE_STRUCT_SA_FAMILY_T(name) sa_family_t name;
#  define LIBIOTRACE_STRUCT_KEY_VALUE_ARRAY(name, max_size, max_length_per_cstring) size_t __size_##name; char **__keys_##name; char **__values_##name;
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
 * size_t libiotrace_struct_print_array_<name of array>(char* buf, size_t size, struct name *data)
 * size_t libiotrace_struct_print_enum_<name of enum>(char* buf, size_t size, struct name *data)
 *
 * for printing struct as json-cstring. The size argument specifies the maximum number
 * of characters to produce.
 * */

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

#  define LIBIOTRACE_STRUCT_ENUM_START(name) size_t libiotrace_struct_print_enum_##name(char* libiotrace_struct_buf, \
                                             size_t libiotrace_struct_size, enum name *libiotrace_struct_data) { \
                                         size_t libiotrace_struct_ret = 0; \
                                         size_t libiotrace_struct_start_size = libiotrace_struct_size; \
                                         switch (*libiotrace_struct_data) {
#  define LIBIOTRACE_STRUCT_ENUM_ELEMENT(name) case name: \
                                           LIBIOTRACE_STRUCT_WRITE(LIBIOTRACE_STRUCT_QUOT(name)",") \
                                           break;
#  define LIBIOTRACE_STRUCT_ENUM_END default: \
                                 LIBIOTRACE_STRUCT_ENUM_ERROR(*libiotrace_struct_data) \
                               } return libiotrace_struct_start_size - libiotrace_struct_size;}

#  define LIBIOTRACE_STRUCT_ARRAY_BITFIELD_START(name) size_t libiotrace_struct_print_array_##name(char* libiotrace_struct_buf, \
                                                       size_t libiotrace_struct_size, struct name *libiotrace_struct_data) { \
                                                   char libiotrace_struct_hasElements = 0; \
                                                   size_t libiotrace_struct_ret = 0; \
                                                   size_t libiotrace_struct_start_size = libiotrace_struct_size; \
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
                                    size_t libiotrace_struct_ret = 0; \
                                    int libiotrace_struct_ret_int ATTRIBUTE_UNUSED = 0; \
                                    size_t libiotrace_struct_start_size = libiotrace_struct_size; \
                                    LIBIOTRACE_STRUCT_WRITE("{")
#  define LIBIOTRACE_STRUCT_END if (libiotrace_struct_hasElements) { \
                            libiotrace_struct_buf--;  /* remove last comma */ \
                            libiotrace_struct_size++; /* and resize buffer size */ \
                          } \
                          LIBIOTRACE_STRUCT_WRITE("}") \
                          return libiotrace_struct_start_size - libiotrace_struct_size;}

#  define LIBIOTRACE_STRUCT_VOID_P_START(name) if (NULL != libiotrace_struct_data->__##name) { \
                                           libiotrace_struct_hasElements = 1; \
                                           LIBIOTRACE_STRUCT_WRITE(LIBIOTRACE_STRUCT_QUOT(name)":") \
                                           switch (libiotrace_struct_data->__void_p_enum_##name) {
#  define LIBIOTRACE_STRUCT_VOID_P_ELEMENT(name, element) case __void_p_enum_##name##_##element: \
                                                      libiotrace_struct_ret = libiotrace_struct_print_##element(libiotrace_struct_buf, \
                                                                          libiotrace_struct_size, (struct element*) \
                                                                          libiotrace_struct_data->__##name); \
                                                      libiotrace_struct_buf += libiotrace_struct_ret;  /* set pointer to end of written characters */ \
                                                      libiotrace_struct_size -= libiotrace_struct_ret; /* resize buffer size */\
                                                      break;
#  define LIBIOTRACE_STRUCT_VOID_P_END(name)   default: \
                                           LIBIOTRACE_STRUCT_ENUM_ERROR(libiotrace_struct_data->__void_p_enum_##name) \
                                         } \
                                         LIBIOTRACE_STRUCT_WRITE(",") \
                                       }

#  define LIBIOTRACE_STRUCT_STRUCT_ARRAY(type, name, max_length) if (NULL != libiotrace_struct_data->__##name) { \
                                                             libiotrace_struct_hasElements = 1; \
                                                             LIBIOTRACE_STRUCT_WRITE(LIBIOTRACE_STRUCT_QUOT(name)":[") \
                                                             size_t libiotrace_struct_count_##name; \
                                                             for (libiotrace_struct_count_##name = 0; \
                                                                  libiotrace_struct_count_##name < libiotrace_struct_data->__size_##name; \
                                                                  libiotrace_struct_count_##name++) { \
                                                               libiotrace_struct_ret = libiotrace_struct_print_##type(libiotrace_struct_buf, \
                                                                                                          libiotrace_struct_size, \
                                                                                                          *((libiotrace_struct_data->__##name) + libiotrace_struct_count_##name)); \
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
#  define LIBIOTRACE_STRUCT_PID_T(name) LIBIOTRACE_STRUCT_ELEMENT(name, %d, libiotrace_struct_data->name)
#  define LIBIOTRACE_STRUCT_CSTRING(name, length) LIBIOTRACE_STRUCT_ESCAPE(name)
#  define LIBIOTRACE_STRUCT_CSTRING_P(name, max_length) LIBIOTRACE_STRUCT_ESCAPE(name)
#  define LIBIOTRACE_STRUCT_CSTRING_P_CONST(name, max_length) LIBIOTRACE_STRUCT_ESCAPE(name)
#  define LIBIOTRACE_STRUCT_CLOCK_T(name) LIBIOTRACE_STRUCT_ELEMENT(name, %lu, libiotrace_struct_data->name)
#  define LIBIOTRACE_STRUCT_FILE_P(name) LIBIOTRACE_STRUCT_ELEMENT(name, "%p", (void *)libiotrace_struct_data->name)
#  define LIBIOTRACE_STRUCT_LONG_INT(name) LIBIOTRACE_STRUCT_ELEMENT(name, %ld, libiotrace_struct_data->name)
#  define LIBIOTRACE_STRUCT_SIZE_T(name) LIBIOTRACE_STRUCT_ELEMENT(name, %lu, libiotrace_struct_data->name)
#  define LIBIOTRACE_STRUCT_SSIZE_T(name) LIBIOTRACE_STRUCT_ELEMENT(name, %ld, libiotrace_struct_data->name)
#  define LIBIOTRACE_STRUCT_OFF_T(name) LIBIOTRACE_STRUCT_ELEMENT(name, %ld, libiotrace_struct_data->name)
#  define LIBIOTRACE_STRUCT_U_INT64_T(name) LIBIOTRACE_STRUCT_ELEMENT(name, %lu, libiotrace_struct_data->name)
#  define LIBIOTRACE_STRUCT_VOID_P(name) LIBIOTRACE_STRUCT_ELEMENT(name, "%p", libiotrace_struct_data->name)
#  define LIBIOTRACE_STRUCT_VOID_P_CONST(name) LIBIOTRACE_STRUCT_ELEMENT(name, "%p", libiotrace_struct_data->name)
#  define LIBIOTRACE_STRUCT_FD_SET_P(name) if (NULL != libiotrace_struct_data->name) { \
                                       libiotrace_struct_hasElements = 1; \
                                       LIBIOTRACE_STRUCT_WRITE(LIBIOTRACE_STRUCT_QUOT(name)":[") \
                                       char libiotrace_struct_hasElements_##name = 0; \
                                       for (int libiotrace_struct_count_##name = 0; libiotrace_struct_count_##name < FD_SETSIZE; libiotrace_struct_count_##name++) { \
                                         if (FD_ISSET(libiotrace_struct_count_##name, libiotrace_struct_data->name)) { \
                                           LIBIOTRACE_STRUCT_SNPRINTF("%d,", libiotrace_struct_count_##name) \
                                           libiotrace_struct_hasElements_##name = 1; \
                                         } \
                                       } \
                                       if (libiotrace_struct_hasElements_##name) { \
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
#  define LIBIOTRACE_STRUCT_MALLOC_STRING_ARRAY(name, max_size, max_length_per_element) if (NULL != libiotrace_struct_data->__##name) { \
                                                                                    libiotrace_struct_hasElements = 1; \
                                                                                    LIBIOTRACE_STRUCT_WRITE(LIBIOTRACE_STRUCT_QUOT(name)":[") \
                                                                                    size_t libiotrace_struct_count_##name; \
                                                                                    for (libiotrace_struct_count_##name = libiotrace_struct_data->__start_##name; \
                                                                                         libiotrace_struct_count_##name < libiotrace_struct_data->__size_##name; \
                                                                                         libiotrace_struct_count_##name++) { \
                                                                                      libiotrace_struct_ret = libiotrace_struct_print_cstring(libiotrace_struct_buf, libiotrace_struct_size, \
                                                                                                                                  libiotrace_struct_data->__##name[libiotrace_struct_count_##name]); \
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
#  define LIBIOTRACE_STRUCT_MALLOC_PTR_ARRAY(name, max_size) if (NULL != libiotrace_struct_data->__##name) { \
                                                         libiotrace_struct_hasElements = 1; \
                                                         LIBIOTRACE_STRUCT_WRITE(LIBIOTRACE_STRUCT_QUOT(name)":[") \
                                                         size_t libiotrace_struct_count_##name; \
                                                         for (libiotrace_struct_count_##name = libiotrace_struct_data->__start_##name; \
                                                              libiotrace_struct_count_##name < libiotrace_struct_data->__size_##name; \
                                                              libiotrace_struct_count_##name++) { \
                                                           LIBIOTRACE_STRUCT_SNPRINTF("\"%p\",", libiotrace_struct_data->__##name[libiotrace_struct_count_##name]) \
                                                         } \
                                                         if (libiotrace_struct_count_##name > 0) { \
                                                           libiotrace_struct_buf--;  /* remove last comma */ \
                                                           libiotrace_struct_size++; /* and resize buffer size */ \
                                                         } \
                                                         LIBIOTRACE_STRUCT_WRITE("],") \
                                                       }
#  define LIBIOTRACE_STRUCT_INT_ARRAY(name, max_size) if (NULL != libiotrace_struct_data->__##name) { \
                                                  libiotrace_struct_hasElements = 1; \
                                                  LIBIOTRACE_STRUCT_WRITE(LIBIOTRACE_STRUCT_QUOT(name)":[") \
                                                  size_t libiotrace_struct_count_##name; \
                                                  for (libiotrace_struct_count_##name = 0; \
                                                       libiotrace_struct_count_##name < libiotrace_struct_data->__size_##name; \
                                                       libiotrace_struct_count_##name++) { \
                                                    LIBIOTRACE_STRUCT_SNPRINTF("%d,", libiotrace_struct_data->__##name[libiotrace_struct_count_##name]) \
                                                  } \
                                                  if (libiotrace_struct_count_##name > 0) { \
                                                    libiotrace_struct_buf--;  /* remove last comma */ \
                                                    libiotrace_struct_size++; /* and resize buffer size */ \
                                                  } \
                                                  LIBIOTRACE_STRUCT_WRITE("],") \
                                                }
#  define LIBIOTRACE_STRUCT_SA_FAMILY_T(name) LIBIOTRACE_STRUCT_ELEMENT(name, %hu, libiotrace_struct_data->name)
#  define LIBIOTRACE_STRUCT_KEY_VALUE_ARRAY(name, max_size, max_length_per_cstring) if (NULL != libiotrace_struct_data->__keys_##name) { \
                                                                                libiotrace_struct_hasElements = 1; \
                                                                                LIBIOTRACE_STRUCT_WRITE(LIBIOTRACE_STRUCT_QUOT(name)":{") \
                                                                                size_t libiotrace_struct_count_##name; \
                                                                                for (libiotrace_struct_count_##name = 0; \
                                                                                     libiotrace_struct_count_##name < libiotrace_struct_data->__size_##name; \
                                                                                     libiotrace_struct_count_##name++) { \
                                                                                  libiotrace_struct_ret = libiotrace_struct_print_cstring(libiotrace_struct_buf, libiotrace_struct_size, \
                                                                                                      libiotrace_struct_data->__keys_##name[libiotrace_struct_count_##name]); \
                                                                                  LIBIOTRACE_STRUCT_SIZE_ERROR(libiotrace_struct_ret, libiotrace_struct_size) \
                                                                                  libiotrace_struct_buf += libiotrace_struct_ret;  /* set pointer to end of written characters */ \
                                                                                  libiotrace_struct_size -= libiotrace_struct_ret; /* resize buffer size */ \
                                                                                  LIBIOTRACE_STRUCT_WRITE(":") \
                                                                                  libiotrace_struct_ret = libiotrace_struct_print_cstring(libiotrace_struct_buf, libiotrace_struct_size, \
                                                                                                      libiotrace_struct_data->__values_##name[libiotrace_struct_count_##name]); \
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
 * size_t libiotrace_struct_max_size_<name of struct>(void)
 * size_t libiotrace_struct_max_size_array_<name of struct>(void)
 * size_t libiotrace_struct_max_size_enum_<name of struct>(void)
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

#  define LIBIOTRACE_STRUCT_ENUM_START(name) size_t libiotrace_struct_max_size_enum_##name(void) { \
                                         size_t libiotrace_struct_size_value = 0;
#  define LIBIOTRACE_STRUCT_ENUM_ELEMENT(name) if (sizeof(#name) > libiotrace_struct_size_value) \
                                           libiotrace_struct_size_value = sizeof(#name); /* get greatest possible value */
#  define LIBIOTRACE_STRUCT_ENUM_END return libiotrace_struct_size_value \
                                      - 1   /* trailing null character */ \
                                      + 2;} /* quotation marks (for value) */

#  define LIBIOTRACE_STRUCT_ARRAY_BITFIELD_START(name) size_t libiotrace_struct_max_size_array_##name(void) { \
                                                   char libiotrace_struct_hasElements = 0; \
                                                   size_t libiotrace_struct_size = 1; /* open parentheses */
#  define LIBIOTRACE_STRUCT_ARRAY_BITFIELD_ELEMENT(name) libiotrace_struct_hasElements = 1; \
                                                   libiotrace_struct_size += sizeof(#name) /* add each possible value */ \
                                                                       - 1  /* trailing null character */ \
                                                                       + 3; /* quotation marks and comma */
#  define LIBIOTRACE_STRUCT_ARRAY_BITFIELD_END if (libiotrace_struct_hasElements) libiotrace_struct_size--; /* remove last comma */ \
                                         return libiotrace_struct_size + 1;} /* close parentheses */

#  define LIBIOTRACE_STRUCT_START(name) size_t libiotrace_struct_max_size_##name(void) { \
                                    char libiotrace_struct_hasElements = 0; \
                                    size_t libiotrace_struct_size_void_p ATTRIBUTE_UNUSED; \
                                    size_t libiotrace_struct_size_void_p_tmp ATTRIBUTE_UNUSED; \
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
#  define LIBIOTRACE_STRUCT_CHAR(name) LIBIOTRACE_STRUCT_ELEMENT_SIZE(name, LIBIOTRACE_STRUCT_TYPE_SIZE_DEC(char) \
                                                                + 1) /* for sign (-) */
#  define LIBIOTRACE_STRUCT_PID_T(name) LIBIOTRACE_STRUCT_ELEMENT_SIZE(name, LIBIOTRACE_STRUCT_TYPE_SIZE_DEC(pid_t) \
                                                                       + 1) /* for sign (-) */
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
#  define LIBIOTRACE_STRUCT_SIZE_T(name) LIBIOTRACE_STRUCT_ELEMENT_SIZE(name, LIBIOTRACE_STRUCT_TYPE_SIZE_DEC(size_t))
#  define LIBIOTRACE_STRUCT_SSIZE_T(name) LIBIOTRACE_STRUCT_ELEMENT_SIZE(name, LIBIOTRACE_STRUCT_TYPE_SIZE_DEC(ssize_t) \
                                                                   + 1) /* for sign (-) */
#  ifdef HAVE_OFF64_T
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

#  define LIBIOTRACE_STRUCT_VOID_P_START(name) if (NULL != libiotrace_struct_data->__##name) { \
                                           switch (libiotrace_struct_data->__void_p_enum_##name) {
#  define LIBIOTRACE_STRUCT_VOID_P_ELEMENT(name, element) case __void_p_enum_##name##_##element: \
                                                      libiotrace_struct_size += libiotrace_struct_sizeof_##element( \
                                                                            (struct element*) libiotrace_struct_data->__##name); \
                                                      break;
#  define LIBIOTRACE_STRUCT_VOID_P_END(name) default: LIBIOTRACE_STRUCT_ENUM_ERROR(libiotrace_struct_data->__void_p_enum_##name) } }

#  define LIBIOTRACE_STRUCT_START(name) int libiotrace_struct_sizeof_##name(struct name *libiotrace_struct_data ATTRIBUTE_UNUSED) { \
                                    size_t libiotrace_struct_size = sizeof(struct name);
#  define LIBIOTRACE_STRUCT_END return libiotrace_struct_size;}

#  define LIBIOTRACE_STRUCT_STRUCT_ARRAY(type, name, max_length) if (NULL != libiotrace_struct_data->__##name) { \
                                                             libiotrace_struct_size += sizeof(struct type *) \
                                                                                 * libiotrace_struct_data->__size_##name; \
                                                             size_t libiotrace_struct_count_##name; \
                                                             for (libiotrace_struct_count_##name = 0; \
                                                                  libiotrace_struct_count_##name < libiotrace_struct_data->__size_##name; \
                                                                  libiotrace_struct_count_##name++) { \
                                                               libiotrace_struct_size += libiotrace_struct_sizeof_##type( \
                                                                                     *((libiotrace_struct_data->__##name) + libiotrace_struct_count_##name)); \
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
#  define LIBIOTRACE_STRUCT_MALLOC_STRING_ARRAY(name, max_size, max_length_per_element) if (NULL != libiotrace_struct_data->__##name) { \
                                                                                    libiotrace_struct_size += sizeof(char *) * (libiotrace_struct_data->__size_##name \
                                                                                                        - libiotrace_struct_data->__start_##name); \
                                                                                    for (size_t libiotrace_struct_count_##name = libiotrace_struct_data->__start_##name; \
                                                                                         libiotrace_struct_count_##name < libiotrace_struct_data->__size_##name && \
                                                                                         libiotrace_struct_count_##name < max_size; \
                                                                                         libiotrace_struct_count_##name++) { \
                                                                                      libiotrace_struct_size += strnlen(libiotrace_struct_data->__##name[libiotrace_struct_count_##name], \
                                                                                                                  max_length_per_element) \
                                                                                                          + 1; /* +1 for trailing null character */ \
                                                                                    } \
                                                                                  }
#  define LIBIOTRACE_STRUCT_MALLOC_PTR_ARRAY(name, max_size) if (NULL != libiotrace_struct_data->__##name) { \
                                                         libiotrace_struct_size += sizeof(void *) * (libiotrace_struct_data->__size_##name \
                                                                             - libiotrace_struct_data->__start_##name); \
                                                       }
#  define LIBIOTRACE_STRUCT_INT_ARRAY(name, max_size) if (NULL != libiotrace_struct_data->__##name) { \
                                                  libiotrace_struct_size += sizeof(int) * libiotrace_struct_data->__size_##name; \
                                                }
#  define LIBIOTRACE_STRUCT_SA_FAMILY_T(name)
#  define LIBIOTRACE_STRUCT_KEY_VALUE_ARRAY(name, max_size, max_length_per_cstring) if (NULL != libiotrace_struct_data->__keys_##name) { \
                                                                                libiotrace_struct_size += sizeof(char *) * libiotrace_struct_data->__size_##name \
                                                                                                    * 2; /* key and value arrays */ \
                                                                                for (size_t libiotrace_struct_count_##name = 0; \
                                                                                     libiotrace_struct_count_##name < libiotrace_struct_data->__size_##name && \
                                                                                     libiotrace_struct_count_##name < max_size; \
                                                                                     libiotrace_struct_count_##name++) { \
                                                                                  libiotrace_struct_size += strnlen(libiotrace_struct_data->__keys_##name[libiotrace_struct_count_##name], \
                                                                                                              max_length_per_cstring) \
                                                                                                      + 1; /* +1 for trailing null character */ \
                                                                                  libiotrace_struct_size += strnlen(libiotrace_struct_data->__values_##name[libiotrace_struct_count_##name], \
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

size_t libiotrace_struct_copy_cstring_p(char *libiotrace_struct_to, const char *libiotrace_struct_from, size_t libiotrace_struct_max_length) {
    size_t libiotrace_struct_i;
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

#  define LIBIOTRACE_STRUCT_VOID_P_START(name) if (NULL != libiotrace_struct_data->__##name) { \
                                           switch (libiotrace_struct_data->__void_p_enum_##name) {
#  define LIBIOTRACE_STRUCT_VOID_P_ELEMENT(name, element) case __void_p_enum_##name##_##element: \
                                                      libiotrace_struct_copy->__##name = libiotrace_struct_buf; \
                                                      libiotrace_struct_buf = libiotrace_struct_copy_##element(libiotrace_struct_buf, \
                                                                          (struct element*) libiotrace_struct_data->__##name); \
                                                      break;
#  define LIBIOTRACE_STRUCT_VOID_P_END(name) default: LIBIOTRACE_STRUCT_ENUM_ERROR(libiotrace_struct_data->__void_p_enum_##name) } }

#  define LIBIOTRACE_STRUCT_START(name) void* libiotrace_struct_copy_##name(void *libiotrace_struct_buf, struct name *libiotrace_struct_data) { \
                                    struct name *libiotrace_struct_copy ATTRIBUTE_UNUSED = (struct name *)libiotrace_struct_buf; \
                                    size_t libiotrace_struct_ret ATTRIBUTE_UNUSED; \
                                    memcpy(libiotrace_struct_buf, (void *) libiotrace_struct_data, sizeof(struct name)); \
                                    libiotrace_struct_buf = (char *)libiotrace_struct_buf + sizeof(struct name);
#  define LIBIOTRACE_STRUCT_END return libiotrace_struct_buf;}

#  define LIBIOTRACE_STRUCT_STRUCT_ARRAY(type, name, max_length) if (NULL != libiotrace_struct_data->__##name) { \
                                                             libiotrace_struct_copy->__##name = (struct type **)libiotrace_struct_buf; \
                                                             libiotrace_struct_buf = (char *)libiotrace_struct_buf + sizeof(struct type *) \
                                                                                * libiotrace_struct_data->__size_##name; \
                                                             size_t libiotrace_struct_count_##name; \
                                                             for (libiotrace_struct_count_##name = 0; \
                                                                  libiotrace_struct_count_##name < libiotrace_struct_data->__size_##name; \
                                                                  libiotrace_struct_count_##name++) { \
                                                               libiotrace_struct_copy->__##name[libiotrace_struct_count_##name] = (struct type *)libiotrace_struct_buf; \
                                                               libiotrace_struct_buf = libiotrace_struct_copy_##type(libiotrace_struct_buf, \
                                                                                                         *((libiotrace_struct_data->__##name) \
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
                                                    libiotrace_struct_buf = (char *)libiotrace_struct_buf + libiotrace_struct_ret; \
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
                                       libiotrace_struct_buf = (char *)libiotrace_struct_buf + sizeof(fd_set); \
                                     }
#  if defined(HAVE_DLMOPEN) && defined(WITH_DL_IO)
#    define LIBIOTRACE_STRUCT_LMID_T(name)
#  endif
#  define LIBIOTRACE_STRUCT_SHORT(name)
#  define LIBIOTRACE_STRUCT_DEV_T(name)
#  define LIBIOTRACE_STRUCT_INO_T(name)
#  define LIBIOTRACE_STRUCT_MALLOC_STRING_ARRAY(name, max_size, max_length_per_element) if (NULL != libiotrace_struct_data->__##name) { \
                                                                                    libiotrace_struct_copy->__##name = (char **) libiotrace_struct_buf; \
                                                                                    libiotrace_struct_buf = (char *)libiotrace_struct_buf + sizeof(char *) * (libiotrace_struct_data->__size_##name \
                                                                                                                         - libiotrace_struct_data->__start_##name); \
                                                                                    for (size_t libiotrace_struct_count_##name = libiotrace_struct_data->__start_##name; \
                                                                                         libiotrace_struct_count_##name < libiotrace_struct_data->__size_##name && \
                                                                                         libiotrace_struct_count_##name - libiotrace_struct_data->__start_##name < max_size; \
                                                                                         libiotrace_struct_count_##name++) { \
                                                                                      if (NULL != libiotrace_struct_data->__##name[libiotrace_struct_count_##name]) { \
                                                                                        libiotrace_struct_copy->__##name[libiotrace_struct_count_##name \
                                                                                                               - libiotrace_struct_data->__start_##name] = (char *) libiotrace_struct_buf; \
                                                                                        libiotrace_struct_ret = libiotrace_struct_copy_cstring_p((char *)libiotrace_struct_buf, \
                                                                                                                                     libiotrace_struct_data->__##name[libiotrace_struct_count_##name], \
                                                                                                                                     max_length_per_element); \
                                                                                        libiotrace_struct_buf = (char *)libiotrace_struct_buf + libiotrace_struct_ret; \
                                                                                      } else { \
                                                                                        libiotrace_struct_copy->__##name[libiotrace_struct_count_##name] = NULL; \
                                                                                      } \
                                                                                    } \
                                                                                    libiotrace_struct_copy->__start_##name = 0; \
                                                                                    libiotrace_struct_copy->__size_##name = libiotrace_struct_data->__size_##name \
                                                                                                                    - libiotrace_struct_data->__start_##name; \
                                                                                    if (libiotrace_struct_copy->__size_##name > max_size) { \
                                                                                      libiotrace_struct_copy->__size_##name = max_size; \
                                                                                    } \
                                                                                  }
#  define LIBIOTRACE_STRUCT_MALLOC_PTR_ARRAY(name, max_size) if (NULL != libiotrace_struct_data->__##name) { \
                                                         libiotrace_struct_copy->__##name = (void *) libiotrace_struct_buf; \
                                                         libiotrace_struct_buf = (char *)libiotrace_struct_buf + sizeof(void *) * (libiotrace_struct_data->__size_##name \
                                                                            - libiotrace_struct_data->__start_##name); \
                                                         for (size_t libiotrace_struct_count_##name = libiotrace_struct_data->__start_##name; \
                                                              libiotrace_struct_count_##name < libiotrace_struct_data->__size_##name && \
                                                              libiotrace_struct_count_##name - libiotrace_struct_data->__start_##name < max_size; \
                                                              libiotrace_struct_count_##name++) { \
                                                           if (NULL != libiotrace_struct_data->__##name[libiotrace_struct_count_##name]) { \
                                                             libiotrace_struct_copy->__##name[libiotrace_struct_count_##name \
                                                                                    - libiotrace_struct_data->__start_##name] = (void *) libiotrace_struct_buf; \
                                                           } else { \
                                                             libiotrace_struct_copy->__##name[libiotrace_struct_count_##name] = NULL; \
                                                           } \
                                                         } \
                                                         libiotrace_struct_copy->__start_##name = 0; \
                                                         libiotrace_struct_copy->__size_##name = libiotrace_struct_data->__size_##name \
                                                                                         - libiotrace_struct_data->__start_##name; \
                                                         if (libiotrace_struct_copy->__size_##name > max_size) { \
                                                           libiotrace_struct_copy->__size_##name = max_size; \
                                                         } \
                                                       }
#  define LIBIOTRACE_STRUCT_INT_ARRAY(name, max_size) if (NULL != libiotrace_struct_data->__##name) { \
                                                  memcpy(libiotrace_struct_buf, (void *) libiotrace_struct_data->__##name, \
                                                         sizeof(int) * libiotrace_struct_copy->__size_##name); \
                                                  libiotrace_struct_copy->__##name = (int *) libiotrace_struct_buf; \
                                                  libiotrace_struct_buf = (char *)libiotrace_struct_buf + sizeof(int) * libiotrace_struct_copy->__size_##name; \
                                                }
#  define LIBIOTRACE_STRUCT_SA_FAMILY_T(name)
#  define LIBIOTRACE_STRUCT_KEY_VALUE_ARRAY(name, max_size, max_length_per_cstring) if (NULL != libiotrace_struct_data->__keys_##name) { \
                                                                                libiotrace_struct_copy->__keys_##name = (char **) libiotrace_struct_buf; \
                                                                                libiotrace_struct_buf = (char *)libiotrace_struct_buf + sizeof(char *) * libiotrace_struct_data->__size_##name; \
                                                                                libiotrace_struct_copy->__values_##name = (char **) libiotrace_struct_buf; \
                                                                                libiotrace_struct_buf = (char *)libiotrace_struct_buf + sizeof(char *) * libiotrace_struct_data->__size_##name; \
                                                                                for (size_t libiotrace_struct_count_##name = 0; \
                                                                                     libiotrace_struct_count_##name < libiotrace_struct_data->__size_##name && \
                                                                                     libiotrace_struct_count_##name < max_size; \
                                                                                     libiotrace_struct_count_##name++) { \
                                                                                  if (NULL != libiotrace_struct_data->__keys_##name[libiotrace_struct_count_##name]) { \
                                                                                    libiotrace_struct_copy->__keys_##name[libiotrace_struct_count_##name] = (char *) libiotrace_struct_buf; \
                                                                                    libiotrace_struct_ret = libiotrace_struct_copy_cstring_p((char *)libiotrace_struct_buf, \
                                                                                                                                 libiotrace_struct_data->__keys_##name[libiotrace_struct_count_##name], \
                                                                                                                                 max_length_per_cstring); \
                                                                                    libiotrace_struct_buf = (char *)libiotrace_struct_buf + libiotrace_struct_ret; \
                                                                                  } else { \
                                                                                    libiotrace_struct_copy->__keys_##name[libiotrace_struct_count_##name] = NULL; \
                                                                                  } \
                                                                                  if (NULL != libiotrace_struct_data->__values_##name[libiotrace_struct_count_##name]) { \
                                                                                    libiotrace_struct_copy->__values_##name[libiotrace_struct_count_##name] = (char *) libiotrace_struct_buf; \
                                                                                    libiotrace_struct_ret = libiotrace_struct_copy_cstring_p((char *)libiotrace_struct_buf, \
                                                                                                                                 libiotrace_struct_data->__values_##name[libiotrace_struct_count_##name], \
                                                                                                                                 max_length_per_cstring); \
                                                                                    libiotrace_struct_buf = (char *)libiotrace_struct_buf + libiotrace_struct_ret; \
                                                                                  } else { \
                                                                                    libiotrace_struct_copy->__values_##name[libiotrace_struct_count_##name] = NULL; \
                                                                                  } \
                                                                                } \
                                                                                if (libiotrace_struct_copy->__size_##name > max_size) { \
                                                                                  libiotrace_struct_copy->__size_##name = max_size; \
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

#  define LIBIOTRACE_STRUCT_VOID_P_START(name) if (NULL != libiotrace_struct_data->__##name) { \
                                           switch (libiotrace_struct_data->__void_p_enum_##name) {
#  define LIBIOTRACE_STRUCT_VOID_P_ELEMENT(name, element) case __void_p_enum_##name##_##element: \
                                                      libiotrace_struct_free_##element((struct element*) libiotrace_struct_data->__##name); \
                                                      break;
#  define LIBIOTRACE_STRUCT_VOID_P_END(name) default: LIBIOTRACE_STRUCT_ENUM_ERROR(libiotrace_struct_data->__void_p_enum_##name) } }

#  define LIBIOTRACE_STRUCT_START(name) void libiotrace_struct_free_##name(struct name *libiotrace_struct_data ATTRIBUTE_UNUSED) {
#  define LIBIOTRACE_STRUCT_END }

#  define LIBIOTRACE_STRUCT_STRUCT_ARRAY(type, name, max_length) if (NULL != libiotrace_struct_data->__##name) { \
                                                             size_t libiotrace_struct_count_##name; \
                                                             for (libiotrace_struct_count_##name = 0; \
                                                                  libiotrace_struct_count_##name < libiotrace_struct_data->__size_##name; \
                                                                  libiotrace_struct_count_##name++) { \
                                                                 libiotrace_struct_free_##type(*((libiotrace_struct_data->__##name) \
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
#  define LIBIOTRACE_STRUCT_MALLOC_STRING_ARRAY(name, max_size, max_length_per_element) if (NULL != libiotrace_struct_data->__##name) { \
                                                                                    CALL_REAL_ALLOC_SYNC(free)(libiotrace_struct_data->__##name); \
                                                                                    libiotrace_struct_data->__##name = NULL; \
                                                                                  }
#  define LIBIOTRACE_STRUCT_MALLOC_PTR_ARRAY(name, max_size) if (NULL != libiotrace_struct_data->__##name) { \
                                                         CALL_REAL_ALLOC_SYNC(free)(libiotrace_struct_data->__##name); \
                                                         libiotrace_struct_data->__##name = NULL; \
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
 * size_t libiotrace_struct_push_max_size_<name of struct>(size_t prefix_length)
 *
 * */


#  define LIBIOTRACE_STRUCT_ELEMENT_SIZE(name, sizeValue) libiotrace_struct_size += prefix_length + sizeof(#name) \
                                                                        - 1  /* terminating '\0' from #name */ \
                                                                        + sizeValue \
                                                                        + 2; /* equal sign and comma */
#  define LIBIOTRACE_STRUCT_TYPE_SIZE_DEC(type) COUNT_DEC_AS_CHAR(type)

#  define LIBIOTRACE_STRUCT_ENUM_START(name) size_t libiotrace_struct_push_max_size_enum_##name(void) { \
                                         size_t libiotrace_struct_size_value = 0;
#  define LIBIOTRACE_STRUCT_ENUM_ELEMENT(name) if (sizeof(#name) > libiotrace_struct_size_value) \
                                           libiotrace_struct_size_value = sizeof(#name); /* get greatest possible value */
#  define LIBIOTRACE_STRUCT_ENUM_END  return libiotrace_struct_size_value \
                                             - 1   /* trailing null character */ \
                                             + 2;} /* quotation marks (for value) */

#  define LIBIOTRACE_STRUCT_ARRAY_BITFIELD_START(name)
#  define LIBIOTRACE_STRUCT_ARRAY_BITFIELD_ELEMENT(name)
#  define LIBIOTRACE_STRUCT_ARRAY_BITFIELD_END

#  define LIBIOTRACE_STRUCT_VOID_P_START(name) libiotrace_struct_size_void_p = 0;
#  define LIBIOTRACE_STRUCT_VOID_P_ELEMENT(name, element) libiotrace_struct_size_void_p_tmp = libiotrace_struct_push_max_size_##element(sizeof(#name) + prefix_length + 1 /*underscore*/); \
                                                    if(libiotrace_struct_size_void_p_tmp > libiotrace_struct_size_void_p) \
                                                      libiotrace_struct_size_void_p = libiotrace_struct_size_void_p_tmp;
#  define LIBIOTRACE_STRUCT_VOID_P_END(name) libiotrace_struct_size += libiotrace_struct_size_void_p;

// TODO: remove ATTRIBUTE_UNUSED from parameters after all data types are implemented
#  define LIBIOTRACE_STRUCT_START(name) size_t libiotrace_struct_push_max_size_##name(size_t prefix_length ATTRIBUTE_UNUSED) { /* prefix e.g. file_type */ \
                                    size_t libiotrace_struct_size_void_p ATTRIBUTE_UNUSED; \
                                    size_t libiotrace_struct_size_void_p_tmp ATTRIBUTE_UNUSED; \
                                    size_t libiotrace_struct_size = 0; /* start with 0 -- no parentheses like json */
#  define LIBIOTRACE_STRUCT_END return libiotrace_struct_size;}

#  define LIBIOTRACE_STRUCT_STRUCT_ARRAY(type, name, max_length)

#  define LIBIOTRACE_STRUCT_STRUCT_P(type, name) libiotrace_struct_size += libiotrace_struct_push_max_size_##type(sizeof(#name) + prefix_length + 1 /*underscore*/);
#  define LIBIOTRACE_STRUCT_STRUCT(type, name) libiotrace_struct_size += libiotrace_struct_push_max_size_##type(sizeof(#name) + prefix_length + 1 /*underscore*/);
#  define LIBIOTRACE_STRUCT_ARRAY_BITFIELD(type, name)
#  define LIBIOTRACE_STRUCT_ENUM(type, name) LIBIOTRACE_STRUCT_ELEMENT_SIZE(name, libiotrace_struct_push_max_size_enum_##type())
#  define LIBIOTRACE_STRUCT_INT(name) LIBIOTRACE_STRUCT_ELEMENT_SIZE(name, LIBIOTRACE_STRUCT_TYPE_SIZE_DEC(int) \
                                                                           + 1  /* for sign (-) */ \
                                                                           + 1) /* for integer postfix (i) */
#  define LIBIOTRACE_STRUCT_CHAR(name) LIBIOTRACE_STRUCT_ELEMENT_SIZE(name, LIBIOTRACE_STRUCT_TYPE_SIZE_DEC(char) \
                                                                            + 1  /* for sign (-) */ \
                                                                            + 1) /* for integer postfix (i) */
#  define LIBIOTRACE_STRUCT_PID_T(name) LIBIOTRACE_STRUCT_ELEMENT_SIZE(name, LIBIOTRACE_STRUCT_TYPE_SIZE_DEC(pid_t) \
                                                                             + 1  /* for sign (-) */ \
                                                                             + 1) /* for integer postfix (i) */
#  define LIBIOTRACE_STRUCT_CSTRING(name, length) LIBIOTRACE_STRUCT_ELEMENT_SIZE(name, length + 2) /* +2 for "" in strings --> Influx 2.X */
#  define LIBIOTRACE_STRUCT_CSTRING_P(name, max_length) LIBIOTRACE_STRUCT_CSTRING(name, max_length)
#  define LIBIOTRACE_STRUCT_CSTRING_P_CONST(name, max_length) LIBIOTRACE_STRUCT_CSTRING(name, max_length)
#  define LIBIOTRACE_STRUCT_CLOCK_T(name) LIBIOTRACE_STRUCT_ELEMENT_SIZE(name, LIBIOTRACE_STRUCT_TYPE_SIZE_DEC(clock_t) \
                                                                               + 1) /* for unsigned integer postfix (u) */
#  define LIBIOTRACE_STRUCT_FILE_P(name) LIBIOTRACE_STRUCT_ELEMENT_SIZE(name, LIBIOTRACE_STRUCT_TYPE_SIZE_HEX(FILE*) \
                                                                              + 2) /* quotation marks (for value) */
#  define LIBIOTRACE_STRUCT_LONG_INT(name) LIBIOTRACE_STRUCT_ELEMENT_SIZE(name, LIBIOTRACE_STRUCT_TYPE_SIZE_DEC(long int) \
                                                                                + 1  /* for sign (-) */ \
                                                                                + 1) /* for integer postfix (i) */
#  define LIBIOTRACE_STRUCT_SIZE_T(name) LIBIOTRACE_STRUCT_ELEMENT_SIZE(name, LIBIOTRACE_STRUCT_TYPE_SIZE_DEC(size_t) \
                                                                              + 1) /* for unsigned integer postfix (u) */
#  define LIBIOTRACE_STRUCT_SSIZE_T(name) LIBIOTRACE_STRUCT_ELEMENT_SIZE(name, LIBIOTRACE_STRUCT_TYPE_SIZE_DEC(ssize_t) \
                                                                               + 1  /* for sign (-) */ \
                                                                               + 1) /* for integer postfix (i) */
#  ifdef HAVE_OFF64_T
#    define LIBIOTRACE_STRUCT_OFF_T(name) LIBIOTRACE_STRUCT_ELEMENT_SIZE(name, LIBIOTRACE_STRUCT_TYPE_SIZE_DEC(off64_t) \
                                                                               + 1  /* for sign (-) */ \
                                                                               + 1) /* for integer postfix (i) */
#  else
#    define LIBIOTRACE_STRUCT_OFF_T(name) LIBIOTRACE_STRUCT_ELEMENT_SIZE(name, LIBIOTRACE_STRUCT_TYPE_SIZE_DEC(off_t) \
                                                                               + 1  /* for sign (-) */ \
                                                                               + 1) /* for integer postfix (i) */
#  endif
#  define LIBIOTRACE_STRUCT_U_INT64_T(name) LIBIOTRACE_STRUCT_ELEMENT_SIZE(name, LIBIOTRACE_STRUCT_TYPE_SIZE_DEC(u_int64_t) \
                                                                                 + 1) /* for unsigned integer postfix (u) */
#  define LIBIOTRACE_STRUCT_VOID_P(name) LIBIOTRACE_STRUCT_ELEMENT_SIZE(name, LIBIOTRACE_STRUCT_TYPE_SIZE_HEX(void*) \
                                                                              + 2) /* quotation marks (for value) */
#  define LIBIOTRACE_STRUCT_VOID_P_CONST(name) LIBIOTRACE_STRUCT_ELEMENT_SIZE(name, LIBIOTRACE_STRUCT_TYPE_SIZE_HEX(void*) \
                                                                                    + 2) /* quotation marks (for value) */
#  define LIBIOTRACE_STRUCT_FD_SET_P(name)
#  if defined(HAVE_DLMOPEN) && defined(WITH_DL_IO)
#    define LIBIOTRACE_STRUCT_LMID_T(name) LIBIOTRACE_STRUCT_ELEMENT_SIZE(name, LIBIOTRACE_STRUCT_TYPE_SIZE_DEC(Lmid_t) \
                                                                                + 1  /* for sign (-) */ \
                                                                                + 1) /* for integer postfix (i) */
#  endif
#  define LIBIOTRACE_STRUCT_SHORT(name) LIBIOTRACE_STRUCT_ELEMENT_SIZE(name, LIBIOTRACE_STRUCT_TYPE_SIZE_DEC(short) \
                                                                             + 1  /* for sign (-) */ \
                                                                             + 1) /* for integer postfix (i) */
#  define LIBIOTRACE_STRUCT_DEV_T(name) LIBIOTRACE_STRUCT_ELEMENT_SIZE(name, LIBIOTRACE_STRUCT_TYPE_SIZE_DEC(dev_t) \
                                                                             + 1) /* for unsigned integer postfix (u) */
#  define LIBIOTRACE_STRUCT_INO_T(name) LIBIOTRACE_STRUCT_ELEMENT_SIZE(name, LIBIOTRACE_STRUCT_TYPE_SIZE_DEC(ino_t) \
                                                                             + 1) /* for unsigned integer postfix (u) */
#  define LIBIOTRACE_STRUCT_MALLOC_STRING_ARRAY(name, max_size, max_length_per_element)
#  define LIBIOTRACE_STRUCT_MALLOC_PTR_ARRAY(name, max_size)
#  define LIBIOTRACE_STRUCT_INT_ARRAY(name, max_size)
#  define LIBIOTRACE_STRUCT_SA_FAMILY_T(name) LIBIOTRACE_STRUCT_ELEMENT_SIZE(name, LIBIOTRACE_STRUCT_TYPE_SIZE_DEC(sa_family_t) \
                                                                                   + 1) /* for unsigned integer postfix (u) */
#  define LIBIOTRACE_STRUCT_KEY_VALUE_ARRAY(name, max_size, max_length_per_cstring)

/* insert new line for new data-type here */
/* ----------------------------------------------------------------------------------------------------------------------- */
#elif LIBIOTRACE_STRUCT == LIBIOTRACE_STRUCT_PUSH
/*
 * Macros for generating HTTP Posts
 *
 * following functions are available after include of the macros
 *
 * size_t libiotrace_struct_push_<name of struct>(void* libiotrace_struct_buffer_to_post, size_t libiotrace_struct_length_of_buffer_to_post, struct name *libiotrace_struct_data);
 *
 * */
#  define LIBIOTRACE_STRUCT_TYPE(name, function) if(*prefix=='\0') { \
                                               LIBIOTRACE_STRUCT_WRITE(#name"=") \
                                           } else { \
                                               LIBIOTRACE_STRUCT_SNPRINTF("%s_"#name"=", prefix) \
                                           } \
                                           libiotrace_struct_ret = function(libiotrace_struct_buf, libiotrace_struct_size, \
                                                               &libiotrace_struct_data->name); \
                                           libiotrace_struct_buf += libiotrace_struct_ret;  /* set pointer to end of written characters */ \
                                           libiotrace_struct_size -= libiotrace_struct_ret; /* resize buffer size */
#  define LIBIOTRACE_STRUCT_ELEMENT(key, template, ...) if(*prefix=='\0') { \
                                                      LIBIOTRACE_STRUCT_SNPRINTF(#key"="#template",", __VA_ARGS__) \
                                                  } else { \
                                                      LIBIOTRACE_STRUCT_SNPRINTF("%s_"#key"="#template",", prefix, __VA_ARGS__) \
                                                  }

#  define LIBIOTRACE_STRUCT_ENUM_START(name) size_t libiotrace_struct_push_enum_##name(char* libiotrace_struct_buf, \
                                             size_t libiotrace_struct_size, enum name *libiotrace_struct_data) { \
                                         size_t libiotrace_struct_ret = 0; \
                                         size_t libiotrace_struct_start_size = libiotrace_struct_size; \
                                         switch (*libiotrace_struct_data) {
#  define LIBIOTRACE_STRUCT_ENUM_ELEMENT(name) case name: \
                                           LIBIOTRACE_STRUCT_WRITE(LIBIOTRACE_STRUCT_QUOT(name)",") \
                                           break;
#  define LIBIOTRACE_STRUCT_ENUM_END default: \
                                 LIBIOTRACE_STRUCT_ENUM_ERROR(*libiotrace_struct_data) \
                               } return libiotrace_struct_start_size - libiotrace_struct_size;}

#  define LIBIOTRACE_STRUCT_ARRAY_BITFIELD_START(name)
#  define LIBIOTRACE_STRUCT_ARRAY_BITFIELD_ELEMENT(name)
#  define LIBIOTRACE_STRUCT_ARRAY_BITFIELD_END

#  define LIBIOTRACE_STRUCT_VOID_P_START(name) if (NULL != libiotrace_struct_data->__##name) { \
                                            char prefix_new [strlen(prefix) + sizeof(#name) +1]; /* +1: underscore between prefix and name */\
                                            if(*prefix == '\0') { \
                                              snprintf(prefix_new, sizeof(prefix_new),"%s", #name);\
                                            } else { \
                                              snprintf(prefix_new, sizeof(prefix_new),"%s_%s", prefix, #name);\
                                            }\
                                            switch (libiotrace_struct_data->__void_p_enum_##name) {
#  define LIBIOTRACE_STRUCT_VOID_P_ELEMENT(name, element) case __void_p_enum_##name##_##element: \
                                                      libiotrace_struct_ret = libiotrace_struct_push_##element(libiotrace_struct_buf, \
                                                                          libiotrace_struct_size, \
                                                                          (struct element*) libiotrace_struct_data->__##name, prefix_new); \
                                                      libiotrace_struct_buf += libiotrace_struct_ret;  /* set pointer to end of written characters */ \
                                                      libiotrace_struct_size -= libiotrace_struct_ret; /* resize buffer size */\
                                                      break;
#  define LIBIOTRACE_STRUCT_VOID_P_END(name)   default: \
                                           LIBIOTRACE_STRUCT_ENUM_ERROR(libiotrace_struct_data->__void_p_enum_##name) \
                                         } \
                                       }

// TODO: remove ATTRIBUTE_UNUSED from parameters after all data types are implemented
#  define LIBIOTRACE_STRUCT_START(name) size_t libiotrace_struct_push_##name(char* libiotrace_struct_buf ATTRIBUTE_UNUSED, size_t libiotrace_struct_size, \
                                        struct name *libiotrace_struct_data ATTRIBUTE_UNUSED, const char* prefix ATTRIBUTE_UNUSED) { \
                                    size_t libiotrace_struct_ret ATTRIBUTE_UNUSED = 0; \
                                    int libiotrace_struct_ret_int ATTRIBUTE_UNUSED = 0; \
                                    size_t libiotrace_struct_start_size = libiotrace_struct_size;
#  define LIBIOTRACE_STRUCT_END return libiotrace_struct_start_size - libiotrace_struct_size;}

#  define LIBIOTRACE_STRUCT_STRUCT_ARRAY(type, name, max_length)

#  define LIBIOTRACE_STRUCT_STRUCT_P(type, name) if (NULL != libiotrace_struct_data->name) { \
                                           char prefix_new_##name [strlen(prefix) + sizeof(#name) +1]; /* +1: underscore between prefix and name */ \
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
#  define LIBIOTRACE_STRUCT_STRUCT(type, name) char prefix_new_##name [strlen(prefix) + sizeof(#name) +1]; /* +1: underscore between prefix and name */\
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
#  define LIBIOTRACE_STRUCT_ENUM(type, name) LIBIOTRACE_STRUCT_TYPE(name, libiotrace_struct_push_enum_##type)
#  define LIBIOTRACE_STRUCT_INT(name) LIBIOTRACE_STRUCT_ELEMENT(name, %di, libiotrace_struct_data->name)
#  define LIBIOTRACE_STRUCT_CHAR(name) LIBIOTRACE_STRUCT_ELEMENT(name, %di, libiotrace_struct_data->name)
#  define LIBIOTRACE_STRUCT_PID_T(name) LIBIOTRACE_STRUCT_ELEMENT(name, %di, libiotrace_struct_data->name)
#  define LIBIOTRACE_STRUCT_CSTRING(name, length) LIBIOTRACE_STRUCT_ELEMENT(name, "%s", libiotrace_struct_data->name)
#  define LIBIOTRACE_STRUCT_CSTRING_P(name, max_length) LIBIOTRACE_STRUCT_ELEMENT(name, "%s", libiotrace_struct_data->name)
#  define LIBIOTRACE_STRUCT_CSTRING_P_CONST(name, max_length) LIBIOTRACE_STRUCT_ELEMENT(name, "%s", libiotrace_struct_data->name)
#  define LIBIOTRACE_STRUCT_CLOCK_T(name) LIBIOTRACE_STRUCT_ELEMENT(name, %luu, libiotrace_struct_data->name)
#  define LIBIOTRACE_STRUCT_FILE_P(name) LIBIOTRACE_STRUCT_ELEMENT(name, "%p", (void *)libiotrace_struct_data->name)
#  define LIBIOTRACE_STRUCT_LONG_INT(name) LIBIOTRACE_STRUCT_ELEMENT(name, %ldi, libiotrace_struct_data->name)
#  define LIBIOTRACE_STRUCT_SIZE_T(name) LIBIOTRACE_STRUCT_ELEMENT(name, %luu, libiotrace_struct_data->name)
#  define LIBIOTRACE_STRUCT_SSIZE_T(name) LIBIOTRACE_STRUCT_ELEMENT(name, %ldi, libiotrace_struct_data->name)
#  define LIBIOTRACE_STRUCT_OFF_T(name) LIBIOTRACE_STRUCT_ELEMENT(name, %ldi, libiotrace_struct_data->name)
#  define LIBIOTRACE_STRUCT_U_INT64_T(name) LIBIOTRACE_STRUCT_ELEMENT(name, %luu, libiotrace_struct_data->name) /*mpi_file->written_bytes*/
#  define LIBIOTRACE_STRUCT_VOID_P(name) LIBIOTRACE_STRUCT_ELEMENT(name, "%p", libiotrace_struct_data->name)
#  define LIBIOTRACE_STRUCT_VOID_P_CONST(name) LIBIOTRACE_STRUCT_ELEMENT(name, "%p", libiotrace_struct_data->name)
#  define LIBIOTRACE_STRUCT_FD_SET_P(name)
#  if defined(HAVE_DLMOPEN) && defined(WITH_DL_IO)
#    define LIBIOTRACE_STRUCT_LMID_T(name) LIBIOTRACE_STRUCT_ELEMENT(name, %ldi, libiotrace_struct_data->name)
#  endif
#  define LIBIOTRACE_STRUCT_SHORT(name) LIBIOTRACE_STRUCT_ELEMENT(name, %di, libiotrace_struct_data->name)
#  define LIBIOTRACE_STRUCT_DEV_T(name) LIBIOTRACE_STRUCT_ELEMENT(name, %luu, libiotrace_struct_data->name)
#  define LIBIOTRACE_STRUCT_INO_T(name) LIBIOTRACE_STRUCT_ELEMENT(name, %luu, libiotrace_struct_data->name)
#  define LIBIOTRACE_STRUCT_MALLOC_STRING_ARRAY(name, max_size, max_length_per_element)
#  define LIBIOTRACE_STRUCT_MALLOC_PTR_ARRAY(name, max_size)
#  define LIBIOTRACE_STRUCT_INT_ARRAY(name, max_size)
#  define LIBIOTRACE_STRUCT_SA_FAMILY_T(name) LIBIOTRACE_STRUCT_ELEMENT(name, %huu, libiotrace_struct_data->name)
#  define LIBIOTRACE_STRUCT_KEY_VALUE_ARRAY(name, max_size, max_length_per_cstring)

/* insert new line for new data-type here */
/* ----------------------------------------------------------------------------------------------------------------------- */
#else
/* ----------------------------------------------------------------------------------------------------------------------- */
#  error "Undefined LIBIOTRACE_STRUCT value."
#endif

#endif
