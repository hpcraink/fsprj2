#ifndef LIBIOTRACE_WRAPPER_DEFINES_H
#define LIBIOTRACE_WRAPPER_DEFINES_H

#include <errno.h>
#include <dlfcn.h>
#include "json_include_struct.h"

#ifdef _GNU_SOURCE
#  define DLSYM(function_macro) __DLSYM(function_macro)
#  define __DLSYM(function) do { __real_##function = dlsym(RTLD_NEXT, #function); \
                                 assert(NULL != __real_##function); \
                               } while (0)
#else
#  error "Function dlsym without macro _GNU_SOURCE not usable!"
#endif

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

#define ERROR_FUNCTION(data) __ERROR_FUNCTION(data)
#if (_POSIX_C_SOURCE >= 200112L) && !  _GNU_SOURCE
#  define __ERROR_FUNCTION(data) if (0 != strerror_r(errno_data.errno_value, tmp_errno_text, MAXERRORTEXT)) { \
                                     snprintf(tmp_errno_text, MAXERRORTEXT, "Unknown error %d", errno_data.errno_value); \
                                 } \
								 errno_data.errno_text = tmp_errno_text;
#else
#  define __ERROR_FUNCTION(data) errno_data.errno_text = strerror_r(errno_data.errno_value, tmp_errno_text, MAXERRORTEXT);
#endif

#define GET_ERRNO(data) char tmp_errno_text[MAXERRORTEXT]; \
                        if (ok != data.return_state && 0 != errno_data.errno_value) { \
                        	ERROR_FUNCTION(data) \
							data.return_state_detail = &errno_data; \
                        } else { \
                            data.return_state_detail = NULL; \
                        }
#define CALL_REAL_FUNCTION_RET(data, return_value, function, ...) data.time_start = gettime(); \
                                                                  errno = errno_data.errno_value; \
                                                                  return_value = CALL_REAL(function)(__VA_ARGS__); \
                                                                  errno_data.errno_value = errno; \
                                                                  data.time_end = gettime();
#define CALL_REAL_FUNCTION(data, function, ...) data.time_start = gettime(); \
                                                errno = errno_data.errno_value; \
                                                CALL_REAL(function)(__VA_ARGS__); \
                                                errno_data.errno_value = errno; \
                                                data.time_end = gettime();
#define WRAP_START(data) struct errno_detail errno_data;\
                         errno_data.errno_value = errno;
#define WRAP_END(data) if(data.file_type == NULL || \
                          data.void_p_enum_file_type == file_memory || \
                          (data.void_p_enum_file_type == file_descriptor \
                           && STDIN_FILENO != ((struct file_descriptor *)data.file_type)->descriptor \
                           && STDOUT_FILENO != ((struct file_descriptor *)data.file_type)->descriptor \
                           && STDERR_FILENO != ((struct file_descriptor *)data.file_type)->descriptor) \
                          || \
                          (data.void_p_enum_file_type == file_stream \
                           && stdin != ((struct file_stream *)data.file_type)->stream \
                           && stdout != ((struct file_stream *)data.file_type)->stream \
                           && stderr != ((struct file_stream *)data.file_type)->stream)) { \
                           GET_ERRNO(data) \
                           writeData(&data); \
                       } \
                       errno = errno_data.errno_value;

#endif /* LIBIOTRACE_WRAPPER_DEFINES_H */
