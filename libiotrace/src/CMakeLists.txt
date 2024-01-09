
# Please keep in alphabetical order
set(LIBIOTRACE_SOURCE
    common/utils.c
    common/gettime.c
    event.c
    io_log_file.c
    libiotrace_defines_utils.c
    libiotrace_functions.c
    libs/llhttp/api.c
    libs/llhttp/http.c
    libs/llhttp/llhttp.c
    os.c
)

set(LIBIOTRACE_COMPILE_OPTIONS
    -D_GNU_SOURCE                   # needed for the RTLD_NEXT macro and other GNU functions
)

set(LIBIOTRACE_SHARED_LINK_OPTIONS
    ${CMAKE_DL_LIBS}                    # Needed for dlsym-function
    pthread                             # Needed for pthread_atfork() and pthread_create()
)
set(LIBIOTRACE_STATIC_LINK_OPTIONS ${CMAKE_DL_LIBS} pthread)


# -----------------------------------------------------------------------------
# 'HAVEs'
# -----------------------------------------------------------------------------
include(CheckIncludeFiles)

check_include_files("stdbool.h" HAVE_STDBOOL_H)
check_include_files("stdlib.h" HAVE_STDLIB_H)
check_include_files("stdint.h" HAVE_STDINT_H)
check_include_files("sys/fcntl.h" HAVE_SYS_FCNTL_H)
check_include_files("sys/time.h" HAVE_SYS_TIME_H)
check_include_files("sys/types.h" HAVE_SYS_TYPES_H)
check_include_files("sys/stat.h" HAVE_SYS_STAT_H)
check_include_files("sys/syscall.h" HAVE_SYS_SYSCALL_H)
check_include_files("unistd.h" HAVE_UNISTD_H)
check_include_files("pthread.h" HAVE_PTHREAD_H)
check_include_files("aio.h" HAVE_AIO_H)
check_include_files("getmnt.h" HAVE_GETMNT_H)
check_include_files("sched.h" HAVE_SCHED_H)
check_include_files("mntent.h" HAVE_MNTENT_H)
check_include_files("utmp.h" HAVE_UTMP_H)
check_include_files("utmpx.h" HAVE_UTMPX_H)

# In order to wrap functions which are replacing POSIX calls to check for security-bugs (_FORTIFY_SOURCE)
# we need to turn on Optimization to make those visible (plus _FORTIFY_SOURCE itself)
list(APPEND CMAKE_REQUIRED_FLAGS       -O2)
list(APPEND CMAKE_REQUIRED_DEFINITIONS -D_GNU_SOURCE -D__USE_LARGEFILE64 -D_FORTIFY_SOURCE=2)

include(CheckSymbolExists)

