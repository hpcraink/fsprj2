#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <limits.h>
#include <assert.h>
#include <math.h>

/*
 * To add a new data-type for generating the struct and the json-cstring seven lines
 * have to be added. The positions for adding a new data-type are marked with:
 * insert new line for new data-type here
 */

/* values for #define JSON_STRUCT */
#define JSON_STRUCT_DATA_TYPE   1 /* generate struct */
#define JSON_STRUCT_SET_VOID_P  2 /* generate function to set VOID_P element */
#define JSON_STRUCT_PRINT       3 /* generate print-function to print struct as json */
#define JSON_STRUCT_BYTES_COUNT 4 /* generate function to evaluate max size of json-string */
#define JSON_STRUCT_SIZEOF      5 /* generate function to evaluate size for JSON_STRUCT_COPY */
#define JSON_STRUCT_COPY        6 /* generate function to deep copy struct (with VOID_P elements) */

/**
 * #define's for escaping cstrings (used in print_cstring()-function)
 */
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

#undef JSON_STRUCT_STRUCT
#undef JSON_STRUCT_ARRAY_BITFIELD
#undef JSON_STRUCT_ENUM
#undef JSON_STRUCT_INT
#undef JSON_STRUCT_PID_T
#undef JSON_STRUCT_CSTRING
#undef JSON_STRUCT_CSTRING_P
#undef JSON_STRUCT_CLOCK_T
#undef JSON_STRUCT_FILE_P
#undef JSON_STRUCT_ENUM_START
/* insert new line for new data-type here */

#if JSON_STRUCT == JSON_STRUCT_DATA_TYPE

#  define JSON_STRUCT_ENUM_START(name) enum name {
#  define JSON_STRUCT_ENUM_ELEMENT(name) name,
#  define JSON_STRUCT_ENUM_END };

#  define JSON_STRUCT_ARRAY_BITFIELD_START(name) struct name {
#  define JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(name) unsigned int name : 1;
#  define JSON_STRUCT_ARRAY_BITFIELD_END };

#  define JSON_STRUCT_VOID_P_START(name) void *name; enum {
#  define JSON_STRUCT_VOID_P_ELEMENT(name, element) element,
#  define JSON_STRUCT_VOID_P_END(name) } void_p_enum_##name;

#  define JSON_STRUCT_START(name) struct name {
#  define JSON_STRUCT_END };

#  define JSON_STRUCT_STRUCT(type, name) struct type name;
#  define JSON_STRUCT_ARRAY_BITFIELD(type, name) struct type name;
#  define JSON_STRUCT_ENUM(type, name) enum type name;
#  define JSON_STRUCT_INT(name) int name;
#  define JSON_STRUCT_PID_T(name) pid_t name;
#  define JSON_STRUCT_CSTRING(name, length) char name[length];
#  define JSON_STRUCT_CSTRING_P(name, max_length) char *name;
#  define JSON_STRUCT_CLOCK_T(name) clock_t name;
#  define JSON_STRUCT_FILE_P(name) FILE* name;
/* insert new line for new data-type here */

#elif JSON_STRUCT == JSON_STRUCT_SET_VOID_P

#  define JSON_STRUCT_ENUM_START(name)
#  define JSON_STRUCT_ENUM_ELEMENT(name)
#  define JSON_STRUCT_ENUM_END

#  define JSON_STRUCT_ARRAY_BITFIELD_START(name)
#  define JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(name)
#  define JSON_STRUCT_ARRAY_BITFIELD_END

#  define JSON_STRUCT_VOID_P_START(name)/* ToDo: set-functions for void pointer elements */
#  define JSON_STRUCT_VOID_P_ELEMENT(name, element) //void set_##name(struct name data, struct element value) {\
                                                    //   \
                                                    //  }
#  define JSON_STRUCT_VOID_P_END(name)

#  define JSON_STRUCT_START(name)
#  define JSON_STRUCT_END

