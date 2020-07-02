#include <limits.h>

#include "json_defines.h"

#ifdef JSON_STRUCT

#define MAXFILENAME PATH_MAX /* get length filename from limits.h */
#ifndef MAX_ERROR_TEXT
#  define MAX_ERROR_TEXT 1024
#endif
#ifndef MAX_FUNCTION_NAME
#  define MAX_FUNCTION_NAME 40
#endif
#ifndef MAX_MSG_FILE_DESCRIPTORS
#  define MAX_MSG_FILE_DESCRIPTORS 200 // TODO: use SCM_MAX_FD instead
#endif

#ifndef MAX_STACKTRACE_DEPTH
#  define MAX_STACKTRACE_DEPTH 50
#endif
#ifndef MAX_STACKTRACE_ENTRY_LENGTH
#  define MAX_STACKTRACE_ENTRY_LENGTH 200
#endif

#define MAX_SOCKADDR_LENGTH ((sizeof(struct sockaddr_storage) >= sizeof(struct sockaddr_un) ? \
                              sizeof(struct sockaddr_storage) : sizeof(struct sockaddr_un)) \
                             - sizeof(sa_family_t))

JSON_STRUCT_START(file_stream)
  JSON_STRUCT_FILE_P(stream)
JSON_STRUCT_END

JSON_STRUCT_START(file_dir)
  JSON_STRUCT_VOID_P(directory_stream)
JSON_STRUCT_END

JSON_STRUCT_START(file_descriptor)
  JSON_STRUCT_INT(descriptor)
JSON_STRUCT_END

JSON_STRUCT_START(file_memory)
  JSON_STRUCT_VOID_P(address)
  JSON_STRUCT_SIZE_T(length)
JSON_STRUCT_END

JSON_STRUCT_START(file_async)
  JSON_STRUCT_VOID_P_CONST(async)
JSON_STRUCT_END

JSON_STRUCT_START(file_mpi)
  JSON_STRUCT_INT(mpi_file)
JSON_STRUCT_END

JSON_STRUCT_START(request_mpi)
  JSON_STRUCT_INT(request_id)
JSON_STRUCT_END

JSON_STRUCT_START(shared_library)
  JSON_STRUCT_VOID_P_CONST(dl_handle)
JSON_STRUCT_END

JSON_STRUCT_START(errno_detail)
  JSON_STRUCT_INT(errno_value)
  JSON_STRUCT_CSTRING_P(errno_text, MAX_ERROR_TEXT)
JSON_STRUCT_END

JSON_STRUCT_ENUM_START(access_mode)
  JSON_STRUCT_ENUM_ELEMENT(read_only)      //O_RDONLY or r
  JSON_STRUCT_ENUM_ELEMENT(write_only)     //O_WRONLY or w, a
  JSON_STRUCT_ENUM_ELEMENT(read_and_write) //O_RDWR or r+, w+, a+
  JSON_STRUCT_ENUM_ELEMENT(unknown_access_mode)
JSON_STRUCT_ENUM_END

// ToDo: check if all possible flags for each BITFIELD are covered

JSON_STRUCT_ARRAY_BITFIELD_START(creation_flags)
#ifdef HAVE_O_CLOEXEC
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(cloexec)
#endif
#ifdef HAVE_O_DIRECTORY
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(directory)
#endif
#ifdef HAVE_O_NOFOLLOW
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(nofollow)
#endif
#ifdef HAVE_O_TMPFILE
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(tmpfile)
#endif
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(creat)
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(excl)
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(noctty)
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(trunc)
JSON_STRUCT_ARRAY_BITFIELD_END

JSON_STRUCT_ARRAY_BITFIELD_START(status_flags)
#ifdef HAVE_O_DIRECT
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(direct)
#endif
#ifdef HAVE_O_NOATIME
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(noatime)
#endif
#ifdef HAVE_O_PATH
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(path)
#endif
#ifdef HAVE_O_LARGEFILE
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(largefile)
#endif
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(append)
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(initial_append)
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(async)
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(dsync)
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(nonblock)
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(ndelay)
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(sync)
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(delete_on_close)
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(unique_open)
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(sequential)
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
#ifdef HAVE_RWF_APPEND
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(append) //RWF_APPEND
#endif
JSON_STRUCT_ARRAY_BITFIELD_END

JSON_STRUCT_ARRAY_BITFIELD_START(memory_map_flags)
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(shared)        //MAP_SHARED
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(private)       //MAP_PRIVATE
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(fixed)         //MAP_FIXED
#ifdef HAVE_MAP_32BIT
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(bit32)         //MAP_32BIT
#endif
#ifdef HAVE_MAP_ANONYMOUS
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(anonymous)     //MAP_ANONYMOUS
#endif
#ifdef HAVE_MAP_DENYWRITE
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(denywrite)     //MAP_DENYWRITE
#endif
#ifdef HAVE_MAP_EXECUTABLE
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(executable)    //MAP_EXECUTABLE
#endif
#ifdef HAVE_MAP_FILE
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(file)          //MAP_FILE
#endif
#ifdef HAVE_MAP_GROWSDOWN
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(growsdown)     //MAP_GROWSDOWN
#endif
#ifdef HAVE_MAP_HUGETLB
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(hugetlb)       //MAP_HUGETLB
#endif
#ifdef HAVE_MAP_HUGE_2MB
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(huge_2mb)      //MAP_HUGE_2MB
#endif
#ifdef HAVE_MAP_HUGE_1GB
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(huge_1gb)      //MAP_HUGE_1GB
#endif
#ifdef HAVE_MAP_LOCKED
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(locked)        //MAP_LOCKED
#endif
#ifdef HAVE_MAP_NONBLOCK
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(nonblock)      //MAP_NONBLOCK
#endif
#ifdef HAVE_MAP_NORESERVE
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(noreserve)     //MAP_NORESERVE
#endif
#ifdef HAVE_MAP_POPULATE
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(populate)      //MAP_POPULATE
#endif
#ifdef HAVE_MAP_STACK
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(stack)         //MAP_STACK
#endif
#ifdef HAVE_MAP_UNINITIALIZED
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(uninitialized) //MAP_UNINITIALIZED
#endif
JSON_STRUCT_ARRAY_BITFIELD_END