check_symbol_exists(__open "fcntl.h" HAVE___OPEN)              # TODO: DOESN'T WORK SOMEHOW ??
check_symbol_exists(__open64 "fcntl.h" HAVE___OPEN64)          # TODO: DOESN'T WORK SOMEHOW ??
check_symbol_exists(__open_2 "fcntl.h" HAVE___OPEN_2)
check_symbol_exists(__open64_2 "fcntl.h" HAVE___OPEN64_2)
check_symbol_exists(open64 "fcntl.h" HAVE_OPEN64)
check_symbol_exists(openat "fcntl.h" HAVE_OPENAT)
check_symbol_exists(creat64 "fcntl.h" HAVE_CREAT64)
check_symbol_exists(pread "unistd.h" HAVE_PREAD)
check_symbol_exists(pread64 "unistd.h" HAVE_PREAD64)
check_symbol_exists(pwrite "unistd.h" HAVE_PWRITE)
check_symbol_exists(pwrite64 "unistd.h" HAVE_PWRITE64)
check_symbol_exists(lseek64 "unistd.h" HAVE_LSEEK64)
check_symbol_exists(readv "sys/uio.h" HAVE_READV)
check_symbol_exists(writev "sys/uio.h" HAVE_WRITEV)
check_symbol_exists(preadv "sys/uio.h" HAVE_PREADV)
check_symbol_exists(preadv64 "sys/uio.h" HAVE_PREADV64)
check_symbol_exists(pwritev "sys/uio.h" HAVE_PWRITEV)
check_symbol_exists(pwritev64 "sys/uio.h" HAVE_PWRITEV64)
check_symbol_exists(preadv2 "sys/uio.h" HAVE_PREADV2)
check_symbol_exists(preadv64v2 "sys/uio.h" HAVE_PREADV64V2)
check_symbol_exists(pwritev2 "sys/uio.h" HAVE_PWRITEV2)
check_symbol_exists(pwritev64v2 "sys/uio.h" HAVE_PWRITEV64V2)
check_symbol_exists(copy_file_range "unistd.h" HAVE_COPY_FILE_RANGE)
check_symbol_exists(mmap "sys/mman.h" HAVE_MMAP)
check_symbol_exists(mmap64 "sys/mman.h" HAVE_MMAP64)
check_symbol_exists(munmap "sys/mman.h" HAVE_MUNMAP)
check_symbol_exists(msync "sys/mman.h" HAVE_MSYNC)
check_symbol_exists(mremap "sys/mman.h" HAVE_MREMAP)
check_symbol_exists(fopen64 "stdio.h" HAVE_FOPEN64)
check_symbol_exists(freopen64 "stdio.h" HAVE_FREOPEN64)
check_symbol_exists(fdopen "stdio.h" HAVE_FDOPEN)
check_symbol_exists(fcloseall "stdio.h" HAVE_FCLOSEALL)
check_symbol_exists(flockfile "stdio.h" HAVE_FLOCKFILE)
check_symbol_exists(ftrylockfile "stdio.h" HAVE_FTRYLOCKFILE)
check_symbol_exists(funlockfile "stdio.h" HAVE_FUNLOCKFILE)
check_symbol_exists(fwide "wchar.h" HAVE_FWIDE)
check_symbol_exists(fputc_unlocked "stdio.h" HAVE_FPUTC_UNLOCKED)
check_symbol_exists(fputwc_unlocked "wchar.h" HAVE_FPUTWC_UNLOCKED)
check_symbol_exists(putc_unlocked "stdio.h" HAVE_PUTC_UNLOCKED)
check_symbol_exists(putwc_unlocked "wchar.h" HAVE_PUTWC_UNLOCKED)
check_symbol_exists(fputs_unlocked "stdio.h" HAVE_FPUTS_UNLOCKED)
check_symbol_exists(fputws_unlocked "wchar.h" HAVE_FPUTWS_UNLOCKED)
check_symbol_exists(putw "stdio.h" HAVE_PUTW)
check_symbol_exists(fgetc_unlocked "stdio.h" HAVE_FGETC_UNLOCKED)
check_symbol_exists(fgetwc_unlocked "wchar.h" HAVE_FGETWC_UNLOCKED)
check_symbol_exists(getc_unlocked "stdio.h" HAVE_GETC_UNLOCKED)
check_symbol_exists(getwc_unlocked "wchar.h" HAVE_GETWC_UNLOCKED)
check_symbol_exists(getw "stdio.h" HAVE_GETW)
check_symbol_exists(getline "stdio.h" HAVE_GETLINE)
check_symbol_exists(getdelim "stdio.h" HAVE_GETDELIM)
check_symbol_exists(fgets_unlocked "stdio.h" HAVE_FGETS_UNLOCKED)
check_symbol_exists(fgetws_unlocked "wchar.h" HAVE_FGETWS_UNLOCKED)
check_symbol_exists(fread_unlocked "stdio.h" HAVE_FREAD_UNLOCKED)
check_symbol_exists(fwrite_unlocked "stdio.h" HAVE_FWRITE_UNLOCKED)
check_symbol_exists(fwprintf "wchar.h" HAVE_FWPRINTF)
check_symbol_exists(vfwprintf "wchar.h" HAVE_VFWPRINTF)
check_symbol_exists(fwscanf "wchar.h" HAVE_FWSCANF)
check_symbol_exists(vfscanf "stdio.h" HAVE_VFSCANF)
check_symbol_exists(vfwscanf "wchar.h" HAVE_VFWSCANF)
check_symbol_exists(feof_unlocked "stdio.h" HAVE_FEOF_UNLOCKED)
check_symbol_exists(ferror_unlocked "stdio.h" HAVE_FERROR_UNLOCKED)
check_symbol_exists(clearerr_unlocked "stdio.h" HAVE_CLEARERR_UNLOCKED)
check_symbol_exists(ftello "stdio.h" HAVE_FTELLO)
check_symbol_exists(ftello64 "stdio.h" HAVE_FTELLO64)
check_symbol_exists(fseeko "stdio.h" HAVE_FSEEKO)
check_symbol_exists(fseeko64 "stdio.h" HAVE_FSEEKO64)
check_symbol_exists(fgetpos64 "stdio.h" HAVE_FGETPOS64)
check_symbol_exists(fsetpos64 "stdio.h" HAVE_FSETPOS64)
check_symbol_exists(fflush_unlocked "stdio.h" HAVE_FFLUSH_UNLOCKED)
check_symbol_exists(setbuffer "stdio.h" HAVE_SETBUFFER)
check_symbol_exists(setlinebuf "stdio.h" HAVE_SETLINEBUF)
check_symbol_exists(madvise "sys/mman.h" HAVE_MADVISE)
check_symbol_exists(posix_madvise "sys/mman.h" HAVE_POSIX_MADVISE)
check_symbol_exists(sync "unistd.h" HAVE_SYNC)
check_symbol_exists(syncfs "unistd.h" HAVE_SYNCFS)
check_symbol_exists(fsync "unistd.h" HAVE_FSYNC)
check_symbol_exists(fdatasync "unistd.h" HAVE_FDATASYNC)
check_symbol_exists(fileno "stdio.h" HAVE_FILENO)
check_symbol_exists(dup3 "unistd.h" HAVE_DUP3)
check_symbol_exists(accept4 "sys/socket.h" HAVE_ACCEPT4)
check_symbol_exists(pipe2 "unistd.h" HAVE_PIPE2)
check_symbol_exists(memfd_create "sys/memfd.h;sys/mman.h" HAVE_MEMFD_CREATE)
check_symbol_exists(tmpfile64 "stdio.h" HAVE_TMPFILE64)
check_symbol_exists(epoll_create "sys/epoll.h" HAVE_EPOLL_CREATE)
check_symbol_exists(epoll_create1 "sys/epoll.h" HAVE_EPOLL_CREATE1)
check_symbol_exists(mkstemp "stdlib.h" HAVE_MKSTEMP)
check_symbol_exists(mkostemp "stdlib.h" HAVE_MKOSTEMP)
check_symbol_exists(mkstemps "stdlib.h" HAVE_MKSTEMPS)
check_symbol_exists(mkostemps "stdlib.h" HAVE_MKOSTEMPS)
check_symbol_exists(eventfd "sys/eventfd.h" HAVE_EVENTFD)
check_symbol_exists(inotify_init "sys/inotify.h" HAVE_INOTIFY_INIT)
check_symbol_exists(inotify_init1 "sys/inotify.h" HAVE_INOTIFY_INIT1)
check_symbol_exists(vfork "unistd.h" HAVE_VFORK)
check_symbol_exists(clone "sched.h" HAVE_CLONE)
check_symbol_exists(dirfd "dirent.h" HAVE_DIRFD)
check_symbol_exists(sendmmsg "sys/socket.h" HAVE_SENDMMSG)
check_symbol_exists(recvmmsg "sys/socket.h" HAVE_RECVMMSG)
check_symbol_exists(memrchr "string.h" HAVE_MEMRCHR)

