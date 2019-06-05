#include <limits.h>

#include "json_defines.h"

#ifdef JSON_STRUCT

#define MAXFILENAME PATH_MAX /* get length filename from limits.h */
#define MAXFUNCTIONNAME 40
#define MAXERRORTEXT 1024

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

// ToDo: check if all possible flags for each BITFIELD are covered

JSON_STRUCT_ARRAY_BITFIELD_START(creation_flags)
#if _POSIX_C_SOURCE >= 200809L || _XOPEN_SOURCE >= 700
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(cloexec)
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(directory)
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(nofollow)
#endif
#ifdef _GNU_SOURCE
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(tmpfile)
#endif
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(creat)
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(excl)
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(noctty)
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(trunc)
JSON_STRUCT_ARRAY_BITFIELD_END

JSON_STRUCT_ARRAY_BITFIELD_START(status_flags)
#ifdef _GNU_SOURCE
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(direct)
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(noatime)
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(path)
#endif
#ifdef _LARGEFILE64_SOURCE
  //ToDo: can this be used for all off64_t functions
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(largefile)
#endif
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(append)
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(async)
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(dsync)
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(nonblock)
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(ndelay)
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(sync)
JSON_STRUCT_ARRAY_BITFIELD_END

JSON_STRUCT_ARRAY_BITFIELD_START(mode_flags)
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(read_by_owner)     //S_IRUSR
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(write_by_owner)    //S_IWUSR
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(execute_by_owner)  //S_IXUSR
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(read_by_group)     //S_IRGRP
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(write_by_group)    //S_IWGRP
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(execute_by_group)  //S_IXGRP
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(read_by_others)    //S_IROTH
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(write_by_others)   //S_IWOTH
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(execute_by_others) //S_IXOTH
JSON_STRUCT_ARRAY_BITFIELD_END

JSON_STRUCT_ARRAY_BITFIELD_START(rwf_flags)
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(hipri)  //RWF_HIPRI
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(dsync)  //RWF_DSYNC
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(sync)   //RWF_SYNC
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(nowait) //RWF_NOWAIT
#ifdef RWF_APPEND
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(append) //RWF_APPEND
#endif
JSON_STRUCT_ARRAY_BITFIELD_END

JSON_STRUCT_ARRAY_BITFIELD_START(memory_map_flags)
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(shared)        //MAP_SHARED
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(private)       //MAP_PRIVATE
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(bit32)         //MAP_32BIT
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(anonymous)     //MAP_ANONYMOUS
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(denywrite)     //MAP_DENYWRITE
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(executable)    //MAP_EXECUTABLE
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(file)          //MAP_FILE
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(fixed)         //MAP_FIXED
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(growsdown)     //MAP_GROWSDOWN
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(hugetlb)       //MAP_HUGETLB
#ifdef MAP_HUGE_2MB
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(huge_2mb)      //MAP_HUGE_2MB
#endif
#ifdef MAP_HUGE_1GB
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(huge_1gb)      //MAP_HUGE_1GB
#endif
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(locked)        //MAP_LOCKED
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(nonblock)      //MAP_NONBLOCK
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(noreserve)     //MAP_NORESERVE
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(populate)      //MAP_POPULATE
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(stack)         //MAP_STACK
#ifdef MAP_UNINITIALIZED
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(uninitialized) //MAP_UNINITIALIZED
#endif
JSON_STRUCT_ARRAY_BITFIELD_END

JSON_STRUCT_ARRAY_BITFIELD_START(memory_protection_flags)
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(executed) //PROT_EXEC
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(read)     //PROT_READ
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(written)  //PROT_WRITE
JSON_STRUCT_ARRAY_BITFIELD_END

JSON_STRUCT_ARRAY_BITFIELD_START(memory_sync_flags)
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(sync)  //MS_SYNC
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(async) //MS_ASYNC
JSON_STRUCT_ARRAY_BITFIELD_END

JSON_STRUCT_ARRAY_BITFIELD_START(memory_remap_flags)
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(maymove) //MREMAP_MAYMOVE
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(fixed)   //MREMAP_FIXED
JSON_STRUCT_ARRAY_BITFIELD_END

JSON_STRUCT_ENUM_START(open_relative_to)
  JSON_STRUCT_ENUM_ELEMENT(file)
  JSON_STRUCT_ENUM_ELEMENT(current_working_dir)
JSON_STRUCT_ENUM_END

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

