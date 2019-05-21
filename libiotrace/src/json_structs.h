#include <limits.h>

#include "json_defines.h"

#ifdef JSON_STRUCT

#define MAXFILENAME PATH_MAX /* get length filename from limits.h */
#define MAXFUNCTIONNAME 40

JSON_STRUCT_START(file_stream)
  JSON_STRUCT_FILE_P(stream)
JSON_STRUCT_END

JSON_STRUCT_START(file_descriptor)
  JSON_STRUCT_INT(descriptor)
JSON_STRUCT_END

JSON_STRUCT_ENUM_START(access_mode)
  JSON_STRUCT_ENUM_ELEMENT(read_only)      //O_RDONLY or r
  JSON_STRUCT_ENUM_ELEMENT(write_only)     //O_WRONLY or w, a
  JSON_STRUCT_ENUM_ELEMENT(read_and_write) //O_RDWR or r+, w+, a+
  JSON_STRUCT_ENUM_ELEMENT(unknown_access_mode)
JSON_STRUCT_ENUM_END

JSON_STRUCT_ARRAY_BITFIELD_START(creation_flags)
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(cloexec)
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(creat)
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(directory)
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(excl)
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(noctty)
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(nofollow)
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(tmpfile)
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(trunc)
JSON_STRUCT_ARRAY_BITFIELD_END

JSON_STRUCT_ARRAY_BITFIELD_START(status_flags)
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(append)
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(async)
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(direct)
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(dsync)
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(largefile)
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(noatime)
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(nonblock)
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(ndelay)
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(path)
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(sync)
JSON_STRUCT_ARRAY_BITFIELD_END

JSON_STRUCT_ENUM_START(lock_mode)
  JSON_STRUCT_ENUM_ELEMENT(internal)        //FSETLOCKING_INTERNAL
  JSON_STRUCT_ENUM_ELEMENT(bycaller)        //FSETLOCKING_BYCALLER
  JSON_STRUCT_ENUM_ELEMENT(query_lock_mode) //FSETLOCKING_QUERY
  JSON_STRUCT_ENUM_ELEMENT(unknown_lock_mode)
JSON_STRUCT_ENUM_END

JSON_STRUCT_ENUM_START(orientation_mode)
  JSON_STRUCT_ENUM_ELEMENT(narrow)
  JSON_STRUCT_ENUM_ELEMENT(wide)
  JSON_STRUCT_ENUM_ELEMENT(not_set)
  JSON_STRUCT_ENUM_ELEMENT(query_orientation_mode)
JSON_STRUCT_ENUM_END

JSON_STRUCT_ENUM_START(read_write_state)
  JSON_STRUCT_ENUM_ELEMENT(ok)
  JSON_STRUCT_ENUM_ELEMENT(eof)
  JSON_STRUCT_ENUM_ELEMENT(error)
  JSON_STRUCT_ENUM_ELEMENT(unknown_read_write_state)
JSON_STRUCT_ENUM_END

/* struct for file open */
JSON_STRUCT_START(open_function)
  JSON_STRUCT_CSTRING_P_CONST(file_name, MAXFILENAME)
  JSON_STRUCT_ENUM(access_mode, mode)
  JSON_STRUCT_ARRAY_BITFIELD(creation_flags, creation)
  JSON_STRUCT_ARRAY_BITFIELD(status_flags, status)
JSON_STRUCT_END

/* struct for file open descriptor as stream */
JSON_STRUCT_START(fdopen_function)
  JSON_STRUCT_INT(descriptor)
  JSON_STRUCT_ENUM(access_mode, mode)
  JSON_STRUCT_ARRAY_BITFIELD(creation_flags, creation)
  JSON_STRUCT_ARRAY_BITFIELD(status_flags, status)
JSON_STRUCT_END

/* struct for file close */
JSON_STRUCT_START(close_function)
  JSON_STRUCT_INT(return_value)
JSON_STRUCT_END