check_symbol_exists(execvpe "unistd.h" HAVE_EXECVPE)
#check_symbol_exists(_Exit "stdlib.h" HAVE_EXIT)
#check_symbol_exists(exit_group "linux/unistd.h" HAVE_EXIT_GROUP)

list(APPEND CMAKE_REQUIRED_LIBRARIES "-lrt")
check_symbol_exists(aio_read64 "aio.h" HAVE_AIO_READ64)
check_symbol_exists(aio_write64 "aio.h" HAVE_AIO_WRITE64)
check_symbol_exists(aio_init "aio.h" HAVE_AIO_INIT)
check_symbol_exists(lio_listio64 "aio.h" HAVE_AIO_LIO_LISTIO64)
check_symbol_exists(aio_error64 "aio.h" HAVE_AIO_ERROR64)
check_symbol_exists(aio_return64 "aio.h" HAVE_AIO_RETURN64)
check_symbol_exists(aio_fsync64 "aio.h" HAVE_AIO_FSYNC64)
check_symbol_exists(aio_suspend64 "aio.h" HAVE_AIO_SUSPEND64)
check_symbol_exists(aio_cancel64 "aio.h" HAVE_AIO_CANCEL64)

list(APPEND CMAKE_REQUIRED_LIBRARIES "-ldl")
check_symbol_exists(dlmopen "dlfcn.h" HAVE_DLMOPEN)

check_symbol_exists(getmntent_r "mntent.h" HAVE_GETMNTENT_R)

check_symbol_exists(reallocarray "stdlib.h" HAVE_REALLOCARRAY)

check_symbol_exists(brk "unistd.h" HAVE_BRK)
check_symbol_exists(sbrk "unistd.h" HAVE_SBRK)

check_symbol_exists(sched_getcpu "sched.h" HAVE_SCHED_GETCPU)

