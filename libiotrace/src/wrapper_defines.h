#ifndef LIBIOTRACE_WRAPPER_DEFINES_H
#define LIBIOTRACE_WRAPPER_DEFINES_H

#include <errno.h>
#include <dlfcn.h>

#include "libiotrace_config.h"
#include "common/gettime.h"
#include "common/utils.h"
#include "common/error.h"
#include "libiotrace_include_struct.h"


#if defined(STRACING_ENABLED) && defined(FILENAME_RESOLUTION_ENABLED)
#  include "stracing/libiotrace/tasks/lsep/stracing_lsep.h"
#  define STRACING_LSEP_PROCESS_NEW_SCEVENTS() stracing_lsep_process_new_scevents()
#else
#  define STRACING_LSEP_PROCESS_NEW_SCEVENTS() do {  } while(0)
#endif

#ifdef FILENAME_RESOLUTION_ENABLED
#  include "fnres/fnres.h"
#  ifdef STRACING_ENABLED
#    define FNRES_TRACE_IOEVENT(IOEVENT_PTR) do { \
         if ((0 != fnres_trace_ioevent((IOEVENT_PTR)) && __void_p_enum_file_type_file_stream == (IOEVENT_PTR)->__void_p_enum_file_type) && \
              0 == stracing_fnres_lookup_and_alias_stream(IOEVENT_PTR)) { \
              fnres_trace_ioevent((IOEVENT_PTR)); \
         } \
     } while(0)
#  else
#    define FNRES_TRACE_IOEVENT(IOEVENT_PTR) (void)fnres_trace_ioevent(IOEVENT_PTR)
#  endif /* STRACING_ENABLED */
#else
#  define FNRES_TRACE_IOEVENT(IOEVENT_PTR) do {  } while(0)
#endif /* FILENAME_RESOLUTION_ENABLED */




#define LINE_BREAK "\r\n"

#ifdef ALL_WRAPPERS_ACTIVE
#  define WRAPPER_ACTIVE 1
#else
#  define WRAPPER_ACTIVE 0
#endif

#define WRAPPER_ACTIVATE(cmp_string, function, toggle) else if (!strcmp(cmp_string, #function)) { \
                                                  active_wrapper_status.function = toggle; \
                                               }

#ifdef WITH_POSIX_IO
#  define CALL_REAL_POSIX_SYNC(function) CALL_REAL(function)
#else
#  define CALL_REAL_POSIX_SYNC(function) function
#endif

#if defined(HAVE___XSTAT) && defined(_STAT_VER_LINUX)
#  define libiotrace_stat(pathname, statbuf) __libiotrace_stat(pathname, statbuf)
#  define __libiotrace_stat(pathname, statbuf) CALL_REAL_POSIX_SYNC(__xstat)(_STAT_VER_LINUX, pathname, statbuf)
#else
#  define libiotrace_stat(pathname, statbuf) __libiotrace_stat(pathname, statbuf)
#  define __libiotrace_stat(pathname, statbuf) CALL_REAL_POSIX_SYNC(stat)(pathname, statbuf)
#endif
#if defined(HAVE___FXSTAT) && defined(_STAT_VER_LINUX)
#  define libiotrace_fstat(fd, statbuf) __libiotrace_fstat(fd, statbuf)
#  define __libiotrace_fstat(fd, statbuf) CALL_REAL_POSIX_SYNC(__fxstat)(_STAT_VER_LINUX, fd, statbuf)
#else
#  define libiotrace_fstat(fd, statbuf) __libiotrace_fstat(fd, statbuf)
#  define __libiotrace_fstat(fd, statbuf) CALL_REAL_POSIX_SYNC(fstat)(fd, statbuf)
#endif

#ifdef WITH_ALLOC
#  define CALL_REAL_ALLOC_SYNC(function) CALL_REAL(function)
#else
#  define CALL_REAL_ALLOC_SYNC(function) function
#endif