JSON_STRUCT_ARRAY_BITFIELD_START(memory_protection_flags)
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(executed) //PROT_EXEC
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(read)     //PROT_READ
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(written)  //PROT_WRITE
JSON_STRUCT_ARRAY_BITFIELD_END

JSON_STRUCT_ARRAY_BITFIELD_START(memory_sync_flags)
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(sync)       //MS_SYNC
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(async)      //MS_ASYNC
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(invalidate) //MS_INVALIDATE
JSON_STRUCT_ARRAY_BITFIELD_END

#ifdef HAVE_MREMAP
JSON_STRUCT_ARRAY_BITFIELD_START(memory_remap_flags)
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(maymove) //MREMAP_MAYMOVE
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(fixed)   //MREMAP_FIXED
JSON_STRUCT_ARRAY_BITFIELD_END
#endif

JSON_STRUCT_ARRAY_BITFIELD_START(dlopen_flags)
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(lazy_binding) //RTLD_LAZY
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(bind_now)     //RTLD_NOW
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(global)       //RTLD_GLOBAL
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(local)        //RTLD_LOCAL
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(no_delete)    //RTLD_NODELETE
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(no_load)      //RTLD_NOLOAD
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(deep_bind)    //RTLD_DEEPBIND
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
#ifdef HAVE_SEEK_DATA
  JSON_STRUCT_ENUM_ELEMENT(next_data)
#endif
#ifdef HAVE_SEEK_HOLE
  JSON_STRUCT_ENUM_ELEMENT(next_hole)
#endif
  JSON_STRUCT_ENUM_ELEMENT(unknown_seek_where)
JSON_STRUCT_ENUM_END

JSON_STRUCT_ENUM_START(buffer_mode)
  JSON_STRUCT_ENUM_ELEMENT(fully_buffered)
  JSON_STRUCT_ENUM_ELEMENT(line_buffered)
  JSON_STRUCT_ENUM_ELEMENT(unbuffered)
  JSON_STRUCT_ENUM_ELEMENT(unknown_buffer_mode)
JSON_STRUCT_ENUM_END

JSON_STRUCT_ENUM_START(madvice_advice)
  JSON_STRUCT_ENUM_ELEMENT(normal)         //MADV_NORMAL
  JSON_STRUCT_ENUM_ELEMENT(madvice_random) //MADV_RANDOM
  JSON_STRUCT_ENUM_ELEMENT(sequential)     //MADV_SEQUENTIAL
  JSON_STRUCT_ENUM_ELEMENT(willneed)       //MADV_WILLNEED
  JSON_STRUCT_ENUM_ELEMENT(dontneed)       //MADV_DONTNEED
#ifdef HAVE_MADV_REMOVE
  JSON_STRUCT_ENUM_ELEMENT(madvice_remove) //MADV_REMOVE
#endif
#ifdef HAVE_MADV_DONTFORK
  JSON_STRUCT_ENUM_ELEMENT(dontfork)       //MADV_DONTFORK
#endif
#ifdef HAVE_MADV_DOFORK
  JSON_STRUCT_ENUM_ELEMENT(dofork)         //MADV_DOFORK
#endif
#ifdef HAVE_MADV_HWPOISON
  JSON_STRUCT_ENUM_ELEMENT(hwpoison)       //MADV_HWPOISON
#endif
#ifdef HAVE_MADV_MERGEABLE
  JSON_STRUCT_ENUM_ELEMENT(mergeable)      //MADV_MERGEABLE
#endif
#ifdef HAVE_MADV_UNMERGEABLE
  JSON_STRUCT_ENUM_ELEMENT(unmergeable)    //MADV_UNMERGEABLE
#endif
#ifdef HAVE_MADV_SOFT_OFFLINE
  JSON_STRUCT_ENUM_ELEMENT(soft_offline)   //MADV_SOFT_OFFLINE
#endif
#ifdef HAVE_MADV_HUGEPAGE
  JSON_STRUCT_ENUM_ELEMENT(hugepage)       //MADV_HUGEPAGE
#endif
#ifdef HAVE_MADV_NOHUGEPAGE
  JSON_STRUCT_ENUM_ELEMENT(nohugepage)     //MADV_NOHUGEPAGE
#endif
#ifdef HAVE_MADV_DONTDUMP
  JSON_STRUCT_ENUM_ELEMENT(dontdump)       //MADV_DONTDUMP
#endif
#ifdef HAVE_MADV_DODUMP
  JSON_STRUCT_ENUM_ELEMENT(dodump)         //MADV_DODUMP
#endif
#ifdef HAVE_MADV_FREE
  JSON_STRUCT_ENUM_ELEMENT(madvice_free)   //MADV_FREE
#endif
#ifdef HAVE_MADV_WIPEONFORK
  JSON_STRUCT_ENUM_ELEMENT(wipeonfork)     //MADV_WIPEONFORK
#endif
#ifdef HAVE_MADV_KEEPONFORK
  JSON_STRUCT_ENUM_ELEMENT(keeponfork)     //MADV_KEEPONFORK
#endif
  JSON_STRUCT_ENUM_ELEMENT(unknown_madvice_advice)
JSON_STRUCT_ENUM_END

JSON_STRUCT_ENUM_START(posix_madvice_advice)
  JSON_STRUCT_ENUM_ELEMENT(posix_normal)         //POSIX_MADV_NORMAL
  JSON_STRUCT_ENUM_ELEMENT(posix_madvice_random) //POSIX_MADV_RANDOM
  JSON_STRUCT_ENUM_ELEMENT(posix_sequential)     //POSIX_MADV_SEQUENTIAL
  JSON_STRUCT_ENUM_ELEMENT(posix_willneed)       //POSIX_MADV_WILLNEED
  JSON_STRUCT_ENUM_ELEMENT(posix_dontneed)       //POSIX_MADV_DONTNEED
  JSON_STRUCT_ENUM_ELEMENT(unknown_posix_madvice_advice)