#  define JSON_STRUCT_STRUCT(type, name)
#  define JSON_STRUCT_ARRAY_BITFIELD(type, name)
#  define JSON_STRUCT_ENUM(type, name)
#  define JSON_STRUCT_INT(name)
#  define JSON_STRUCT_PID_T(name)
#  define JSON_STRUCT_CSTRING(name, length)
#  define JSON_STRUCT_CSTRING_P(name, max_length)
#  define JSON_STRUCT_CLOCK_T(name)
#  define JSON_STRUCT_FILE_P(name)
/* insert new line for new data-type here */

#elif JSON_STRUCT == JSON_STRUCT_PRINT

/*
 * generate functions
 *
 * int print_<name of struct>(char* buf, size_t size, struct name data)
 * int print_array_<name of array>(char* buf, size_t size, struct name data)
 * int print_enum_<name of enum>(char* buf, size_t size, struct name data)
 *
 * for printing struct as json-cstring. The size argument specifies the maximum number
 * of characters to produce.
 * */

#define JSON_STRUCT_ESCAPE_PRINT_2BYTES(character) if (size - ret > 2) { \
                                                     *buf++ = '\\'; \
                                                     *buf++ = character; \
                                                     src++; \
                                                     ret += 2; \
                                                   } else { \
                                                     size = 0; /* buffer not big enough: break while */ \
                                                   }

int print_cstring(char* buf, size_t size, char* src) {
    const char *hex = "0123456789abcdef";
    int ret = 0;

    if (size > 2) {
        *buf++ = '\"';
        ret++;
    } else {
        return ret;
    }

    while (*src != '\0' && size - ret > 2) {
        if ((unsigned char)*src >= ' ') {
            switch (*src) {
            case '\"':
            case '\\':
#if JSON_STRUCT_ESCAPE_SLASH
            case '/':
#endif
                JSON_STRUCT_ESCAPE_PRINT_2BYTES(*src)
                break;
            default:
                *buf++ = *src;
                src++;
                ret++;
            }
        } else {
            switch (*src) {
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
                if (size - ret > 7) {
                    *buf++ = '\\';
                    *buf++ = 'u';
                    *buf++ = '0';
                    *buf++ = '0';
                    *buf++ = hex[((unsigned char)*src >> 4)];
                    *buf++ = hex[(*src) & 0x0f];
                    src++;
                    ret += 6;
                } else {
                    size = 0; /* buffer not big enough: break while */ \
                }
            }
        }
    }

    *buf++ = '\"';
    ret++;
    *buf = '\0';
    return ret;
}

#  define SNPRINTF(...) ret = snprintf(buf, size, __VA_ARGS__); \
                        assert(ret < size); /* don't write more characters then size of buffer */ \
                                            /* ToDo: fail with error-message */ \
                        buf += ret;  /* set pointer to end of written characters */ \
                        size -= ret; /* resize buffer size */

#  define JSON_STRUCT_QUOT(key) "\""#key"\""

#  define JSON_STRUCT_TYPE(name, function) hasElements = 1; \
                                           SNPRINTF(JSON_STRUCT_QUOT(name)":") \
                                           ret = function(buf, size, data.name); \
                                           buf += ret;  /* set pointer to end of written characters */ \
                                           size -= ret; /* resize buffer size */
#  define JSON_STRUCT_ELEMENT(key, template, ...) hasElements = 1; \
                                                  SNPRINTF(JSON_STRUCT_QUOT(key)":"#template",", __VA_ARGS__)
#  define JSON_STRUCT_ESCAPE(name) hasElements = 1; \
                                   SNPRINTF(JSON_STRUCT_QUOT(name)":") \
                                   ret = print_cstring(buf, size, data.name); \
                                   buf += ret;  /* set pointer to end of written characters */ \
                                   size -= ret; /* resize buffer size */ \
                                   SNPRINTF(",")

#  define JSON_STRUCT_ENUM_START(name) int print_enum_##name(char* buf, size_t size, enum name data) { \
                                         int ret = 0; \
                                         int start_size = size; \
                                         switch (data) {
#  define JSON_STRUCT_ENUM_ELEMENT(name) case name: \
                                           SNPRINTF(JSON_STRUCT_QUOT(name)",") \
                                           break;
#  define JSON_STRUCT_ENUM_END default: \
                                 assert(0); /* ToDo: fail with error-message or print error to json (max size of json?) */ \
                               } return start_size - size;}