JSON_STRUCT_ENUM_START(seek_where)
  JSON_STRUCT_ENUM_ELEMENT(beginning_of_file)
  JSON_STRUCT_ENUM_ELEMENT(current_position)
  JSON_STRUCT_ENUM_ELEMENT(end_of_file)
  JSON_STRUCT_ENUM_ELEMENT(unknown_seek_where)
JSON_STRUCT_ENUM_END

JSON_STRUCT_ENUM_START(buffer_mode)
  JSON_STRUCT_ENUM_ELEMENT(fully_buffered)
  JSON_STRUCT_ENUM_ELEMENT(line_buffered)
  JSON_STRUCT_ENUM_ELEMENT(unbuffered)
  JSON_STRUCT_ENUM_ELEMENT(unknown_buffer_mode)
JSON_STRUCT_ENUM_END

JSON_STRUCT_ENUM_START(boolean)
  JSON_STRUCT_ENUM_ELEMENT(true)
  JSON_STRUCT_ENUM_ELEMENT(false)
JSON_STRUCT_ENUM_END

/* struct for file open */
JSON_STRUCT_START(open_function)
  JSON_STRUCT_CSTRING_P_CONST(file_name, MAXFILENAME)
  JSON_STRUCT_ENUM(access_mode, mode)
  JSON_STRUCT_ARRAY_BITFIELD(creation_flags, creation)
  JSON_STRUCT_ARRAY_BITFIELD(status_flags, status)
  JSON_STRUCT_ARRAY_BITFIELD(mode_flags, file_mode)
JSON_STRUCT_END

/* struct for file openat */
JSON_STRUCT_START(openat_function)
  JSON_STRUCT_CSTRING_P_CONST(file_name, MAXFILENAME)
  JSON_STRUCT_ENUM(access_mode, mode)
  JSON_STRUCT_ARRAY_BITFIELD(creation_flags, creation)
  JSON_STRUCT_ARRAY_BITFIELD(status_flags, status)
  JSON_STRUCT_ARRAY_BITFIELD(mode_flags, file_mode)
  JSON_STRUCT_ENUM(open_relative_to, relative_to)
  JSON_STRUCT_INT(file_descriptor)
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
  JSON_STRUCT_ENUM(read_write_state, return_state)
JSON_STRUCT_END

/* struct for file lock */
JSON_STRUCT_START(lock_function)
JSON_STRUCT_END

/* struct for file try lock */
JSON_STRUCT_START(trylock_function)
  JSON_STRUCT_ENUM(read_write_state, return_state)
JSON_STRUCT_END

/* struct for file information */
JSON_STRUCT_START(information_function)
  JSON_STRUCT_ENUM(boolean, return_bool)
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

/* struct for file pwrite */
JSON_STRUCT_START(pwrite_function)
  JSON_STRUCT_ENUM(read_write_state, return_state)
  JSON_STRUCT_SIZE_T(written_bytes)
  JSON_STRUCT_OFF_T(position)
JSON_STRUCT_END

/* struct for file pwrite2 */
JSON_STRUCT_START(pwrite2_function)
  JSON_STRUCT_ENUM(read_write_state, return_state)
  JSON_STRUCT_SIZE_T(written_bytes)
  JSON_STRUCT_OFF_T(position)
  JSON_STRUCT_ARRAY_BITFIELD(rwf_flags, flags)
JSON_STRUCT_END

/* struct for file read */
JSON_STRUCT_START(read_function)
  JSON_STRUCT_ENUM(read_write_state, return_state)
  JSON_STRUCT_SIZE_T(read_bytes)
JSON_STRUCT_END

/* struct for file pread */
JSON_STRUCT_START(pread_function)
  JSON_STRUCT_ENUM(read_write_state, return_state)
  JSON_STRUCT_SIZE_T(read_bytes)
  JSON_STRUCT_OFF_T(position)
JSON_STRUCT_END

/* struct for file pread2 */
JSON_STRUCT_START(pread2_function)
  JSON_STRUCT_ENUM(read_write_state, return_state)
  JSON_STRUCT_SIZE_T(read_bytes)
  JSON_STRUCT_OFF_T(position)
  JSON_STRUCT_ARRAY_BITFIELD(rwf_flags, flags)
JSON_STRUCT_END

/* struct for file copy read */
JSON_STRUCT_START(copy_read_function)
  JSON_STRUCT_ENUM(read_write_state, return_state)
  JSON_STRUCT_SIZE_T(read_bytes)
  JSON_STRUCT_OFF_T(position)
  JSON_STRUCT_ENUM(seek_where, relative_to)
  JSON_STRUCT_INT(to_file_descriptor)