JSON_STRUCT_ENUM_END

JSON_STRUCT_ENUM_START(listio_opcode)
  JSON_STRUCT_ENUM_ELEMENT(lio_read)  //LIO_READ
  JSON_STRUCT_ENUM_ELEMENT(lio_write) //LIO_WRITE
  JSON_STRUCT_ENUM_ELEMENT(unknown_listio_opcode)
JSON_STRUCT_ENUM_END

JSON_STRUCT_ENUM_START(listio_mode)
  JSON_STRUCT_ENUM_ELEMENT(lio_wait)   //LIO_WAIT
  JSON_STRUCT_ENUM_ELEMENT(lio_nowait) //LIO_NOWAIT
  JSON_STRUCT_ENUM_ELEMENT(unknown_listio_mode)
JSON_STRUCT_ENUM_END

JSON_STRUCT_ENUM_START(async_state)
  JSON_STRUCT_ENUM_ELEMENT(inprogress) //EINPROGRESS
  JSON_STRUCT_ENUM_ELEMENT(canceled)   //ECANCELED
  JSON_STRUCT_ENUM_ELEMENT(completed)  //0
  JSON_STRUCT_ENUM_ELEMENT(failed)
JSON_STRUCT_ENUM_END

JSON_STRUCT_ENUM_START(async_sync_mode)
  JSON_STRUCT_ENUM_ELEMENT(like_fdatasync) //O_DSYNC
  JSON_STRUCT_ENUM_ELEMENT(like_fsync)     //O_SYNC
  JSON_STRUCT_ENUM_ELEMENT(unknown_async_sync_mode)
JSON_STRUCT_ENUM_END

JSON_STRUCT_ENUM_START(async_cancel_state)
  JSON_STRUCT_ENUM_ELEMENT(is_canceled)     //AIO_CANCELED
  JSON_STRUCT_ENUM_ELEMENT(not_canceled) //AIO_NOTCANCELED
  JSON_STRUCT_ENUM_ELEMENT(allready_done)     //AIO_ALLDONE
  JSON_STRUCT_ENUM_ELEMENT(unknown_async_cancel_state)
JSON_STRUCT_ENUM_END

JSON_STRUCT_ENUM_START(boolean)
  JSON_STRUCT_ENUM_ELEMENT(true)
  JSON_STRUCT_ENUM_ELEMENT(false)
JSON_STRUCT_ENUM_END

/* struct for timeval */
JSON_STRUCT_START(json_timeval)
  JSON_STRUCT_LONG_INT(sec)
  JSON_STRUCT_LONG_INT(micro_sec)
JSON_STRUCT_END

/* struct for timespec */
JSON_STRUCT_START(json_timespec)
  JSON_STRUCT_LONG_INT(sec)
  JSON_STRUCT_LONG_INT(nano_sec)
JSON_STRUCT_END

/* struct for file_id */
JSON_STRUCT_START(file_id)
  JSON_STRUCT_DEV_T(device_id)
  JSON_STRUCT_INO_T(inode_nr)
JSON_STRUCT_END

/* struct for file open */
JSON_STRUCT_START(open_function)
  JSON_STRUCT_ENUM(access_mode, mode)
  JSON_STRUCT_ARRAY_BITFIELD(creation_flags, creation)
  JSON_STRUCT_ARRAY_BITFIELD(status_flags, status)
  JSON_STRUCT_ARRAY_BITFIELD(mode_flags, file_mode)
  JSON_STRUCT_CSTRING_P_CONST(file_name, MAXFILENAME)
  JSON_STRUCT_STRUCT(file_id, id)
JSON_STRUCT_END

JSON_STRUCT_START(mpi_open_function)
  JSON_STRUCT_ENUM(access_mode, mode)
  JSON_STRUCT_ARRAY_BITFIELD(creation_flags, creation)
  JSON_STRUCT_ARRAY_BITFIELD(status_flags, status)
  JSON_STRUCT_ARRAY_BITFIELD(mode_flags, file_mode)
  JSON_STRUCT_CSTRING_P_CONST(file_name, MAXFILENAME)
  JSON_STRUCT_STRUCT(file_id, id)
  JSON_STRUCT_KEY_VALUE_ARRAY(file_hints, MAX_MPI_FILE_HINTS, MAX_MPI_FILE_HINT_LENGTH)
JSON_STRUCT_END

JSON_STRUCT_START(mpi_delete_function)
  JSON_STRUCT_CSTRING_P_CONST(file_name, MAXFILENAME)
  JSON_STRUCT_STRUCT(file_id, id)
  JSON_STRUCT_KEY_VALUE_ARRAY(file_hints, MAX_MPI_FILE_HINTS, MAX_MPI_FILE_HINT_LENGTH)
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
  JSON_STRUCT_STRUCT(file_id, id)
JSON_STRUCT_END

/* struct for file open descriptor as stream */
JSON_STRUCT_START(fdopen_function)
  JSON_STRUCT_INT(descriptor)
  JSON_STRUCT_ENUM(access_mode, mode)
  JSON_STRUCT_ARRAY_BITFIELD(creation_flags, creation)
  JSON_STRUCT_ARRAY_BITFIELD(status_flags, status)
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
  JSON_STRUCT_SIZE_T(written_bytes)
JSON_STRUCT_END

/* struct for file pwrite */
JSON_STRUCT_START(pwrite_function)
  JSON_STRUCT_SIZE_T(written_bytes)
  JSON_STRUCT_OFF_T(position)
JSON_STRUCT_END

/* struct for file pwrite2 */
JSON_STRUCT_START(pwrite2_function)
  JSON_STRUCT_SIZE_T(written_bytes)
  JSON_STRUCT_OFF_T(position)
  JSON_STRUCT_ARRAY_BITFIELD(rwf_flags, flags)
JSON_STRUCT_END

/* struct for file read */
JSON_STRUCT_START(read_function)
  JSON_STRUCT_SIZE_T(read_bytes)
JSON_STRUCT_END

