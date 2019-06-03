#ifndef LIBIOTRACE_WRAPPER_DEFINES_H
#define LIBIOTRACE_WRAPPER_DEFINES_H

#include <errno.h>

#define CALL_REAL(function_macro) __CALL_REAL(function_macro)
#define __CALL_REAL(function) __real_##function

#define REAL(function_macro) __REAL(function_macro)
#ifdef IO_LIB_STATIC
#  define REAL_TYPE
#  define __REAL(function_name) __real_##function_name
#  define REAL_INIT
#else
#  define REAL_TYPE static
#  define __REAL(function_name) (*__real_##function_name)
#  define REAL_INIT = NULL
#endif

#define WRAP(function_macro) __WRAP(function_macro)
#ifdef IO_LIB_STATIC
#  define __WRAP(function_name) __wrap_##function_name
// ToDo: __func__ dependencies
#  define POSIX_IO_SET_FUNCTION_NAME(data) strncpy(data, __func__ + 7, MAXFUNCTIONNAME) /* +7 removes beginning __wrap_ from __func__ */
#else
#  define __WRAP(function_name) function_name
#  define POSIX_IO_SET_FUNCTION_NAME(data) strncpy(data, __func__, MAXFUNCTIONNAME)
#endif

#define GET_ERRNO(data) char tmp_errno_text[1024]; \
                        if (0 != data.errno_value) { \
                        	data.errno_text = strerror_r(data.errno_value, tmp_errno_text, 1024); /* ToDo: get 1024 from predefined macro */\
                        } else { \
                        	*tmp_errno_text = '\0'; \
                        	data.errno_text = tmp_errno_text; \
                        }
#define CALL_REAL_FUNCTION_RET(data, return_value, function, ...) data.time_start = gettime(); \
                                                                  errno = data.errno_value; \
                                                                  return_value = CALL_REAL(function)(__VA_ARGS__); \
                                                                  data.errno_value = errno; \
                                                                  data.time_end = gettime(); \
                                                                  GET_ERRNO(data)
#define CALL_REAL_FUNCTION(data, function, ...) data.time_start = gettime(); \
                                                errno = data.errno_value; \
                                                CALL_REAL(function)(__VA_ARGS__); \
                                                data.errno_value = errno; \
                                                data.time_end = gettime(); \
                                                GET_ERRNO(data)
#define WRAP_START(data) data.errno_value = errno;
#define WRAP_END(data) if((data.void_p_enum_file_type == file_descriptor \
                           && *(int *)data.file_type != STDIN_FILENO \
                           && *(int *)data.file_type != STDOUT_FILENO \
                           && *(int *)data.file_type != STDERR_FILENO) \
                          || \
                          (data.void_p_enum_file_type == file_stream \
                           && *(FILE **)data.file_type != stdin /* ToDo: dup can duplicate a stream, use fileno to get descriptor? */\
                           && *(FILE **)data.file_type != stdout \
                           && *(FILE **)data.file_type != stderr)) { \
                           writeData(&data); \
                       } \
                       errno = data.errno_value;

#endif /* LIBIOTRACE_WRAPPER_DEFINES_H */
