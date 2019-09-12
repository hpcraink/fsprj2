enable_testing()

add_subdirectory(internal)

# ToDo: generate file with set LINK_OPTIONS_STATIC and include it here

if(WITH_POSIX_IO)
    list(APPEND LINK_OPTIONS_STATIC
        "-Wl,-wrap=open"
        "-Wl,-wrap=open64"
        "-Wl,-wrap=openat"
        "-Wl,-wrap=creat"
        "-Wl,-wrap=creat64"
        "-Wl,-wrap=close"
        "-Wl,-wrap=read"
        "-Wl,-wrap=pread"
        "-Wl,-wrap=pread64"
        "-Wl,-wrap=write"
        "-Wl,-wrap=pwrite"
        "-Wl,-wrap=pwrite64"
        "-Wl,-wrap=lseek"
        "-Wl,-wrap=lseek64"
        "-Wl,-wrap=readv"
        "-Wl,-wrap=writev"
        "-Wl,-wrap=preadv"
        "-Wl,-wrap=preadv64"
        "-Wl,-wrap=pwritev"
        "-Wl,-wrap=pwritev64"
        "-Wl,-wrap=preadv2"
        "-Wl,-wrap=preadv64v2"
        "-Wl,-wrap=pwritev2"
        "-Wl,-wrap=pwritev64v2"
        "-Wl,-wrap=copy_file_range"
        "-Wl,-wrap=mmap"
        "-Wl,-wrap=mmap64"
        "-Wl,-wrap=munmap"
        "-Wl,-wrap=msync"
        "-Wl,-wrap=mremap"
        "-Wl,-wrap=madvise"
        "-Wl,-wrap=posix_madvise"
        "-Wl,-wrap=select"
        "-Wl,-wrap=sync"
        "-Wl,-wrap=syncfs"
        "-Wl,-wrap=fsync"
        "-Wl,-wrap=fdatasync"
        "-Wl,-wrap=dup"
        "-Wl,-wrap=dup2"
        "-Wl,-wrap=dup3"
        "-Wl,-wrap=fopen"
        "-Wl,-wrap=fopen64"
        "-Wl,-wrap=freopen"
        "-Wl,-wrap=freopen64"
        "-Wl,-wrap=fdopen"
        "-Wl,-wrap=fclose"
        "-Wl,-wrap=fcloseall"
        "-Wl,-wrap=flockfile"
        "-Wl,-wrap=ftrylockfile"
        "-Wl,-wrap=funlockfile"
        "-Wl,-wrap=fwide"
        "-Wl,-wrap=fputc"
        "-Wl,-wrap=fputwc"
        "-Wl,-wrap=fputc_unlocked"
        "-Wl,-wrap=fputwc_unlocked"
        "-Wl,-wrap=putc"
        "-Wl,-wrap=_IO_putc"
        "-Wl,-wrap=putwc"
        "-Wl,-wrap=putc_unlocked"
        "-Wl,-wrap=putwc_unlocked"
        "-Wl,-wrap=fputs"
        "-Wl,-wrap=fputws"
        "-Wl,-wrap=fputs_unlocked"
        "-Wl,-wrap=fputws_unlocked"
        "-Wl,-wrap=putw"
        "-Wl,-wrap=fgetc"
        "-Wl,-wrap=fgetwc"
        "-Wl,-wrap=fgetc_unlocked"
        "-Wl,-wrap=fgetwc_unlocked"
        "-Wl,-wrap=getc"
        "-Wl,-wrap=_IO_getc"
        "-Wl,-wrap=getwc"
        "-Wl,-wrap=getc_unlocked"
        "-Wl,-wrap=getwc_unlocked"
        "-Wl,-wrap=getw"
        "-Wl,-wrap=getline"
        "-Wl,-wrap=getdelim"
        "-Wl,-wrap=fgets"
        "-Wl,-wrap=fgetws"
        "-Wl,-wrap=fgets_unlocked"
        "-Wl,-wrap=fgetws_unlocked"
        "-Wl,-wrap=ungetc"
        "-Wl,-wrap=ungetwc"
        "-Wl,-wrap=fread"
        "-Wl,-wrap=fread_unlocked"
        "-Wl,-wrap=fwrite"
        "-Wl,-wrap=fwrite_unlocked"
        "-Wl,-wrap=fprintf"
        "-Wl,-wrap=fwprintf"
        "-Wl,-wrap=vfprintf"
        "-Wl,-wrap=vfwprintf"
        "-Wl,-wrap=fscanf"
        "-Wl,-wrap=fwscanf"
        "-Wl,-wrap=vfscanf"
        "-Wl,-wrap=vfwscanf"
        "-Wl,-wrap=feof"
        "-Wl,-wrap=feof_unlocked"
        "-Wl,-wrap=ferror"
        "-Wl,-wrap=ferror_unlocked"
        "-Wl,-wrap=clearerr"
        "-Wl,-wrap=clearerr_unlocked"
        "-Wl,-wrap=ftell"
        "-Wl,-wrap=ftello"
        "-Wl,-wrap=ftello64"
        "-Wl,-wrap=fseek"
        "-Wl,-wrap=fseeko"
        "-Wl,-wrap=fseeko64"
        "-Wl,-wrap=rewind"
        "-Wl,-wrap=fgetpos"
        "-Wl,-wrap=fgetpos64"
        "-Wl,-wrap=fsetpos"
        "-Wl,-wrap=fsetpos64"
        "-Wl,-wrap=fflush"
        "-Wl,-wrap=fflush_unlocked"
        "-Wl,-wrap=setvbuf"
        "-Wl,-wrap=setbuf"
        "-Wl,-wrap=setbuffer"
        "-Wl,-wrap=setlinebuf"
        "-Wl,-wrap=fileno"
        "-Wl,-wrap=__freadable"
        "-Wl,-wrap=__fwritable"
        "-Wl,-wrap=__freading"
        "-Wl,-wrap=__fwriting"
        "-Wl,-wrap=__fsetlocking"
        "-Wl,-wrap=_flushlbf"
        "-Wl,-wrap=__fpurge"
        "-Wl,-wrap=__flbf"
        "-Wl,-wrap=__fbufsize"
        "-Wl,-wrap=__fpending"
    )