#ToDo: right place to search for off64_t?
check_symbol_exists(off64_t "stdio.h,sys/types.h" HAVE_OFF64_T)
check_symbol_exists(O_CLOEXEC "fcntl.h" HAVE_O_CLOEXEC)
check_symbol_exists(O_DIRECTORY "fcntl.h" HAVE_O_DIRECTORY)
check_symbol_exists(O_NOFOLLOW "fcntl.h" HAVE_O_NOFOLLOW)
check_symbol_exists(O_TMPFILE "fcntl.h" HAVE_O_TMPFILE)
check_symbol_exists(O_DIRECT "fcntl.h" HAVE_O_DIRECT)
check_symbol_exists(O_NOATIME "fcntl.h" HAVE_O_NOATIME)
check_symbol_exists(O_PATH "fcntl.h" HAVE_O_PATH)
check_symbol_exists(O_LARGEFILE "fcntl.h" HAVE_O_LARGEFILE)
check_symbol_exists(RWF_APPEND "sys/uio.h" HAVE_RWF_APPEND)
check_symbol_exists(MAP_32BIT "sys/mman.h" HAVE_MAP_32BIT)
check_symbol_exists(MAP_ANONYMOUS "sys/mman.h" HAVE_MAP_ANONYMOUS)
check_symbol_exists(MAP_DENYWRITE "sys/mman.h" HAVE_MAP_DENYWRITE)
check_symbol_exists(MAP_EXECUTABLE "sys/mman.h" HAVE_MAP_EXECUTABLE)
check_symbol_exists(MAP_FILE "sys/mman.h" HAVE_MAP_FILE)
check_symbol_exists(MAP_GROWSDOWN "sys/mman.h" HAVE_MAP_GROWSDOWN)
check_symbol_exists(MAP_HUGETLB "sys/mman.h" HAVE_MAP_HUGETLB)
check_symbol_exists(MAP_HUGE_2MB "sys/mman.h" HAVE_MAP_HUGE_2MB)
check_symbol_exists(MAP_HUGE_1GB "sys/mman.h" HAVE_MAP_HUGE_1GB)
check_symbol_exists(MAP_LOCKED "sys/mman.h" HAVE_MAP_LOCKED)
check_symbol_exists(MAP_NONBLOCK "sys/mman.h" HAVE_MAP_NONBLOCK)
check_symbol_exists(MAP_NORESERVE "sys/mman.h" HAVE_MAP_NORESERVE)
check_symbol_exists(MAP_POPULATE "sys/mman.h" HAVE_MAP_POPULATE)
check_symbol_exists(MAP_STACK "sys/mman.h" HAVE_MAP_STACK)
check_symbol_exists(MAP_UNINITIALIZED "sys/mman.h" HAVE_MAP_UNINITIALIZED)
check_symbol_exists(MADV_REMOVE "sys/mman.h" HAVE_MADV_REMOVE)
check_symbol_exists(MADV_DONTFORK "sys/mman.h" HAVE_MADV_DONTFORK)
check_symbol_exists(MADV_DOFORK "sys/mman.h" HAVE_MADV_DOFORK)
check_symbol_exists(MADV_HWPOISON "sys/mman.h" HAVE_MADV_HWPOISON)
check_symbol_exists(MADV_MERGEABLE "sys/mman.h" HAVE_MADV_MERGEABLE)
check_symbol_exists(MADV_UNMERGEABLE "sys/mman.h" HAVE_MADV_UNMERGEABLE)
check_symbol_exists(MADV_SOFT_OFFLINE "sys/mman.h" HAVE_MADV_SOFT_OFFLINE)
check_symbol_exists(MADV_HUGEPAGE "sys/mman.h" HAVE_MADV_HUGEPAGE)
check_symbol_exists(MADV_NOHUGEPAGE "sys/mman.h" HAVE_MADV_NOHUGEPAGE)
check_symbol_exists(MADV_DONTDUMP "sys/mman.h" HAVE_MADV_DONTDUMP)
check_symbol_exists(MADV_DODUMP "sys/mman.h" HAVE_MADV_DODUMP)
check_symbol_exists(MADV_FREE "sys/mman.h" HAVE_MADV_FREE)
check_symbol_exists(MADV_WIPEONFORK "sys/mman.h" HAVE_MADV_WIPEONFORK)
check_symbol_exists(MADV_KEEPONFORK "sys/mman.h" HAVE_MADV_KEEPONFORK)
check_symbol_exists(SEEK_DATA "unistd.h" HAVE_SEEK_DATA)
check_symbol_exists(SEEK_HOLE "unistd.h" HAVE_SEEK_HOLE)