#ifdef _GNU_SOURCE
#  define DLSYM(function_macro) __DLSYM(function_macro)
#  define __DLSYM(function) do { dlerror(); /* clear old error conditions */\
                                 __extension__({ \
                                     /* dlsym returns a void pointer (data pointer) */ \
                                     /* ISO C forbids cast to function pointer      */ \
                                     /* => __extension__ to suppress the warning    */ \
                                     __real_##function = dlsym(RTLD_NEXT, #function); \
                                 }); \
                                 char * dlsym_dlerror_##function = dlerror(); \
                                 if (NULL != dlsym_dlerror_##function) { \
                                     LOG_ERROR_AND_DIE("dlsym error (%s)", dlsym_dlerror_##function); \
                                 } \
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
#  define REAL_TYPE extern
#  define __REAL(function_name) (*__real_##function_name)
#  define REAL_INIT
#endif
#define REAL_DEFINITION_TYPE
#define REAL_DEFINITION REAL
#define REAL_DEFINITION_INIT = NULL

#define WRAP(function_macro) __WRAP(function_macro)
#ifdef IO_LIB_TEST
#  define __WRAP(function_name) __test_##function_name
// ToDo: __func__ dependencies
#  define POSIX_IO_SET_FUNCTION_NAME(data) strncpy(data, __func__ + 7, MAX_FUNCTION_NAME) /* +7 removes beginning __wrap_ from __func__ */
#  define POSIX_IO_SET_FUNCTION_NAME_NO_WRAPPER(data) strncpy(data, __func__, MAX_FUNCTION_NAME)
#elif defined(IO_LIB_STATIC)
#  define __WRAP(function_name) __wrap_##function_name
// ToDo: __func__ dependencies
#  define POSIX_IO_SET_FUNCTION_NAME(data) strncpy(data, __func__ + 7, MAX_FUNCTION_NAME) /* +7 removes beginning __wrap_ from __func__ */
#  define POSIX_IO_SET_FUNCTION_NAME_NO_WRAPPER(data) strncpy(data, __func__, MAX_FUNCTION_NAME)
#else
#  define __WRAP(function_name) function_name
#  define POSIX_IO_SET_FUNCTION_NAME(data) strncpy(data, __func__, MAX_FUNCTION_NAME)
#  define POSIX_IO_SET_FUNCTION_NAME_NO_WRAPPER(data) POSIX_IO_SET_FUNCTION_NAME(data)
#endif
#define POSIX_IO_SET_FUNCTION_NAME_STRING(data, string) strncpy(data, string, MAX_FUNCTION_NAME)

#define ERROR_FUNCTION(data) __ERROR_FUNCTION(data)
#if (_POSIX_C_SOURCE >= 200112L) && !  _GNU_SOURCE
#  define __ERROR_FUNCTION(data) if (0 != strerror_r(errno_data.errno_value, tmp_errno_text, MAX_ERROR_TEXT)) { \
                                     snprintf(tmp_errno_text, MAX_ERROR_TEXT, "Unknown error %d", errno_data.errno_value); \
                                 } \
                                 errno_data.errno_text = tmp_errno_text;
#else
#  define __ERROR_FUNCTION(data) errno_data.errno_text = strerror_r(errno_data.errno_value, tmp_errno_text, MAX_ERROR_TEXT);
#endif
#define MPI_ERROR_FUNCTION(data) int tmp_len_errno_string; \
                                 MPI_Error_string(errno_data.errno_value, tmp_errno_text, &tmp_len_errno_string); \
                                 errno_data.errno_text = tmp_errno_text;

#define GET_ERRNO(data) {char tmp_errno_text[MAX_ERROR_TEXT]; \
                         if (error == data.return_state && 0 != errno_data.errno_value) { \
                             ERROR_FUNCTION(data) \
                             data.return_state_detail = &errno_data; \
                         } else { \
                             data.return_state_detail = NULL; \
                         }} while(0);
#define GET_MPI_ERRNO(data) {char tmp_errno_text[MPI_MAX_ERROR_STRING]; \
                             if (error == data.return_state && MPI_SUCCESS != errno_data.errno_value) { \
                                 MPI_ERROR_FUNCTION(data) \
                                 data.return_state_detail = &errno_data; \
                             } else { \
                                 data.return_state_detail = NULL; \
                             }} while(0);