/* struct for file pread */
JSON_STRUCT_START(pread_function)
  JSON_STRUCT_SIZE_T(read_bytes)
  JSON_STRUCT_OFF_T(position)
JSON_STRUCT_END

/* struct for file pread2 */
JSON_STRUCT_START(pread2_function)
  JSON_STRUCT_SIZE_T(read_bytes)
  JSON_STRUCT_OFF_T(position)
  JSON_STRUCT_ARRAY_BITFIELD(rwf_flags, flags)
JSON_STRUCT_END

/* struct for file copy read */
JSON_STRUCT_START(copy_read_function)
  JSON_STRUCT_SIZE_T(read_bytes)
  JSON_STRUCT_OFF_T(position)
  JSON_STRUCT_ENUM(seek_where, relative_to)
  JSON_STRUCT_INT(to_file_descriptor)
JSON_STRUCT_END

/* struct for file copy write */
JSON_STRUCT_START(copy_write_function)
  JSON_STRUCT_SIZE_T(written_bytes)
  JSON_STRUCT_OFF_T(position)
  JSON_STRUCT_ENUM(seek_where, relative_to)
  JSON_STRUCT_INT(from_file_descriptor)
JSON_STRUCT_END

/* struct for file unget */
JSON_STRUCT_START(unget_function)
  JSON_STRUCT_INT(buffer_bytes)
JSON_STRUCT_END

/* struct for file position */
JSON_STRUCT_START(position_function)
  JSON_STRUCT_OFF_T(position)
JSON_STRUCT_END

/* struct for file positioning */
JSON_STRUCT_START(positioning_function)
  JSON_STRUCT_ENUM(seek_where, relative_to)
  JSON_STRUCT_OFF_T(offset)
JSON_STRUCT_END

/* struct for file lpositioning */
JSON_STRUCT_START(lpositioning_function)
  JSON_STRUCT_ENUM(seek_where, relative_to)
  JSON_STRUCT_OFF_T(offset)
  JSON_STRUCT_OFF_T(new_offset_relative_to_beginning_of_file)
JSON_STRUCT_END

/* struct for file buffer */
JSON_STRUCT_START(buffer_function)
  JSON_STRUCT_ENUM(buffer_mode, buffer_mode)
  JSON_STRUCT_SIZE_T(buffer_size)
JSON_STRUCT_END

/* struct for fileno */
JSON_STRUCT_START(fileno_function)
  JSON_STRUCT_INT(file_descriptor)
JSON_STRUCT_END

/* struct for file bufsize */
JSON_STRUCT_START(bufsize_function)
  JSON_STRUCT_SIZE_T(buffer_size)
JSON_STRUCT_END

/* struct for memory map */
JSON_STRUCT_START(memory_map_function)
  JSON_STRUCT_VOID_P(address)
  JSON_STRUCT_SIZE_T(length)
  JSON_STRUCT_OFF_T(offset)
  JSON_STRUCT_ARRAY_BITFIELD(memory_protection_flags, protection_flags)
  JSON_STRUCT_ARRAY_BITFIELD(memory_map_flags, map_flags)
JSON_STRUCT_END

/* struct for memory sync */
JSON_STRUCT_START(memory_sync_function)
  JSON_STRUCT_ARRAY_BITFIELD(memory_sync_flags, sync_flags)
JSON_STRUCT_END

/* struct for memory remap */
#ifdef HAVE_MREMAP
JSON_STRUCT_START(memory_remap_function)
  JSON_STRUCT_VOID_P(new_address)
  JSON_STRUCT_SIZE_T(new_length)
  JSON_STRUCT_ARRAY_BITFIELD(memory_remap_flags, remap_flags)
JSON_STRUCT_END
#endif

/* struct for memory madvise */
JSON_STRUCT_START(memory_madvise_function)
  JSON_STRUCT_ENUM(madvice_advice, advice)
JSON_STRUCT_END

/* struct for memory posix madvise */
JSON_STRUCT_START(memory_posix_madvise_function)
  JSON_STRUCT_ENUM(posix_madvice_advice, advice)
JSON_STRUCT_END

/* struct for file select */
JSON_STRUCT_START(select_function)
  JSON_STRUCT_STRUCT(json_timeval, timeout)
  JSON_STRUCT_FD_SET_P(files_waiting_for_read)
  JSON_STRUCT_FD_SET_P(files_waiting_for_write)
  JSON_STRUCT_FD_SET_P(files_waiting_for_except)
  JSON_STRUCT_FD_SET_P(files_ready_for_read)
  JSON_STRUCT_FD_SET_P(files_ready_for_write)
  JSON_STRUCT_FD_SET_P(files_ready_for_except)
JSON_STRUCT_END

/* struct for file asynchronous read */
JSON_STRUCT_START(asynchronous_read_function)
  JSON_STRUCT_VOID_P(async)
  JSON_STRUCT_SIZE_T(bytes_to_read)
  JSON_STRUCT_OFF_T(position)
  JSON_STRUCT_INT(lower_prio)
JSON_STRUCT_END

/* struct for file asynchronous write */
JSON_STRUCT_START(asynchronous_write_function)
  JSON_STRUCT_VOID_P(async)
  JSON_STRUCT_SIZE_T(bytes_to_write)
  JSON_STRUCT_OFF_T(position)
  JSON_STRUCT_INT(lower_prio)
JSON_STRUCT_END

/* struct for file asynchronous listio */
JSON_STRUCT_START(asynchronous_listio_function)
  JSON_STRUCT_ENUM(listio_mode, mode)
  JSON_STRUCT_ENUM(listio_opcode, opcode)
  JSON_STRUCT_VOID_P_START(request_data)
    JSON_STRUCT_VOID_P_ELEMENT(request_data, asynchronous_read_function)
    JSON_STRUCT_VOID_P_ELEMENT(request_data, asynchronous_write_function)
  JSON_STRUCT_VOID_P_END(request_data)
JSON_STRUCT_END

/* struct for file asynchronous error */
JSON_STRUCT_START(asynchronous_error_function)
  JSON_STRUCT_ENUM(async_state, state)