check_symbol_exists(HOST_NAME_MAX "limits.h" HAVE_HOST_NAME_MAX)
check_symbol_exists(_POSIX_HOST_NAME_MAX "limits.h" HAVE__POSIX_HOST_NAME_MAX)


include(${libiotrace_SOURCE_DIR}/cmake/CheckCompilerAttributes.cmake)
include(${libiotrace_SOURCE_DIR}/cmake/CheckRDTSC.cmake)
CheckCompilerAttributes()
CheckRDTSC()


# XXX ToDo: check for __uint128_t support (only on 64-bit architecture and some compiler-versions)




# -----------------------------------------------------------------------------
# CMake- / Compile options
# -----------------------------------------------------------------------------
option (LOGGING "write log (can be changed during execution with functions from libiotrace.h; is only used if ENABLE_OUTPUT is set to LOGFILE or LOGFILE_AND_INFLUXDB)" ON)
#compile using cmake -DLOGGING for write log (can be changed during execution with functions from libiotrace.h)
if(LOGGING)
    list(APPEND LIBIOTRACE_COMPILE_OPTIONS -DLOGGING)
endif()

option (SENDING "send log to influxdb (can be changed during execution with functions from libiotrace.h; is only used if ENABLE_OUTPUT is set to INFLUXDB or LOGFILE_AND_INFLUXDB)" ON)
#compile using cmake -DSENDING for sending log to influxdb (can be changed during execution with functions from libiotrace.h)
if(SENDING)
    list(APPEND LIBIOTRACE_COMPILE_OPTIONS -DSENDING)
endif()

option (STACKTRACE_PTR "write stacktrace pointer to log (can be changed during execution with functions from libiotrace.h)" OFF)
#compile using cmake -DSTACKTRACE_PTR for writing stacktrace pointer to log
if(STACKTRACE_PTR)
    list(APPEND LIBIOTRACE_COMPILE_OPTIONS -DSTACKTRACE_PTR)
endif()

option (STACKTRACE_SYMBOL "write stacktrace symbols to log (can be changed during execution with functions from libiotrace.h)" OFF)
#compile using cmake -DSTACKTRACE_SYMBOL for writing stacktrace symbols to log
if(STACKTRACE_SYMBOL)
    list(APPEND LIBIOTRACE_COMPILE_OPTIONS -DSTACKTRACE_SYMBOL)
endif()

option (LOG_WRAPPER_TIME "log start and end time of function wrappers" ON)
#compile using cmake -DLOG_WRAPPER_TIME for logging start and end time of function wrappers
if(LOG_WRAPPER_TIME)
    list(APPEND LIBIOTRACE_COMPILE_OPTIONS -DLOG_WRAPPER_TIME)
endif()

option (WITH_STD_IO "log stdin, stdout and stderr IO" ON)
#compile using cmake -DWITH_STD_IO for stdin, stdout and stderr
if(WITH_STD_IO)
    list(APPEND LIBIOTRACE_COMPILE_OPTIONS -DWITH_STD_IO)
endif()

option (WITH_POSIX_IO "build wrappers for Posix-IO" ON)
#compile using cmake -DWITH_POSIX_IO for Posix-IO
if(WITH_POSIX_IO)
    list(APPEND LIBIOTRACE_COMPILE_OPTIONS -DWITH_POSIX_IO)
    list(APPEND LIBIOTRACE_SOURCE posix_io.c)
endif()

option (WITH_POSIX_AIO "build wrappers for asynchronous Posix-IO" OFF)
#compile using cmake -DWITH_POSIX_AIO for asynchronous Posix-IO
if(WITH_POSIX_AIO)
    if(HAVE_AIO_H)
        list(APPEND LIBIOTRACE_COMPILE_OPTIONS -DWITH_POSIX_AIO)
        list(APPEND LIBIOTRACE_SOURCE posix_aio.c)
    else()
        message(SEND_ERROR "Missing aio.h for asynchronous Posix-IO.")
    endif()
endif()

option (STRACING_ENABLED "trace syscalls" OFF)     # Install required libs: `sudo apt install -y libunwind-dev libdw-dev`
#compile using cmake -DSTRACING_ENABLED for syscall tracing
if (STRACING_ENABLED)
    if(NOT WITH_POSIX_IO)
        message(FATAL_ERROR "`STRACING_ENABLED` requires `WITH_POSIX_IO`")
    endif()

    list(APPEND LIBIOTRACE_COMPILE_OPTIONS -DSTRACING_ENABLED)
    add_subdirectory(libs/rmind-ringbuf)
    add_subdirectory(stracing)