JSON_STRUCT_END

/* struct for file copy write */
JSON_STRUCT_START(copy_write_function)
  JSON_STRUCT_ENUM(read_write_state, return_state)
  JSON_STRUCT_SIZE_T(written_bytes)
  JSON_STRUCT_OFF_T(position)
  JSON_STRUCT_ENUM(seek_where, relative_to)
  JSON_STRUCT_INT(from_file_descriptor)
JSON_STRUCT_END

/* struct for file scan */
JSON_STRUCT_START(scan_function)
  JSON_STRUCT_ENUM(read_write_state, return_state)
JSON_STRUCT_END

/* struct for file unget */
JSON_STRUCT_START(unget_function)
  JSON_STRUCT_ENUM(read_write_state, return_state)
  JSON_STRUCT_INT(buffer_bytes)
JSON_STRUCT_END

/* struct for file clear error */
JSON_STRUCT_START(clearerr_function)
JSON_STRUCT_END

/* struct for file position */
JSON_STRUCT_START(position_function)
  JSON_STRUCT_ENUM(read_write_state, return_state)
  JSON_STRUCT_OFF_T(position)
JSON_STRUCT_END

/* struct for file pos */
JSON_STRUCT_START(pos_function)
  JSON_STRUCT_ENUM(read_write_state, return_state)
JSON_STRUCT_END

/* struct for file positioning */
JSON_STRUCT_START(positioning_function)
  JSON_STRUCT_ENUM(read_write_state, return_state)
  JSON_STRUCT_ENUM(seek_where, relative_to)
  JSON_STRUCT_OFF_T(offset)
JSON_STRUCT_END

/* struct for file lpositioning */
JSON_STRUCT_START(lpositioning_function)
  JSON_STRUCT_ENUM(read_write_state, return_state)
  JSON_STRUCT_ENUM(seek_where, relative_to)
  JSON_STRUCT_OFF_T(offset)
  JSON_STRUCT_OFF_T(new_offset_relative_to_beginning_of_file)
JSON_STRUCT_END

/* struct for file flush */
JSON_STRUCT_START(flush_function)
  JSON_STRUCT_ENUM(read_write_state, return_state)
JSON_STRUCT_END

/* struct for file flushlbf */
JSON_STRUCT_START(flushlbf_function)
JSON_STRUCT_END

/* struct for file purge */
JSON_STRUCT_START(purge_function)
JSON_STRUCT_END

/* struct for file buffer */
JSON_STRUCT_START(buffer_function)
  JSON_STRUCT_ENUM(read_write_state, return_state)
  JSON_STRUCT_ENUM(buffer_mode, buffer_mode)
  JSON_STRUCT_SIZE_T(buffer_size)
JSON_STRUCT_END

/* struct for file bufsize */
JSON_STRUCT_START(bufsize_function)
  JSON_STRUCT_SIZE_T(buffer_size)
JSON_STRUCT_END

/* struct for memory map */
JSON_STRUCT_START(memory_map_function)
  JSON_STRUCT_ENUM(read_write_state, return_state)
  JSON_STRUCT_VOID_P(address)
  JSON_STRUCT_SIZE_T(length)
  JSON_STRUCT_OFF_T(offset)
  JSON_STRUCT_ARRAY_BITFIELD(memory_protection_flags, protection_flags)
  JSON_STRUCT_ARRAY_BITFIELD(memory_map_flags, map_flags)
JSON_STRUCT_END

/* struct for memory unmap */
JSON_STRUCT_START(memory_unmap_function)
  JSON_STRUCT_ENUM(read_write_state, return_state)
  JSON_STRUCT_VOID_P(address)
  JSON_STRUCT_SIZE_T(length)
JSON_STRUCT_END

/* struct for memory sync */
JSON_STRUCT_START(memory_sync_function)
  JSON_STRUCT_ENUM(read_write_state, return_state)
  JSON_STRUCT_VOID_P(address)
  JSON_STRUCT_SIZE_T(length)
  JSON_STRUCT_ARRAY_BITFIELD(memory_sync_flags, sync_flags)
JSON_STRUCT_END