JSON_STRUCT_END

/* struct for file asynchronous return */
JSON_STRUCT_START(asynchronous_return_function)
  JSON_STRUCT_SSIZE_T(return_value)
JSON_STRUCT_END

/* struct for file asynchronous sync */
JSON_STRUCT_START(asynchronous_sync_function)
  JSON_STRUCT_ENUM(async_sync_mode, sync_mode)
JSON_STRUCT_END

/* struct for file asynchronous suspend */
JSON_STRUCT_START(asynchronous_suspend_function)
  JSON_STRUCT_STRUCT(json_timespec, timeout)
JSON_STRUCT_END

/* struct for file asynchronous cancel */
JSON_STRUCT_START(asynchronous_cancel_function)
  JSON_STRUCT_VOID_P_CONST(async)
  JSON_STRUCT_ENUM(async_cancel_state, state)
JSON_STRUCT_END

/* struct for file asynchronous init */
JSON_STRUCT_START(asynchronous_init_function)
  JSON_STRUCT_INT(max_threads)
  JSON_STRUCT_INT(max_simultaneous_requests)
  JSON_STRUCT_INT(seconds_idle_before_terminate)
JSON_STRUCT_END

/* struct for file dlopen */
JSON_STRUCT_START(dlopen_function)
  JSON_STRUCT_CSTRING_P_CONST(file_name, MAXFILENAME)
  JSON_STRUCT_ARRAY_BITFIELD(dlopen_flags, dl_flags)
  JSON_STRUCT_STRUCT(file_id, id)
JSON_STRUCT_END

#ifdef HAVE_DLMOPEN
JSON_STRUCT_START(so_namespace_id)
  JSON_STRUCT_LMID_T(id)
JSON_STRUCT_END
#endif

JSON_STRUCT_ENUM_START(so_namespace_mode_enum)
  JSON_STRUCT_ENUM_ELEMENT(initial_namespace) //LM_ID_BASE
  JSON_STRUCT_ENUM_ELEMENT(new_namespace)     //LM_ID_NEWLM
  JSON_STRUCT_ENUM_ELEMENT(unknown_so_namespace_mode)
JSON_STRUCT_ENUM_END

JSON_STRUCT_START(so_namespace_mode)
  JSON_STRUCT_ENUM(so_namespace_mode_enum, mode)
JSON_STRUCT_END

/* struct for file dlmopen */
#ifdef HAVE_DLMOPEN
JSON_STRUCT_START(dlmopen_function)
  JSON_STRUCT_VOID_P_START(so_namespace)
    JSON_STRUCT_VOID_P_ELEMENT(so_namespace, so_namespace_id)
    JSON_STRUCT_VOID_P_ELEMENT(so_namespace, so_namespace_mode)
  JSON_STRUCT_VOID_P_END(so_namespace)
  JSON_STRUCT_CSTRING_P_CONST(file_name, MAXFILENAME)
  JSON_STRUCT_ARRAY_BITFIELD(dlopen_flags, dl_flags)
  JSON_STRUCT_STRUCT(file_id, id)
JSON_STRUCT_END
#endif

/* struct for dup */
JSON_STRUCT_START(dup_function)
  JSON_STRUCT_INT(new_descriptor)
JSON_STRUCT_END

/* struct for dup3 */
JSON_STRUCT_START(dup3_function)
  JSON_STRUCT_INT(new_descriptor)
  JSON_STRUCT_ARRAY_BITFIELD(creation_flags, creation)
JSON_STRUCT_END

JSON_STRUCT_ENUM_START(fcntl_cmd)
  JSON_STRUCT_ENUM_ELEMENT(dupfd)            //F_DUPFD
  JSON_STRUCT_ENUM_ELEMENT(dupfd_cloexec)    //F_DUPFD_CLOEXEC
  JSON_STRUCT_ENUM_ELEMENT(getfd)            //F_GETFD
  JSON_STRUCT_ENUM_ELEMENT(setfd)            //F_SETFD
  JSON_STRUCT_ENUM_ELEMENT(getfl)            //F_GETFL
  JSON_STRUCT_ENUM_ELEMENT(setfl)            //F_SETFL
  JSON_STRUCT_ENUM_ELEMENT(setlk)            //F_SETLK
  JSON_STRUCT_ENUM_ELEMENT(setlkw)           //F_SETLKW
  JSON_STRUCT_ENUM_ELEMENT(getlk)            //F_GETLK
  JSON_STRUCT_ENUM_ELEMENT(ofd_setlk)        //F_OFD_SETLK
  JSON_STRUCT_ENUM_ELEMENT(ofd_setlkw)       //F_OFD_SETLKW
  JSON_STRUCT_ENUM_ELEMENT(ofd_getlk)        //F_OFD_GETLK
  JSON_STRUCT_ENUM_ELEMENT(getown)           //F_GETOWN
  JSON_STRUCT_ENUM_ELEMENT(setown)           //F_SETOWN
  JSON_STRUCT_ENUM_ELEMENT(getown_ex)        //F_GETOWN_EX
  JSON_STRUCT_ENUM_ELEMENT(setown_ex)        //F_SETOWN_EX
  JSON_STRUCT_ENUM_ELEMENT(getsig)           //F_GETSIG
  JSON_STRUCT_ENUM_ELEMENT(setsig)           //F_SETSIG
  JSON_STRUCT_ENUM_ELEMENT(setlease)         //F_SETLEASE
  JSON_STRUCT_ENUM_ELEMENT(getlease)         //F_GETLEASE
  JSON_STRUCT_ENUM_ELEMENT(notify)           //F_NOTIFY
  JSON_STRUCT_ENUM_ELEMENT(setpipe_sz)       //F_SETPIPE_SZ
  JSON_STRUCT_ENUM_ELEMENT(getpipe_sz)       //F_GETPIPE_SZ
  JSON_STRUCT_ENUM_ELEMENT(add_seals)        //F_ADD_SEALS
  JSON_STRUCT_ENUM_ELEMENT(get_seals)        //F_GET_SEALS
  JSON_STRUCT_ENUM_ELEMENT(get_rw_hint)      //F_GET_RW_HINT
  JSON_STRUCT_ENUM_ELEMENT(set_rw_hint)      //F_SET_RW_HINT
  JSON_STRUCT_ENUM_ELEMENT(get_file_rw_hint) //F_GET_FILE_RW_HINT
  JSON_STRUCT_ENUM_ELEMENT(set_file_rw_hint) //F_SET_FILE_RW_HINT
  JSON_STRUCT_ENUM_ELEMENT(unknown_fcntl_cmd)