else()
    # Remove CMake options from cache
    unset(STRACING_LINUX_SRC_DIR CACHE)
    unset(STRACING_UXD_SOCKET_FILEPATH CACHE)
    unset(STRACING_UXD_REG_SOCKET_BACKLOG_SIZE CACHE)
    unset(STRACING_STRACER_LOG_OUTPUT CACHE)
    unset(STRACING_STRACER_TIMEOUT_IN_MSEC CACHE)
    unset(STRACING_FNRES_RB_SIZE CACHE)
    unset(STRACING_FNRES_SCERBMAP_SIZE CACHE)
endif()

option (WITH_MPI_IO "build wrappers for MPI-IO" OFF)
#compile using cmake -DWITH_MPI_IO for MPI-IO
if(WITH_MPI_IO)
    find_package(MPI REQUIRED)
    if(MPI_C_FOUND)
        include_directories(${MPI_C_INCLUDE_DIRS})
    else()
        message(SEND_ERROR "Missing MPI-Package.")
    endif()
    list(APPEND LIBIOTRACE_COMPILE_OPTIONS -DWITH_MPI_IO)
    list(APPEND LIBIOTRACE_SOURCE mpi_io.c)
endif()

option (WITH_DL_IO "build wrappers for dlopen() and dlmopen()" OFF)
#compile using cmake -DWITH_DL_IO for dlopen()
if(WITH_DL_IO)
    list(APPEND LIBIOTRACE_COMPILE_OPTIONS -DWITH_DL_IO)
    list(APPEND LIBIOTRACE_SOURCE dl_io.c)
endif()


option (WITH_ALLOC "build wrappers for memory allocations" OFF)
#compile using cmake -DWITH_ALLOC for memory allocations
if(WITH_ALLOC)
    list(APPEND LIBIOTRACE_COMPILE_OPTIONS -DWITH_ALLOC)
    list(APPEND LIBIOTRACE_SOURCE alloc.c)
else()
    # Remove CMake options from cache
    unset(WITH_ALLOC_CPU CACHE)
endif()

option (WITH_ALLOC_CPU "Additionaly log logical CPU on which the allocation was performed" OFF)
#compile using cmake -DWITH_ALLOC_CPU to collect usable size per memory allocation
if(WITH_ALLOC_CPU)
    list(APPEND LIBIOTRACE_COMPILE_OPTIONS -DWITH_ALLOC_CPU)
endif()

if(NOT WITH_ALLOC AND NOT WITH_DL_IO AND NOT WITH_MPI_IO AND NOT WITH_POSIX_AIO AND NOT WITH_POSIX_IO)
    message(FATAL_ERROR "at least one group of wrappers must be included")
endif()

option (FILENAME_RESOLUTION_ENABLED "create mapping b/w file handles and filenames during runtime" OFF)
# Compile using cmake -DFILENAME_RESOLUTION_ENABLED for creating mappings b/w file handles and filenames during runtime
if (FILENAME_RESOLUTION_ENABLED OR STRACING_ENABLED)
    add_subdirectory(libs/atomic_hash)
else()
    # Remove atomic_hash's CMake options from cache
    unset(HASH_FUNCTION CACHE)
    unset(HASH_FUNCTION_VALUES CACHE)
endif()
if (FILENAME_RESOLUTION_ENABLED)
    list(APPEND LIBIOTRACE_COMPILE_OPTIONS -DFILENAME_RESOLUTION_ENABLED)
    list(APPEND LIBIOTRACE_SOURCE
            fnres/fnres.c
            fnres/fnmap/fnmap_atomic_hash.c)

    list(APPEND LIBIOTRACE_SHARED_LINK_OPTIONS atomichash-object)
    list(APPEND LIBIOTRACE_STATIC_LINK_OPTIONS atomichash-object)
endif()


option (ALL_WRAPPERS_ACTIVE "Activate or deactivate all wrappers" ON)
if(ALL_WRAPPERS_ACTIVE)
    list(APPEND LIBIOTRACE_COMPILE_OPTIONS -DALL_WRAPPERS_ACTIVE=${ALL_WRAPPERS_ACTIVE})
endif()

option (REALTIME "Use the real system time (is affected by discontinuous jumps in the system time (e.g., if the system administrator manually changes the clock), and by the incremental adjustments performed by adjtime and NTP)" ON)
if(REALTIME)
    list(APPEND LIBIOTRACE_COMPILE_OPTIONS -DREALTIME)
endif()