#ifdef IO_LIB_STATIC
#  define CALL_REAL_FUNCTION_RET(data, return_value, function, ...) __CALL_REAL_FUNCTION_RET(data, return_value, function, __VA_ARGS__)
#  define CALL_REAL_FUNCTION_RET_NO_RETURN(data, return_value, function, ...) __CALL_REAL_FUNCTION_RET_NO_RETURN(data, return_value, function, __VA_ARGS__)
#  define CALL_REAL_FUNCTION(data, function, ...) __CALL_REAL_FUNCTION(data, function, __VA_ARGS__)
#  define CALL_REAL_FUNCTION_NO_RETURN(data, function, ...) __CALL_REAL_FUNCTION_NO_RETURN(data, function, __VA_ARGS__)
#else
#  define CALL_REAL_FUNCTION_RET(data, return_value, function, ...) __CALL_REAL_FUNCTION_RET(data, return_value, function, __VA_ARGS__)
#  define CALL_REAL_FUNCTION_RET_NO_RETURN(data, return_value, function, ...) __CALL_REAL_FUNCTION_RET_NO_RETURN(data, return_value, function, __VA_ARGS__)
#  define CALL_REAL_FUNCTION(data, function, ...) __CALL_REAL_FUNCTION(data, function, __VA_ARGS__)
#  define CALL_REAL_FUNCTION_NO_RETURN(data, function, ...) __CALL_REAL_FUNCTION_NO_RETURN(data, function, __VA_ARGS__)
#endif

#if defined(IOTRACE_ENABLE_LOGFILE) || defined(IOTRACE_ENABLE_INFLUXDB) || defined(ENABLE_REMOTE_CONTROL)
#  define CALL_CLEANUP_PROCESS() cleanup_process()
#else
#  define CALL_CLEANUP_PROCESS()
#endif

#define __CALL_REAL_FUNCTION_RET(data, return_value, function, ...) data.time_start = gettime(); \
                                                                    errno = errno_data.errno_value; \
                                                                    return_value = CALL_REAL(function)(__VA_ARGS__); \
                                                                    errno_data.errno_value = errno; \
                                                                    data.time_end = gettime();
#define __CALL_REAL_FUNCTION_RET_NO_RETURN(data, return_value, function, ...) data.time_start = gettime(); \
                                                                              data.time_end = gettime(); \
                                                                              WRAP_END(data, function) \
                                                                              CALL_CLEANUP_PROCESS(); \
                                                                              errno = errno_data.errno_value; \
                                                                              return_value = CALL_REAL(function)(__VA_ARGS__); \
                                                                              errno_data.errno_value = errno;
#define __CALL_REAL_FUNCTION(data, function, ...) data.time_start = gettime(); \
                                                  errno = errno_data.errno_value; \
                                                  CALL_REAL(function)(__VA_ARGS__); \
                                                  errno_data.errno_value = errno; \
                                                  data.time_end = gettime();
#define __CALL_REAL_FUNCTION_NO_RETURN(data, function, ...) data.time_start = gettime(); \
                                                            data.time_end = gettime(); \
                                                            WRAP_END(data, function) \
                                                            CALL_CLEANUP_PROCESS(); \
                                                            errno = errno_data.errno_value; \
                                                            CALL_REAL(function)(__VA_ARGS__); \
                                                            errno_data.errno_value = errno; \
                                                            abort();
#define CALL_REAL_MPI_FUNCTION_RET(data, return_value, function, ...) data.time_start = gettime(); \
                                                                      errno = errno_value; \
                                                                      return_value = P##function(__VA_ARGS__); \
                                                                      errno_value = errno; \
                                                                      data.time_end = gettime();
#define CALL_REAL_MPI_FUNCTION(function, ...) if(!active_wrapper_status.function) { \
                                                  return P##function(__VA_ARGS__); \
                                              }
#define SET_MPI_ERROR(errno, status) if (MPI_ERR_IN_STATUS == errno && MPI_STATUS_IGNORE != status) { \
                                         errno_data.errno_value = status->MPI_ERROR; \
                                     } else { \
                                         errno_data.errno_value = errno; \
                                     }
#ifdef LOG_WRAPPER_TIME
#  define WRAPPER_TIME_START(data) data.wrapper.time_start = gettime();
#  define WRAPPER_TIME_END(data) data.wrapper.time_end = gettime();
#else
#  define WRAPPER_TIME_START(data)
#  define WRAPPER_TIME_END(data)
#endif
#define WRAP_START(data) struct errno_detail errno_data; \
                         errno_data.errno_value = errno; \
                         WRAPPER_TIME_START(data) \
                         if (!init_done) { \
                             init_process(); /* if some __attribute__((constructor))-function calls a wrapped function: */ \
                                             /* init_process() must be called first */ \
                         }
#define WRAP_MPI_START(data) int errno_value; \
                             struct errno_detail errno_data; \
                             errno_value = errno; \
                             WRAPPER_TIME_START(data) \
                             if (!init_done) { \
                                 init_process(); /* if some __attribute__((constructor))-function calls a wrapped function: */ \
                                                 /* init_process() must be called first */ \
                             }