JSON_STRUCT_ENUM_END

JSON_STRUCT_ARRAY_BITFIELD_START(file_descriptor_flags)
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(cloexec) //FD_CLOEXEC
JSON_STRUCT_ARRAY_BITFIELD_END

JSON_STRUCT_START(fcntl_fd_function)
  JSON_STRUCT_ARRAY_BITFIELD(file_descriptor_flags, fd_flags)
JSON_STRUCT_END

JSON_STRUCT_START(fcntl_fl_function)
  JSON_STRUCT_ENUM(access_mode, mode)
  JSON_STRUCT_ARRAY_BITFIELD(creation_flags, creation)
  JSON_STRUCT_ARRAY_BITFIELD(status_flags, status)
JSON_STRUCT_END

JSON_STRUCT_ENUM_START(lock_type)
  JSON_STRUCT_ENUM_ELEMENT(read_lock)  //F_RDLCK
  JSON_STRUCT_ENUM_ELEMENT(write_lock) //F_WRLCK
  JSON_STRUCT_ENUM_ELEMENT(unlock)     //F_UNLCK
  JSON_STRUCT_ENUM_ELEMENT(unknown_lock_type)
JSON_STRUCT_ENUM_END

JSON_STRUCT_START(fcntl_flock)
  JSON_STRUCT_ENUM(lock_type, type)
  JSON_STRUCT_ENUM(seek_where, relative_to)
  JSON_STRUCT_OFF_T(start)
  JSON_STRUCT_OFF_T(len)
  JSON_STRUCT_PID_T(pid)
JSON_STRUCT_END

JSON_STRUCT_ENUM_START(owner_type)
  JSON_STRUCT_ENUM_ELEMENT(owner_thread)        //F_OWNER_TID
  JSON_STRUCT_ENUM_ELEMENT(owner_process)       //F_OWNER_PID
  JSON_STRUCT_ENUM_ELEMENT(owner_process_group) //F_OWNER_PGRP
  JSON_STRUCT_ENUM_ELEMENT(unknown_owner_type)
JSON_STRUCT_ENUM_END

JSON_STRUCT_START(fcntl_own)
  JSON_STRUCT_ENUM(owner_type, type)
  JSON_STRUCT_PID_T(id)
JSON_STRUCT_END

JSON_STRUCT_START(fcntl_sig)
  JSON_STRUCT_INT(signal)
JSON_STRUCT_END

JSON_STRUCT_ENUM_START(lease_type)
  JSON_STRUCT_ENUM_ELEMENT(read_lease)   //F_RDLCK
  JSON_STRUCT_ENUM_ELEMENT(write_lease)  //F_WRLCK
  JSON_STRUCT_ENUM_ELEMENT(unlock_lease) //F_UNLCK
  JSON_STRUCT_ENUM_ELEMENT(unknown_lease_type)
JSON_STRUCT_ENUM_END

JSON_STRUCT_START(fcntl_lease)
  JSON_STRUCT_ENUM(lease_type, type)
JSON_STRUCT_END

JSON_STRUCT_ARRAY_BITFIELD_START(directory_notify_flags)
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(directory_access)    //DN_ACCESS
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(directory_modify)    //DN_MODIFY
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(directory_create)    //DN_CREATE
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(directory_delete)    //DN_DELETE
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(directory_rename)    //DN_RENAME
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(directory_attribute) //DN_ATTRIB
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(directory_multishot) //DN_MULTISHOT
JSON_STRUCT_ARRAY_BITFIELD_END

JSON_STRUCT_START(fcntl_dnotify)
  JSON_STRUCT_ARRAY_BITFIELD(directory_notify_flags, flags)
JSON_STRUCT_END

JSON_STRUCT_START(fcntl_pipe_size)
  JSON_STRUCT_INT(bytes)
JSON_STRUCT_END

JSON_STRUCT_ARRAY_BITFIELD_START(seal_flags)
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(seal_seal)   //F_SEAL_SEAL
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(seal_shrink) //F_SEAL_SHRINK
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(seal_grow)   //F_SEAL_GROW
  JSON_STRUCT_ARRAY_BITFIELD_ELEMENT(seal_write)  //F_SEAL_WRITE
JSON_STRUCT_ARRAY_BITFIELD_END

JSON_STRUCT_START(fcntl_seal)
  JSON_STRUCT_ARRAY_BITFIELD(seal_flags, flags)
JSON_STRUCT_END

JSON_STRUCT_ENUM_START(hint_write_life)
  JSON_STRUCT_ENUM_ELEMENT(hint_write_life_not_set) //RWH_WRITE_LIFE_NOT_SET
  JSON_STRUCT_ENUM_ELEMENT(hint_write_life_none)    //RWH_WRITE_LIFE_NONE
  JSON_STRUCT_ENUM_ELEMENT(hint_write_life_short)   //RWH_WRITE_LIFE_SHORT
  JSON_STRUCT_ENUM_ELEMENT(hint_write_life_medium)  //RWH_WRITE_LIFE_MEDIUM
  JSON_STRUCT_ENUM_ELEMENT(hint_write_life_long)    //RWH_WRITE_LIFE_LONG
  JSON_STRUCT_ENUM_ELEMENT(hint_write_life_extreme) //RWH_WRITE_LIFE_EXTREME
  JSON_STRUCT_ENUM_ELEMENT(unknown_hint_write_life)
JSON_STRUCT_ENUM_END

JSON_STRUCT_START(fcntl_hint)
  JSON_STRUCT_ENUM(hint_write_life, hint)