option (ENABLE_REMOTE_CONTROL "Remotely control wrappers (toggle on/off) via REST-API" ON)
if(ENABLE_REMOTE_CONTROL)
    list(APPEND LIBIOTRACE_COMPILE_OPTIONS -DENABLE_REMOTE_CONTROL)
endif()

option (ENABLE_FILESYSTEM_METADATA "writes mount points to LOGFILE and/or to INFLUXDB if ENABLE_OUTPUT is set to LOGFILE, INFLUXDB or LOGFILE_AND_INFLUXDB" ON)
if(ENABLE_FILESYSTEM_METADATA)
    list(APPEND LIBIOTRACE_COMPILE_OPTIONS -DENABLE_FILESYSTEM_METADATA)
endif()

option (ENABLE_POWER_MEASUREMENT_POWERCAP "build with Powercap for Powermeasurment" OFF)
if(ENABLE_POWER_MEASUREMENT_POWERCAP)
    list(APPEND LIBIOTRACE_COMPILE_OPTIONS -DENABLE_POWER_MEASUREMENT_POWERCAP)
endif()

option (ENABLE_POWER_MEASUREMENT_RAPL "build with rapl for Powermeasurment" OFF)
if(ENABLE_POWER_MEASUREMENT_RAPL)
    list(APPEND LIBIOTRACE_COMPILE_OPTIONS -DENABLE_POWER_MEASUREMENT_RAPL)
endif()

set(BUFFER_SIZE "1048576" CACHE STRING "buffer size per process")
list(APPEND LIBIOTRACE_COMPILE_OPTIONS -DBUFFER_SIZE=${BUFFER_SIZE})

set(MAX_FUNCTION_NAME "40" CACHE STRING "max length of function names")
list(APPEND LIBIOTRACE_COMPILE_OPTIONS -DMAX_FUNCTION_NAME=${MAX_FUNCTION_NAME})

set(MAX_ERROR_TEXT "1024" CACHE STRING "max length of error text for errno number")
list(APPEND LIBIOTRACE_COMPILE_OPTIONS -DMAX_ERROR_TEXT=${MAX_ERROR_TEXT})

set(MAX_MMSG_MESSAGES "20" CACHE STRING "max number of messages send/received over a socket via single function call")
list(APPEND LIBIOTRACE_COMPILE_OPTIONS -DMAX_MMSG_MESSAGES=${MAX_MMSG_MESSAGES})

set(MAX_MSG_FILE_DESCRIPTORS "200" CACHE STRING "max number of file descriptors send/received over a socket via single message")
list(APPEND LIBIOTRACE_COMPILE_OPTIONS -DMAX_MSG_FILE_DESCRIPTORS=${MAX_MSG_FILE_DESCRIPTORS})

set(MAX_EXEC_ARRAY_LENGTH "1000" CACHE STRING "max number of environment variables processed in wrapper for exec functions")
list(APPEND LIBIOTRACE_COMPILE_OPTIONS -DMAX_EXEC_ARRAY_LENGTH=${MAX_EXEC_ARRAY_LENGTH})

set(MAX_INFLUX_TOKEN "200" CACHE STRING "max length of influxdb token")
list(APPEND LIBIOTRACE_COMPILE_OPTIONS -DMAX_INFLUX_TOKEN=${MAX_INFLUX_TOKEN})

set(MAX_STACKTRACE_DEPTH "50" CACHE STRING "max number of function call entries in a single stack trace")
list(APPEND LIBIOTRACE_COMPILE_OPTIONS -DMAX_STACKTRACE_DEPTH=${MAX_STACKTRACE_DEPTH})

set(MAX_STACKTRACE_ENTRY_LENGTH "200" CACHE STRING "max length of single entry in stack trace")
list(APPEND LIBIOTRACE_COMPILE_OPTIONS -DMAX_STACKTRACE_ENTRY_LENGTH=${MAX_STACKTRACE_ENTRY_LENGTH})

set(MAX_MPI_FILE_HINTS "30" CACHE STRING "max number of MPI_Info file hints")
list(APPEND LIBIOTRACE_COMPILE_OPTIONS -DMAX_MPI_FILE_HINTS=${MAX_MPI_FILE_HINTS})

set(MAX_MPI_FILE_HINT_LENGTH "30" CACHE STRING "max length per MPI_Info file hint (key + value)")
list(APPEND LIBIOTRACE_COMPILE_OPTIONS -DMAX_MPI_FILE_HINT_LENGTH=${MAX_MPI_FILE_HINT_LENGTH})