#  define JSON_STRUCT_ARRAY_BITFIELD_START(name) int print_array_##name(char* buf, size_t size, struct name data) { \
                                                   int hasElements = 0; \
                                                   int ret = 0; \
                                                   int start_size = size; \
                                                   SNPRINTF("[")
#  define JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(name) if (data.name) { \
                                                     hasElements = 1; \
                                                     SNPRINTF(JSON_STRUCT_QUOT(name)",") \
                                                   }
#  define JSON_STRUCT_ARRAY_BITFIELD_END if (hasElements) { \
                                           buf--;  /* remove last comma */ \
                                           size++; /* and resize buffer size */ \
                                         } \
                                         SNPRINTF("],") \
                                         return start_size - size;}

#  define JSON_STRUCT_START(name) int print_##name(char* buf, size_t size, struct name data) { \
                                    int hasElements = 0; \
                                    int ret = 0; \
                                    int start_size = size; \
                                    SNPRINTF("{")
#  define JSON_STRUCT_END if (hasElements) { \
                            buf--;  /* remove last comma */ \
                            size++; /* and resize buffer size */ \
                          } \
                          SNPRINTF("}") \
                          return start_size - size;}

#  define JSON_STRUCT_VOID_P_START(name) hasElements = 1; \
                                         SNPRINTF(JSON_STRUCT_QUOT(name)":") \
                                         switch (data.void_p_enum_##name) {
#  define JSON_STRUCT_VOID_P_ELEMENT(name, element) case element: \
                                                      ret = print_##element(buf, size, *((struct element*) data.name)); \
                                                      buf += ret;  /* set pointer to end of written characters */ \
                                                      size -= ret; /* resize buffer size */\
                                                      break;
#  define JSON_STRUCT_VOID_P_END(name) default: \
                                         assert(0); /* ToDo: fail with error-message or print error to json (max size of json?) */ \
                                       } \
                                       SNPRINTF(",")

#  define JSON_STRUCT_STRUCT(type, name) JSON_STRUCT_TYPE(name, print_##type) \
                                         SNPRINTF(",")
#  define JSON_STRUCT_ARRAY_BITFIELD(type, name) JSON_STRUCT_TYPE(name, print_array_##type)
#  define JSON_STRUCT_ENUM(type, name) JSON_STRUCT_TYPE(name, print_enum_##type)
#  define JSON_STRUCT_INT(name) JSON_STRUCT_ELEMENT(name, %d, data.name)
#  define JSON_STRUCT_PID_T(name) JSON_STRUCT_ELEMENT(name, %u, data.name)
#  define JSON_STRUCT_CSTRING(name, length) JSON_STRUCT_ESCAPE(name)
#  define JSON_STRUCT_CSTRING_P(name, max_length) JSON_STRUCT_ESCAPE(name)
#  define JSON_STRUCT_CLOCK_T(name) JSON_STRUCT_ELEMENT(name, %lu, data.name)
#  define JSON_STRUCT_FILE_P(name) JSON_STRUCT_ELEMENT(name, "%p", data.name)
/* insert new line for new data-type here */

#elif JSON_STRUCT == JSON_STRUCT_BYTES_COUNT

/*
 * generate functions
 *
 * int max_size_<name of struct>()
 * int max_size_array_<name of struct>()
 * int max_size_enum_<name of struct>()
 *
 * for evaluating max size of json-string. The returned size is without trailing
 * null character.
 * */