JSON_STRUCT_END

/* struct for fcntl */
JSON_STRUCT_START(fcntl_function)
  JSON_STRUCT_ENUM(fcntl_cmd, cmd)
  JSON_STRUCT_VOID_P_START(cmd_data)
    JSON_STRUCT_VOID_P_ELEMENT(cmd_data, dup_function)
    JSON_STRUCT_VOID_P_ELEMENT(cmd_data, fcntl_fd_function)
    JSON_STRUCT_VOID_P_ELEMENT(cmd_data, fcntl_fl_function)
    JSON_STRUCT_VOID_P_ELEMENT(cmd_data, fcntl_flock)
    JSON_STRUCT_VOID_P_ELEMENT(cmd_data, fcntl_own)
    JSON_STRUCT_VOID_P_ELEMENT(cmd_data, fcntl_sig)
    JSON_STRUCT_VOID_P_ELEMENT(cmd_data, fcntl_lease)
    JSON_STRUCT_VOID_P_ELEMENT(cmd_data, fcntl_dnotify)
    JSON_STRUCT_VOID_P_ELEMENT(cmd_data, fcntl_pipe_size)
    JSON_STRUCT_VOID_P_ELEMENT(cmd_data, fcntl_seal)
    JSON_STRUCT_VOID_P_ELEMENT(cmd_data, fcntl_hint)
  JSON_STRUCT_VOID_P_END(cmd_data)
JSON_STRUCT_END

/* struct for file-pair functions (pipe, pipe2) */
JSON_STRUCT_START(file_pair)
  JSON_STRUCT_INT(descriptor1)
  JSON_STRUCT_INT(descriptor2)
JSON_STRUCT_END

/* struct for socketpair */
JSON_STRUCT_START(socketpair_function)
  JSON_STRUCT_ENUM(boolean, connection_based)
  JSON_STRUCT_INT(descriptor1)
  JSON_STRUCT_INT(descriptor2)
JSON_STRUCT_END

/* struct for socket */
JSON_STRUCT_START(socket_function)
  JSON_STRUCT_ENUM(boolean, connection_based)
JSON_STRUCT_END

/* struct for fork */
JSON_STRUCT_START(fork_function)
  JSON_STRUCT_PID_T(pid)
JSON_STRUCT_END

/* struct for readdir */
JSON_STRUCT_START(readdir_function)
  JSON_STRUCT_CSTRING_P_CONST(file_name, MAXFILENAME)
JSON_STRUCT_END

/* struct for dirfd */
JSON_STRUCT_START(dirfd_function)
  JSON_STRUCT_INT(descriptor)
JSON_STRUCT_END

/* struct for bind and connect */
JSON_STRUCT_START(sockaddr_function)
  JSON_STRUCT_SA_FAMILY_T(family)
  JSON_STRUCT_CSTRING_P(address, MAX_SOCKADDR_LENGTH * 2 + 1) // *2 for hex and +1 for terminating '\0'
JSON_STRUCT_END

/* struct for sendmsg and recvmsg */
JSON_STRUCT_START(msg_function)
  JSON_STRUCT_STRUCT_P(sockaddr_function, sockaddr)
  JSON_STRUCT_INT_ARRAY(descriptors, MAX_MSG_FILE_DESCRIPTORS)
JSON_STRUCT_END

/* struct for sendmmsg and recvmmsg */
JSON_STRUCT_START(mmsg_function)
  JSON_STRUCT_STRUCT_ARRAY(msg_function, messages, MAX_MMSG_MESSAGES)
JSON_STRUCT_END

/* struct for accept */
JSON_STRUCT_START(accept_function)
  JSON_STRUCT_INT(new_descriptor)
JSON_STRUCT_END

/* struct for dataype size */
JSON_STRUCT_START(mpi_immediate)
  JSON_STRUCT_SIZE_T(datatype_size)
  JSON_STRUCT_INT(request_id)
JSON_STRUCT_END

/* struct for dataype size */
JSON_STRUCT_START(mpi_immediate_at)
  JSON_STRUCT_SIZE_T(datatype_size)
  JSON_STRUCT_INT(request_id)
  JSON_STRUCT_OFF_T(position)
JSON_STRUCT_END

/* struct for dataype size */
JSON_STRUCT_START(mpi_wait)
  JSON_STRUCT_INT(count_datatypes)
JSON_STRUCT_END




/* struct for additional wrapper informations */
#ifdef LOG_WRAPPER_TIME
JSON_STRUCT_START(wrapper_data)
  JSON_STRUCT_U_INT64_T(time_start)
  JSON_STRUCT_U_INT64_T(time_end)
JSON_STRUCT_END
#endif

/* basic struct for every call */
JSON_STRUCT_START(basic)
  JSON_STRUCT_CSTRING_P(hostname, HOST_NAME_MAX)
  JSON_STRUCT_PID_T(process_id)
  JSON_STRUCT_PID_T(thread_id)
  // ToDo: function_name as CSTRING_P ?
  JSON_STRUCT_CSTRING(function_name, MAX_FUNCTION_NAME)
  JSON_STRUCT_U_INT64_T(time_start)
  JSON_STRUCT_U_INT64_T(time_end)
  JSON_STRUCT_ENUM(read_write_state, return_state)
  JSON_STRUCT_STRUCT_P(errno_detail, return_state_detail)
  JSON_STRUCT_MALLOC_STRING_ARRAY(stacktrace_symbols, MAX_STACKTRACE_DEPTH, MAX_STACKTRACE_ENTRY_LENGTH)
  JSON_STRUCT_MALLOC_PTR_ARRAY(stacktrace_pointer, MAX_STACKTRACE_DEPTH)
#ifdef LOG_WRAPPER_TIME
  JSON_STRUCT_STRUCT(wrapper_data, wrapper)