/* struct for memory remap */
JSON_STRUCT_START(memory_remap_function)
  JSON_STRUCT_ENUM(read_write_state, return_state)
  JSON_STRUCT_VOID_P(address)
  JSON_STRUCT_SIZE_T(length)
  JSON_STRUCT_VOID_P(new_address)
  JSON_STRUCT_SIZE_T(new_length)
  JSON_STRUCT_ARRAY_BITFIELD(memory_remap_flags, remap_flags)
JSON_STRUCT_END

/* basic struct for every call */
JSON_STRUCT_START(basic)
  JSON_STRUCT_CSTRING_P(hostname, HOST_NAME_MAX)
  JSON_STRUCT_PID_T(process_id)
  JSON_STRUCT_PID_T(thread_id)
  // ToDo: function_name as CSTRING_P ?
  JSON_STRUCT_CSTRING(function_name, MAXFUNCTIONNAME)
  JSON_STRUCT_U_INT64_T(time_start)
  JSON_STRUCT_U_INT64_T(time_end)
  JSON_STRUCT_INT(errno_value)
  JSON_STRUCT_CSTRING_P(errno_text, MAXERRORTEXT)
  JSON_STRUCT_VOID_P_START(file_type)
    JSON_STRUCT_VOID_P_ELEMENT(file_type, file_stream)
    JSON_STRUCT_VOID_P_ELEMENT(file_type, file_descriptor)
  JSON_STRUCT_VOID_P_END(file_type)
  // ToDo: new field for boolean which shows if file position is changed (e.g. copy_file_range don't change file position)
  JSON_STRUCT_VOID_P_START(function_data)
    JSON_STRUCT_VOID_P_ELEMENT(function_data, open_function)
    JSON_STRUCT_VOID_P_ELEMENT(function_data, openat_function)
    JSON_STRUCT_VOID_P_ELEMENT(function_data, fdopen_function)
    JSON_STRUCT_VOID_P_ELEMENT(function_data, close_function)
    JSON_STRUCT_VOID_P_ELEMENT(function_data, lock_function)
    JSON_STRUCT_VOID_P_ELEMENT(function_data, trylock_function)
    JSON_STRUCT_VOID_P_ELEMENT(function_data, information_function)
    JSON_STRUCT_VOID_P_ELEMENT(function_data, lock_mode_function)
    JSON_STRUCT_VOID_P_ELEMENT(function_data, orientation_mode_function)
    JSON_STRUCT_VOID_P_ELEMENT(function_data, write_function)
    JSON_STRUCT_VOID_P_ELEMENT(function_data, pwrite_function)
    JSON_STRUCT_VOID_P_ELEMENT(function_data, pwrite2_function)
    JSON_STRUCT_VOID_P_ELEMENT(function_data, read_function)
    JSON_STRUCT_VOID_P_ELEMENT(function_data, pread_function)
    JSON_STRUCT_VOID_P_ELEMENT(function_data, pread2_function)
    JSON_STRUCT_VOID_P_ELEMENT(function_data, copy_read_function)
    JSON_STRUCT_VOID_P_ELEMENT(function_data, copy_write_function)
    JSON_STRUCT_VOID_P_ELEMENT(function_data, scan_function)
    JSON_STRUCT_VOID_P_ELEMENT(function_data, unget_function)
    JSON_STRUCT_VOID_P_ELEMENT(function_data, clearerr_function)
    JSON_STRUCT_VOID_P_ELEMENT(function_data, position_function)
    JSON_STRUCT_VOID_P_ELEMENT(function_data, pos_function)
    JSON_STRUCT_VOID_P_ELEMENT(function_data, positioning_function)
    JSON_STRUCT_VOID_P_ELEMENT(function_data, lpositioning_function)
    JSON_STRUCT_VOID_P_ELEMENT(function_data, flush_function)
    JSON_STRUCT_VOID_P_ELEMENT(function_data, flushlbf_function)
    JSON_STRUCT_VOID_P_ELEMENT(function_data, purge_function)
    JSON_STRUCT_VOID_P_ELEMENT(function_data, buffer_function)
    JSON_STRUCT_VOID_P_ELEMENT(function_data, bufsize_function)
    JSON_STRUCT_VOID_P_ELEMENT(function_data, memory_map_function)
    JSON_STRUCT_VOID_P_ELEMENT(function_data, memory_unmap_function)
    JSON_STRUCT_VOID_P_ELEMENT(function_data, memory_sync_function)
    JSON_STRUCT_VOID_P_ELEMENT(function_data, memory_remap_function)
  JSON_STRUCT_VOID_P_END(function_data)
JSON_STRUCT_END

#endif