#  define JSON_STRUCT_TYPE_SIZE_DEC(type) ceil(log10(pow(2, sizeof(type) * CHAR_BIT)))
#  define JSON_STRUCT_TYPE_SIZE_HEX(type) ((sizeof(type) * 2) + 2) /* +2 for "0x"-prefix */
#  define JSON_STRUCT_ELEMENT_SIZE(name, sizeValue) hasElements = 1; \
                                                    size += sizeof(#name) \
                                                            - 1  /* trailing null character */ \
                                                            + sizeValue \
                                                            + 4; /* quotation marks (for key), colon and comma */

#  define JSON_STRUCT_ENUM_START(name) int max_size_enum_##name() { \
                                         size_t size_value = 0;
#  define JSON_STRUCT_ENUM_ELEMENT(name) if (sizeof(#name) > size_value) \
                                           size_value = sizeof(#name); /* get greatest possible value */
#  define JSON_STRUCT_ENUM_END return size_value \
                                      - 1   /* trailing null character */ \
                                      + 2;} /* quotation marks (for value) */

#  define JSON_STRUCT_ARRAY_BITFIELD_START(name) int max_size_array_##name() { \
                                                   int hasElements = 0; \
                                                   size_t size = 1; /* open parentheses */
#  define JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(name) hasElements = 1; \
                                                   size += sizeof(#name) /* add each possible value */ \
                                                           - 1  /* trailing null character */ \
                                                           + 3; /* quotation marks and comma */
#  define JSON_STRUCT_ARRAY_BITFIELD_END if (hasElements) size--; /* remove last comma */ \
                                         return size + 1;} /* close parentheses */

#  define JSON_STRUCT_START(name) int max_size_##name() { \
                                    int hasElements = 0; \
                                    size_t size = 1; /* open parentheses */
#  define JSON_STRUCT_END if (hasElements) size--; /* remove last comma */ \
                          size++; /* close parentheses */ \
                          return size;}

#  define JSON_STRUCT_VOID_P_START(name) hasElements = 1; \
                                         size += sizeof(#name) \
                                                 - 1  /* trailing null character */ \
                                                 + 4; /* quotation marks (for key), colon and comma */ \
                                         int size_##name = 0; \
                                         int size_tmp_##name = 0;
#  define JSON_STRUCT_VOID_P_ELEMENT(name, element) size_tmp_##name = max_size_##element(); \
                                                    if(size_tmp_##name > size_##name) size_##name = size_tmp_##name;
#  define JSON_STRUCT_VOID_P_END(name) size += size_##name;

#  define JSON_STRUCT_STRUCT(type, name) JSON_STRUCT_ELEMENT_SIZE(name, max_size_##type())
#  define JSON_STRUCT_ARRAY_BITFIELD(type, name) JSON_STRUCT_ELEMENT_SIZE(name, max_size_array_##type())
#  define JSON_STRUCT_ENUM(type, name) JSON_STRUCT_ELEMENT_SIZE(name, max_size_enum_##type())
#  define JSON_STRUCT_INT(name) JSON_STRUCT_ELEMENT_SIZE(name, JSON_STRUCT_TYPE_SIZE_DEC(int) \
                                                                + 1) /* for sign (-) */
#  define JSON_STRUCT_PID_T(name) JSON_STRUCT_ELEMENT_SIZE(name, JSON_STRUCT_TYPE_SIZE_DEC(pid_t))
#  define JSON_STRUCT_CSTRING(name,length) JSON_STRUCT_ELEMENT_SIZE(name, (length - 1) /* -1 trailing null character */ \
                                                                          * 6  /* *6 for escaping (\u00ff) */ \
                                                                          + 2) /* +2 quotation marks (for value) */
#  define JSON_STRUCT_CSTRING_P(name, max_length) JSON_STRUCT_CSTRING(name, max_length)
#  define JSON_STRUCT_CLOCK_T(name) JSON_STRUCT_ELEMENT_SIZE(name, JSON_STRUCT_TYPE_SIZE_DEC(clock_t))
#  define JSON_STRUCT_FILE_P(name) JSON_STRUCT_ELEMENT_SIZE(name, JSON_STRUCT_TYPE_SIZE_HEX(FILE*) \
                                                                   + 2) /* quotation marks (for value) */
/* insert new line for new data-type here */