#endif
  JSON_STRUCT_VOID_P_START(file_type)
    JSON_STRUCT_VOID_P_ELEMENT(file_type, file_stream)
    JSON_STRUCT_VOID_P_ELEMENT(file_type, file_dir)
    JSON_STRUCT_VOID_P_ELEMENT(file_type, file_descriptor)
    JSON_STRUCT_VOID_P_ELEMENT(file_type, file_memory)
    JSON_STRUCT_VOID_P_ELEMENT(file_type, file_async)
    JSON_STRUCT_VOID_P_ELEMENT(file_type, file_mpi)
    JSON_STRUCT_VOID_P_ELEMENT(file_type, shared_library)
    JSON_STRUCT_VOID_P_ELEMENT(file_type, request_mpi)
  JSON_STRUCT_VOID_P_END(file_type)
  // ToDo: new field for boolean which shows if file position has changed (e.g. copy_file_range don't change file position)
  // or corrupted (e.g. async functions)
  JSON_STRUCT_VOID_P_START(function_data)
    JSON_STRUCT_VOID_P_ELEMENT(function_data, fork_function)
    JSON_STRUCT_VOID_P_ELEMENT(function_data, open_function)
    JSON_STRUCT_VOID_P_ELEMENT(function_data, openat_function)
    JSON_STRUCT_VOID_P_ELEMENT(function_data, fdopen_function)
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
    JSON_STRUCT_VOID_P_ELEMENT(function_data, unget_function)
    JSON_STRUCT_VOID_P_ELEMENT(function_data, position_function)
    JSON_STRUCT_VOID_P_ELEMENT(function_data, positioning_function)
    JSON_STRUCT_VOID_P_ELEMENT(function_data, lpositioning_function)
    JSON_STRUCT_VOID_P_ELEMENT(function_data, buffer_function)
    JSON_STRUCT_VOID_P_ELEMENT(function_data, fileno_function)
    JSON_STRUCT_VOID_P_ELEMENT(function_data, bufsize_function)
    JSON_STRUCT_VOID_P_ELEMENT(function_data, memory_map_function)
    JSON_STRUCT_VOID_P_ELEMENT(function_data, memory_sync_function)
#ifdef HAVE_MREMAP
    JSON_STRUCT_VOID_P_ELEMENT(function_data, memory_remap_function)
#endif
    JSON_STRUCT_VOID_P_ELEMENT(function_data, memory_madvise_function)
    JSON_STRUCT_VOID_P_ELEMENT(function_data, select_function)
    JSON_STRUCT_VOID_P_ELEMENT(function_data, memory_posix_madvise_function)
    JSON_STRUCT_VOID_P_ELEMENT(function_data, asynchronous_read_function)
    JSON_STRUCT_VOID_P_ELEMENT(function_data, asynchronous_write_function)
    JSON_STRUCT_VOID_P_ELEMENT(function_data, asynchronous_listio_function)
    JSON_STRUCT_VOID_P_ELEMENT(function_data, asynchronous_error_function)
    JSON_STRUCT_VOID_P_ELEMENT(function_data, asynchronous_return_function)
    JSON_STRUCT_VOID_P_ELEMENT(function_data, asynchronous_sync_function)
    JSON_STRUCT_VOID_P_ELEMENT(function_data, asynchronous_suspend_function)
    JSON_STRUCT_VOID_P_ELEMENT(function_data, asynchronous_cancel_function)
    JSON_STRUCT_VOID_P_ELEMENT(function_data, asynchronous_init_function)
    JSON_STRUCT_VOID_P_ELEMENT(function_data, dlopen_function)
#ifdef HAVE_DLMOPEN
    JSON_STRUCT_VOID_P_ELEMENT(function_data, dlmopen_function)
#endif
    JSON_STRUCT_VOID_P_ELEMENT(function_data, dup_function)
    JSON_STRUCT_VOID_P_ELEMENT(function_data, dup3_function)
    JSON_STRUCT_VOID_P_ELEMENT(function_data, fcntl_function)
    JSON_STRUCT_VOID_P_ELEMENT(function_data, file_pair)
    JSON_STRUCT_VOID_P_ELEMENT(function_data, readdir_function)
    JSON_STRUCT_VOID_P_ELEMENT(function_data, dirfd_function)
    JSON_STRUCT_VOID_P_ELEMENT(function_data, msg_function)
    JSON_STRUCT_VOID_P_ELEMENT(function_data, mmsg_function)
    JSON_STRUCT_VOID_P_ELEMENT(function_data, sockaddr_function)
    JSON_STRUCT_VOID_P_ELEMENT(function_data, accept_function)
    JSON_STRUCT_VOID_P_ELEMENT(function_data, socketpair_function)
    JSON_STRUCT_VOID_P_ELEMENT(function_data, socket_function)
    JSON_STRUCT_VOID_P_ELEMENT(function_data, mpi_open_function)
    JSON_STRUCT_VOID_P_ELEMENT(function_data, mpi_immediate)
    JSON_STRUCT_VOID_P_ELEMENT(function_data, mpi_wait)
    JSON_STRUCT_VOID_P_ELEMENT(function_data, mpi_delete_function)
    JSON_STRUCT_VOID_P_ELEMENT(function_data, mpi_immediate_at)
  JSON_STRUCT_VOID_P_END(function_data)
JSON_STRUCT_END

/* filesystem description */
JSON_STRUCT_START(filesystem)
  JSON_STRUCT_DEV_T(device_id)
  JSON_STRUCT_CSTRING_P(name, MAXFILENAME)
  JSON_STRUCT_CSTRING_P(path_prefix, MAXFILENAME)
  JSON_STRUCT_CSTRING_P(mount_type, MAXFILENAME)
  JSON_STRUCT_CSTRING_P(mount_options, MAXFILENAME)
  JSON_STRUCT_INT(dump_frequency_in_days)
  JSON_STRUCT_INT(pass_number_on_parallel_fsck)
JSON_STRUCT_END

/* working dir */
JSON_STRUCT_START(working_dir)
  JSON_STRUCT_U_INT64_T(time)
  JSON_STRUCT_CSTRING_P(hostname, HOST_NAME_MAX)
  JSON_STRUCT_PID_T(process_id)
  JSON_STRUCT_CSTRING_P(dir, MAXFILENAME)
JSON_STRUCT_END

#endif
