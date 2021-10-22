#include "libiotrace_defines_utils.h"

/* #define's for escaping cstrings (used in print_cstring()-function) */
#ifndef LIBIOTRACE_STRUCT_ESCAPE_SLASH
#  define LIBIOTRACE_STRUCT_ESCAPE_SLASH 0
#endif

#define LIBIOTRACE_STRUCT_ESCAPE_PRINT_2BYTES(character) if (libiotrace_struct_size - libiotrace_struct_ret > 2) { \
                                                       *libiotrace_struct_buf++ = '\\'; \
                                                       *libiotrace_struct_buf++ = character; \
                                                       libiotrace_struct_src++; \
                                                       libiotrace_struct_ret += 2; \
                                                   } else { \
                                                       libiotrace_struct_size = 0; /* buffer not big enough: break while */ \
                                                   }

size_t libiotrace_struct_print_cstring(char* libiotrace_struct_buf, size_t libiotrace_struct_size, const char* libiotrace_struct_src) {
    const char *libiotrace_struct_hex = "0123456789abcdef";
    size_t libiotrace_struct_ret = 0;

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

size_t libiotrace_struct_write(char* libiotrace_struct_buf, size_t libiotrace_struct_size, const char* libiotrace_struct_src) {
    size_t libiotrace_struct_ret = 0;

    while (*libiotrace_struct_src != '\0' && libiotrace_struct_size - libiotrace_struct_ret > 1) {
        *libiotrace_struct_buf++ = *libiotrace_struct_src++;
        libiotrace_struct_ret++;
    }

    *libiotrace_struct_buf = '\0';
    return libiotrace_struct_ret;
}