/* struct for file lock */
JSON_STRUCT_START(lock_function)
JSON_STRUCT_END

/* struct for file try lock */
JSON_STRUCT_START(trylock_function)
  JSON_STRUCT_INT(return_value)
JSON_STRUCT_END

/* struct for file information */
JSON_STRUCT_START(information_function)
  JSON_STRUCT_INT(return_value)
JSON_STRUCT_END

/* struct for file lock_mode */
JSON_STRUCT_START(lock_mode_function)
  JSON_STRUCT_ENUM(lock_mode, set_mode)
  JSON_STRUCT_ENUM(lock_mode, return_mode)
JSON_STRUCT_END

/* struct for file orientation_mode */
JSON_STRUCT_START(orientation_mode_function)
  JSON_STRUCT_ENUM(orientation_mode, set_mode)
  JSON_STRUCT_ENUM(orientation_mode, return_mode)
JSON_STRUCT_END

/* struct for file write */
JSON_STRUCT_START(write_function)
  JSON_STRUCT_ENUM(read_write_state, return_state)
  JSON_STRUCT_SIZE_T(written_bytes)
JSON_STRUCT_END

/* struct for file read */
JSON_STRUCT_START(read_function)
  JSON_STRUCT_ENUM(read_write_state, return_state)
  JSON_STRUCT_SIZE_T(read_bytes)
JSON_STRUCT_END

/* struct for file unget */
JSON_STRUCT_START(unget_function)
  JSON_STRUCT_ENUM(read_write_state, return_state)
  JSON_STRUCT_INT(buffer_bytes)
JSON_STRUCT_END

/* struct for file clear error */
JSON_STRUCT_START(clearerr_function)
JSON_STRUCT_END

/* struct for time (has to be similar to struct timespec for function clock_gettime) */
JSON_STRUCT_START(time)
  JSON_STRUCT_LONG_INT(sec)
  JSON_STRUCT_LONG_INT(nsec)
JSON_STRUCT_END

/* basic struct for every call */
JSON_STRUCT_START(basic)
  JSON_STRUCT_CSTRING_P(hostname, HOST_NAME_MAX)
  JSON_STRUCT_PID_T(process_id)
  JSON_STRUCT_PID_T(thread_id)
  // ToDo: function_name as CSTRING_P ?
  JSON_STRUCT_CSTRING(function_name, MAXFUNCTIONNAME)
  JSON_STRUCT_STRUCT(time, time_start)
  JSON_STRUCT_STRUCT(time, time_end)
  JSON_STRUCT_VOID_P_START(file_type)
    JSON_STRUCT_VOID_P_ELEMENT(file_type, file_stream)
    JSON_STRUCT_VOID_P_ELEMENT(file_type, file_descriptor)
  JSON_STRUCT_VOID_P_END(file_type)
  JSON_STRUCT_VOID_P_START(function_data)
    JSON_STRUCT_VOID_P_ELEMENT(function_data, open_function)
    JSON_STRUCT_VOID_P_ELEMENT(function_data, fdopen_function)
    JSON_STRUCT_VOID_P_ELEMENT(function_data, close_function)
    JSON_STRUCT_VOID_P_ELEMENT(function_data, lock_function)
    JSON_STRUCT_VOID_P_ELEMENT(function_data, trylock_function)
    JSON_STRUCT_VOID_P_ELEMENT(function_data, information_function)
    JSON_STRUCT_VOID_P_ELEMENT(function_data, lock_mode_function)
	JSON_STRUCT_VOID_P_ELEMENT(function_data, orientation_mode_function)
    JSON_STRUCT_VOID_P_ELEMENT(function_data, write_function)
	JSON_STRUCT_VOID_P_ELEMENT(function_data, read_function)
	JSON_STRUCT_VOID_P_ELEMENT(function_data, unget_function)
	JSON_STRUCT_VOID_P_ELEMENT(function_data, clearerr_function)
  JSON_STRUCT_VOID_P_END(function_data)
JSON_STRUCT_END

#endif