endif()

if(WITH_POSIX_AIO)
    list(APPEND LINK_OPTIONS "-lrt")
    list(APPEND LINK_OPTIONS_STATIC
        "-Wl,-wrap=aio_read"
        "-Wl,-wrap=aio_read64"
        "-Wl,-wrap=aio_write"
        "-Wl,-wrap=aio_write64"
        "-Wl,-wrap=lio_listio"
        "-Wl,-wrap=lio_listio64"
        "-Wl,-wrap=aio_error"
        "-Wl,-wrap=aio_error64"
        "-Wl,-wrap=aio_return"
        "-Wl,-wrap=aio_return64"
        "-Wl,-wrap=aio_fsync"
        "-Wl,-wrap=aio_fsync64"
        "-Wl,-wrap=aio_suspend"
        "-Wl,-wrap=aio_suspend64"
        "-Wl,-wrap=aio_cancel"
        "-Wl,-wrap=aio_cancel64"
        "-Wl,-wrap=aio_init"
    )
endif()

if(WITH_MPI_IO)
    list(APPEND LINK_OPTIONS MPI::MPI_C)
    list(APPEND LINK_OPTIONS_STATIC
        "-Wl,-wrap=MPI_File_open"
    )
endif()

if(WITH_DL_IO)
    list(APPEND LINK_OPTIONS "-ldl")
    list(APPEND LINK_OPTIONS_STATIC
        "-Wl,-wrap=dlopen"
        "-Wl,-wrap=dlmopen"
    )
endif()

list(APPEND LINK_OPTIONS
    "-lpthread"
)

##########################################################################################################

if(WITH_POSIX_IO)
    # test with: LD_PRELOAD=../src/libiotrace_shared.so ./read_byte
    add_executable(read_byte_shared read_byte.c)
    target_link_libraries(read_byte_shared ${LINK_OPTIONS})
    #add_test ("Read" read_byte)

    add_library(read_byte_object OBJECT read_byte.c)

    # test with: ./read_byte_static
    add_executable(read_byte_static
        $<TARGET_OBJECTS:read_byte_object>
        $<TARGET_OBJECTS:iotrace_object>
    )
    target_link_libraries(read_byte_static ${LINK_OPTIONS_STATIC})
    target_link_libraries(read_byte_static ${LINK_OPTIONS})