#elif JSON_STRUCT == JSON_STRUCT_SIZEOF

#  define JSON_STRUCT_ENUM_START(name)
#  define JSON_STRUCT_ENUM_ELEMENT(name)
#  define JSON_STRUCT_ENUM_END

#  define JSON_STRUCT_ARRAY_BITFIELD_START(name)
#  define JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(name)
#  define JSON_STRUCT_ARRAY_BITFIELD_END

#  define JSON_STRUCT_VOID_P_START(name) switch (data.void_p_enum_##name) {
#  define JSON_STRUCT_VOID_P_ELEMENT(name, element) case element: \
                                                      size += sizeof_##element(*((struct element*) data.name)); \
                                                      break;
#  define JSON_STRUCT_VOID_P_END(name) }

#  define JSON_STRUCT_START(name) int sizeof_##name(struct name data) { \
                                    size_t size = sizeof(struct name);
#  define JSON_STRUCT_END return size;}

#  define JSON_STRUCT_STRUCT(type, name)
#  define JSON_STRUCT_ARRAY_BITFIELD(type, name)
#  define JSON_STRUCT_ENUM(type, name)
#  define JSON_STRUCT_INT(name)
#  define JSON_STRUCT_PID_T(name)
#  define JSON_STRUCT_CSTRING(name, length)
#  define JSON_STRUCT_CSTRING_P(name, max_length) size += strnlen(data.name, max_length) + 1; /* +1 for trailing null character */
#  define JSON_STRUCT_CLOCK_T(name)
#  define JSON_STRUCT_FILE_P(name)
/* insert new line for new data-type here */

#elif JSON_STRUCT == JSON_STRUCT_COPY

int copy_cstring_p(char *to, char *from, size_t max_length) {
    int i;
    for (i = 1; i < max_length && *from != '\0'; i++) {
        *to = *from;
        to++;
        from++;
    }
    if (i == max_length && *from != '\0') {
        /* it's ok to write on (to + max_length), because expected size in
         * sizeof_##name(struct name data) is +1 for trailing null character */
        *to = *from;
        to++;
    }
    *to = '\0';

    return i;
}

#  define JSON_STRUCT_ENUM_START(name)
#  define JSON_STRUCT_ENUM_ELEMENT(name)
#  define JSON_STRUCT_ENUM_END

#  define JSON_STRUCT_ARRAY_BITFIELD_START(name)
#  define JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(name)
#  define JSON_STRUCT_ARRAY_BITFIELD_END

#  define JSON_STRUCT_VOID_P_START(name) switch (data.void_p_enum_##name) {
#  define JSON_STRUCT_VOID_P_ELEMENT(name, element) case element: \
                                                      copy->name = buf; \
                                                      buf = copy_##element(buf, *((struct element*) data.name)); \
                                                      break;
#  define JSON_STRUCT_VOID_P_END(name) }

#  define JSON_STRUCT_START(name) void* copy_##name(void *buf, struct name data) { \
                                    struct name *copy = (struct name *)buf; \
                                    int ret; \
                                    memcpy(buf, (void *)(&data), sizeof(struct name)); \
                                    buf += sizeof(struct name);
#  define JSON_STRUCT_END return buf;}

#  define JSON_STRUCT_STRUCT(type, name)
#  define JSON_STRUCT_ARRAY_BITFIELD(type, name)
#  define JSON_STRUCT_ENUM(type, name)
#  define JSON_STRUCT_INT(name)
#  define JSON_STRUCT_PID_T(name)
#  define JSON_STRUCT_CSTRING(name, length)
#  define JSON_STRUCT_CSTRING_P(name, max_length) ret = copy_cstring_p((char *)buf, data.name, max_length); \
                                                  copy->name = (char *)buf; \
                                                  buf += ret;
#  define JSON_STRUCT_CLOCK_T(name)
#  define JSON_STRUCT_FILE_P(name)
/* insert new line for new data-type here */

#else
#  error "Undefined JSON_STRUCT value."
#endif

#endif