set(MAX_MPI_IMESSAGES "1000" CACHE STRING "max number of immediate messages that could be processed in MPI_Waitall and MPI_Testall")
list(APPEND LIBIOTRACE_COMPILE_OPTIONS -DMAX_MPI_IMESSAGES=${MAX_MPI_IMESSAGES})

set(PORT_RANGE_MIN "50000" CACHE STRING "begin of port range to control live tracing")
list(APPEND LIBIOTRACE_COMPILE_OPTIONS -DPORT_RANGE_MIN=${PORT_RANGE_MIN})

set(PORT_RANGE_MAX "60000" CACHE STRING "end of port range to control live tracing")
list(APPEND LIBIOTRACE_COMPILE_OPTIONS -DPORT_RANGE_MAX=${PORT_RANGE_MAX})

set(SELECT_TIMEOUT_SECONDS "1" CACHE STRING "interval in which new threads are added to the communication_thread (reads responses from influxdb)")
list(APPEND LIBIOTRACE_COMPILE_OPTIONS -DSELECT_TIMEOUT_SECONDS=${SELECT_TIMEOUT_SECONDS})

set(STACKTRACE_DEPTH "0" CACHE STRING "stacktrace depth for logging (can be changed during execution with functions from libiotrace.h); value must be lower or equal than MAX_STACKTRACE_DEPTH")
set(STACKTRACE_DEPTH_VALUES "0;1;2;3;4;5;6;7;8;9;10;11;12;13;14;15;16;17;18;19;20;21;22;23;24;25;26;27;28;29;30;31;32")
set_property(CACHE STACKTRACE_DEPTH PROPERTY STRINGS ${STACKTRACE_DEPTH_VALUES})
list(APPEND LIBIOTRACE_COMPILE_OPTIONS -DSTACKTRACE_DEPTH=${STACKTRACE_DEPTH})

set(ENABLE_OUTPUT "LOGFILE_AND_INFLUXDB" CACHE STRING "include functions to write to one or multiple targets")
set(ENABLE_OUTPUT_VALUES "LOGFILE_AND_INFLUXDB;LOGFILE;INFLUXDB")
set_property(CACHE ENABLE_OUTPUT PROPERTY STRINGS ${ENABLE_OUTPUT_VALUES})
list(APPEND LIBIOTRACE_COMPILE_OPTIONS -DENABLE_OUTPUT=${ENABLE_OUTPUT})


# Let the user chose the gettime functionality
# XXX Instead of hard-coding these settings, they should be chosen dependent on availability...
set_property(SOURCE common/gettime.c CACHE LIBIOTRACE_GETTIME PROPERTY STRINGS "LIBIOTRACE_GETTIME" "WANT_GETTIMEOFDAY;WANT_GETTIME;WANT_RDTSC")
set(LIBIOTRACE_GETTIME "NOTHING" CACHE STRING "Build with the timing option")
set(WANT_GETTIMEOFDAY 1 CACHE BOOL "If gettimeofday is selected")
set(WANT_GETTIME 1 CACHE BOOL "If gettimeof is selected")
set(WANT_RDTSC 1 CACHE BOOL "If rdtsc is selected")



# -----------------------------------------------------------------------------
# CMake targets
# -----------------------------------------------------------------------------
set(LIB_BASENAME "iotrace")

add_library(iotrace-shared SHARED ${LIBIOTRACE_SOURCE})
target_compile_options(iotrace-shared PRIVATE ${LIBIOTRACE_COMPILE_OPTIONS})
target_link_libraries(iotrace-shared PRIVATE ${LIBIOTRACE_SHARED_LINK_OPTIONS})
set_target_properties(iotrace-shared PROPERTIES OUTPUT_NAME ${LIB_BASENAME})

add_library(iotrace-static STATIC ${LIBIOTRACE_SOURCE})
target_compile_options(iotrace-static PRIVATE ${LIBIOTRACE_COMPILE_OPTIONS})
target_link_libraries(iotrace-static PRIVATE ${LIBIOTRACE_STATIC_LINK_OPTIONS})  # TODO: DO WE ALSO NEED target_link_libraries FOR STATIC (i.e., is this line required) ??
set_target_properties(iotrace-static PROPERTIES OUTPUT_NAME ${LIB_BASENAME})

add_library(iotrace-object OBJECT ${LIBIOTRACE_SOURCE})
target_compile_options(iotrace-object PRIVATE ${LIBIOTRACE_COMPILE_OPTIONS} -DIO_LIB_STATIC)

add_library(iotrace_control SHARED libiotrace.c)


#install(TARGETS iotrace-static
#        ARCHIVE DESTINATION lib
#)