endif()

##########################################################################################################

if(WITH_POSIX_IO)
    # test with: LD_PRELOAD=../src/libiotrace_shared.so ./read_stream
    add_executable(read_stream_shared read_stream.c)
    target_compile_options(read_stream_shared PRIVATE -D_GNU_SOURCE)
    target_link_libraries(read_stream_shared ${LINK_OPTIONS})

    add_library(read_stream_object OBJECT read_stream.c)
    target_compile_options(read_stream_object PRIVATE -D_GNU_SOURCE)

    # test with: ./read_stream_static
    add_executable(read_stream_static
        $<TARGET_OBJECTS:read_stream_object>
        $<TARGET_OBJECTS:iotrace_object>
    )
    target_link_libraries(read_stream_static ${LINK_OPTIONS_STATIC})
    target_link_libraries(read_stream_static ${LINK_OPTIONS})
endif()

##########################################################################################################

if(WITH_POSIX_IO)
    # test with: LD_PRELOAD=../src/libiotrace_shared.so ./read_byte_and_stream_shared
    add_executable(read_byte_and_stream_shared read_byte_and_stream.c)
    target_link_libraries(read_byte_and_stream_shared ${LINK_OPTIONS})

    add_library(read_byte_and_stream_object OBJECT read_byte_and_stream.c)

    # test with: ./read_byte_and_stream_static
    add_executable(read_byte_and_stream_static
        $<TARGET_OBJECTS:read_byte_and_stream_object>
        $<TARGET_OBJECTS:iotrace_object>
    )
    target_link_libraries(read_byte_and_stream_static ${LINK_OPTIONS_STATIC})
    target_link_libraries(read_byte_and_stream_static ${LINK_OPTIONS})
endif()

##########################################################################################################

if(WITH_POSIX_IO)
    # test with: LD_PRELOAD=../src/libiotrace_shared.so ./write_stream_shared
    add_executable(write_stream_shared write_stream.c)
    target_link_libraries(write_stream_shared ${LINK_OPTIONS})

    add_library(write_stream_object OBJECT write_stream.c)

    # test with: ./write_stream_static
    add_executable(write_stream_static
        $<TARGET_OBJECTS:write_stream_object>
        $<TARGET_OBJECTS:iotrace_object>
    )
    target_link_libraries(write_stream_static ${LINK_OPTIONS_STATIC})
    target_link_libraries(write_stream_static ${LINK_OPTIONS})
endif()

##########################################################################################################

if(WITH_POSIX_IO)
    #test with: LD_PRELOAD=../src/libiotrace_shared.so OMP_NUM_THREADS=2 ./read_stream_openmp_shared
    add_executable(read_stream_openmp_shared read_stream_openmp.c)

    find_package(OpenMP)
    if(OpenMP_C_FOUND)
        target_link_libraries(read_stream_openmp_shared PUBLIC OpenMP::OpenMP_C)
    endif()
endif()

##########################################################################################################

if(WITH_POSIX_AIO)
    # test with: LD_PRELOAD=../src/libiotrace_shared.so ./read_async_byte
    add_executable(read_async_byte_shared read_async_byte.c)
    target_link_libraries(read_async_byte_shared ${LINK_OPTIONS})

    add_library(read_async_byte_object OBJECT read_async_byte.c)

    # test with: ./read_async_byte_static
    add_executable(read_async_byte_static
        $<TARGET_OBJECTS:read_async_byte_object>
        $<TARGET_OBJECTS:iotrace_object>
    )
    target_link_libraries(read_async_byte_static ${LINK_OPTIONS_STATIC})
    target_link_libraries(read_async_byte_static ${LINK_OPTIONS})
endif()

##########################################################################################################

if(WITH_MPI_IO)
    # test with: mpirun -np 4 -x LD_PRELOAD=../src/libiotrace_shared.so ./read_MPI
    add_executable(read_MPI read_MPI.c)
    target_link_libraries(read_MPI PRIVATE ${LINK_OPTIONS})
endif()

##########################################################################################################