#ifdef IOTRACE_ENABLE_INFLUXDB
#  define CALL_WRITE_INTO_INFLUXDB(data) data.time_diff = data.time_end - data.time_start; \
                                         write_into_influxdb(&data)
#else
#  define CALL_WRITE_INTO_INFLUXDB(data)
#endif

#ifdef IOTRACE_ENABLE_LOGFILE
#  define CALL_WRITE_INTO_BUFFER(data) io_log_file_buffer_write(&data)
#else
#  define CALL_WRITE_INTO_BUFFER(data)
#endif

//ToDo: use kcmp() with KCMP_FILE as type to check if file descriptor is STDIN_FILENO, STDOUT_FILENO or STDERR_FILENO
//kcmp() is linux-specific! But without kcmp() a duped descriptor will not be recognized! That will lead to problems by following analysis of written data!
//ToDo: check for which file_type should not be written instead of which should be written
#define WRAP_END(data, functionname) __WRAP_END(data, functionname)

#ifndef WITH_STD_IO
#  ifdef WITH_ALLOC
#    define CHECK_FTYPE_ALLOC(DATA) DATA.__void_p_enum_file_type == __void_p_enum_file_type_file_alloc ||
#  else
#    define CHECK_FTYPE_ALLOC(DATA)
#  endif /* WITH_ALLOC */
#  define __WRAP_END(data, functionname) GET_ERRNO(data) \
                         if(data.__file_type == NULL || \
                            data.__void_p_enum_file_type == __void_p_enum_file_type_file_memory || \
                            data.__void_p_enum_file_type == __void_p_enum_file_type_file_async || \
                            data.__void_p_enum_file_type == __void_p_enum_file_type_file_mpi || \
                            data.__void_p_enum_file_type == __void_p_enum_file_type_shared_library || \
                            CHECK_FTYPE_ALLOC(data) \
                            (data.__void_p_enum_file_type == __void_p_enum_file_type_file_descriptor \
                             && STDIN_FILENO != ((struct file_descriptor *)data.__file_type)->descriptor \
                             && STDOUT_FILENO != ((struct file_descriptor *)data.__file_type)->descriptor \
                             && STDERR_FILENO != ((struct file_descriptor *)data.__file_type)->descriptor) \
                            || \
                            (data.__void_p_enum_file_type == __void_p_enum_file_type_file_stream \
                             && stdin != ((struct file_stream *)data.__file_type)->stream \
                             && stdout != ((struct file_stream *)data.__file_type)->stream \
                             && stderr != ((struct file_stream *)data.__file_type)->stream)) {     \
                            STRACING_LSEP_PROCESS_NEW_SCEVENTS();                             \
                            FNRES_TRACE_IOEVENT(&data); \
                            if(active_wrapper_status.functionname){ \
                              CALL_WRITE_INTO_INFLUXDB(data); \
                              CALL_WRITE_INTO_BUFFER(data); \
                            } \
                         } \
                         WRAP_FREE(&data) \
                         errno = errno_data.errno_value;
#  else
#    define __WRAP_END(data, functionname) GET_ERRNO(data) \
                         STRACING_LSEP_PROCESS_NEW_SCEVENTS(); \
                         FNRES_TRACE_IOEVENT(&data); \
                         if(active_wrapper_status.functionname){ \
                           CALL_WRITE_INTO_INFLUXDB(data); \
                           CALL_WRITE_INTO_BUFFER(data); \
                         } \
                         WRAP_FREE(&data) \
                         errno = errno_data.errno_value;
#  endif /* WITH_STD_IO */

#define WRAP_MPI_END(data, functionname) GET_MPI_ERRNO(data) \
                           STRACING_LSEP_PROCESS_NEW_SCEVENTS(); \
                           FNRES_TRACE_IOEVENT(&data); \
                           CALL_WRITE_INTO_INFLUXDB(data); \
                           CALL_WRITE_INTO_BUFFER(data); \
                           WRAP_FREE(&data) \
                           errno = errno_value;

#define WRAP_END_WITHOUT_WRITE(data) WRAP_FREE(&data) \
                                     errno = errno_data.errno_value;

#define WRAP_FREE(data) free_memory(data);

#endif /* LIBIOTRACE_WRAPPER_DEFINES_H */
