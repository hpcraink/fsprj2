include(CheckSymbolExists)

check_include_files("stdbool.h" HAVE_STDBOOL_H)
check_include_files("stdlib.h" HAVE_STDLIB_H)
check_include_files("stdint.h" HAVE_STDINT_H)
check_include_files("sys/fcntl.h" HAVE_SYS_FCNTL_H)
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
# we need to turn on Optimization to make those visible (plus _FORTIFY_SOURCE itselve)
list(APPEND CMAKE_REQUIRED_FLAGS       -O2)
list(APPEND CMAKE_REQUIRED_DEFINITIONS -D_GNU_SOURCE -D__USE_LARGEFILE64 -D_FORTIFY_SOURCE=2)

check_symbol_exists(__open "fcntl.h" HAVE___OPEN)              # $$$$$$$$ TODO: DOESN'T WORK SOMEHOW $$$$$$$$
check_symbol_exists(__open64 "fcntl.h" HAVE___OPEN64)          # $$$$$$$$ TODO: DOESN'T WORK SOMEHOW $$$$$$$$
check_symbol_exists(__open_2 "fcntl.h" HAVE___OPEN_2)          # $$$$$$$$ TODO: DOESN'T WORK SOMEHOW $$$$$$$$
check_symbol_exists(__open64_2 "fcntl.h" HAVE___OPEN64_2)      # $$$$$$$$ TODO: DOESN'T WORK SOMEHOW $$$$$$$$
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

# Please keep in alphabetical order
set(SOURCE
    event.c
    init.c
    llhttp/api.c
    llhttp/http.c
    llhttp/llhttp.c
    os.c
    utils.c
)

#compile using cmake -DWITH_POSIX_IO for Posix-IO
if(WITH_POSIX_IO)
    list(APPEND SOURCE posix_io.c)
endif()

#compile using cmake -DWITH_POSIX_AIO for asynchronous Posix-IO
if(WITH_POSIX_AIO)
    if(HAVE_AIO_H)
        list(APPEND SOURCE posix_aio.c)
    else()
        message(SEND_ERROR "Missing aio.h for asynchronous Posix-IO.")
    endif()
endif()

#compile using cmake -DWITH_MPI_IO for MPI-IO
if(WITH_MPI_IO)
    list(APPEND SOURCE mpi_io.c)
endif()

#compile using cmake -DWITH_DL_IO for dlopen()
if(WITH_DL_IO)
    list(APPEND SOURCE dl_io.c)
endif()

#compile using cmake -DWITH_ALLOC for memory allocations
if(WITH_ALLOC)
    list(APPEND SOURCE alloc.c)
endif()

if(WITH_FILENAME_RESOLUTION)
    list(APPEND SOURCE
            fnres/fctevent.c
            fnres/internal/map/fnmap.c
            fnres/internal/map/impl/hash_functions/hash_city.c
            fnres/internal/map/impl/atomic_hash.c)
endif()

add_library(iotrace_shared SHARED ${SOURCE})

add_library(iotrace_static STATIC ${SOURCE})

add_library(iotrace_object OBJECT ${SOURCE})

add_library(iotrace_control SHARED libiotrace.c)


# ${CMAKE_DL_LIBS} needed for dlsym-function
# "-lpthread" needed for pthread_atfork()
if(WITH_FILENAME_RESOLUTION)
    set(CMAKE_C_FLAGS "-march=native -msse4.2 -D_GNU_SOURCE")      # sse4.2 is required for 'hash_city.c'

    target_link_libraries(iotrace_shared PRIVATE ${CMAKE_DL_LIBS} m pthread)
    target_link_libraries(iotrace_static PRIVATE ${CMAKE_DL_LIBS} m pthread)        # $$$ TODO: DO WE ALSO NEED target_link_libraries FOR STATIC (i.e., is this line required) ?? $$$
else()
    target_link_libraries(iotrace_shared PRIVATE ${CMAKE_DL_LIBS} "-lpthread")
endif()

target_compile_options(iotrace_shared PRIVATE ${OPTIONS})
target_compile_options(iotrace_static PRIVATE ${OPTIONS})
target_compile_options(iotrace_object PRIVATE ${OPTIONS})

# object for static linking needs
target_compile_options(iotrace_object PRIVATE -DIO_LIB_STATIC)

#install(TARGETS iotrace_static
#        ARCHIVE DESTINATION lib
#)
