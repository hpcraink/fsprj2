
# -----------------------------------------------------------------------------
# CMake- / Compile options
# -----------------------------------------------------------------------------
# ensure debug info and assert functions are build during compile of tests
set(CMAKE_BUILD_TYPE Debug)

# XXX For production version, the default should be OFF
# And a CI/CD server should compile using cmake -DWITH_UNIT_TESTS
option (WITH_UNIT_TESTS "enable building unit tests" ON)
if (WITH_UNIT_TESTS)
    # Solution from https://www.scivision.dev/cmake-git-submodule/
    #
    # If this fails, start anew by adding the cunit git as submodule
    # git rm --cached test/ext/cunit
    # rm -rf test/ext
    # git submodule add https://gitlab.com/cunity/cunit.git test/ext/cunit

    # A better way than just plain COMPILING our own version of CUnit
    # might be to find_package(CUnit REQUIRED) and check for pre-installed version.
    include(${libiotrace_SOURCE_DIR}/cmake/AddGitSubModule.cmake)

    add_git_submodule(test/ext/cunit)
    add_subdirectory(ext)
    set(CMAKE_PREFIX_PATH "${PROJECT_SOURCE_DIR}/test/ext/cunit/CUnit/;${CMAKE_PREFIX_PATH}")

    enable_testing()


    # add_subdirectory(internal)            # !! WARNING: Breaks arm64 build !!


    include(CheckIncludeFiles)

    check_include_files("linux/kcmp.h" HAVE_LINUX_KCMP_H)

    # ToDo: generate file with set TESTS_LINK_OPTIONS_STATIC and include it here


    list(APPEND TESTS_LINK_OPTIONS_STATIC
            "-Wl,-wrap=execve"
            "-Wl,-wrap=execv"
            "-Wl,-wrap=execl"
            "-Wl,-wrap=execvp"
            "-Wl,-wrap=execlp"
            "-Wl,-wrap=execvpe"
            "-Wl,-wrap=execle"
            "-Wl,-wrap=_exit"
            "-Wl,-wrap=_Exit"
            "-Wl,-wrap=exit_group"
            "-Wl,-wrap=pthread_create"
    )

    if(WITH_POSIX_IO)
        list(APPEND TESTS_LINK_OPTIONS_STATIC
            "-Wl,-wrap=open"
            "-Wl,-wrap=__open"
            "-Wl,-wrap=__open64"
            "-Wl,-wrap=__open_2"
            "-Wl,-wrap=__open64_2"
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
            "-Wl,-wrap=fcntl"
            "-Wl,-wrap=socket"
            "-Wl,-wrap=accept"
            "-Wl,-wrap=accept4"
            "-Wl,-wrap=socketpair"
            "-Wl,-wrap=connect"
            "-Wl,-wrap=bind"
            "-Wl,-wrap=pipe"
            "-Wl,-wrap=pipe2"
            "-Wl,-wrap=memfd_create"
            "-Wl,-wrap=epoll_create"
            "-Wl,-wrap=epoll_create1"
            "-Wl,-wrap=mkstemp"
            "-Wl,-wrap=mkostemp"
            "-Wl,-wrap=mkstemps"
            "-Wl,-wrap=mkostemps"
            "-Wl,-wrap=eventfd"
            "-Wl,-wrap=inotify_init"
            "-Wl,-wrap=inotify_init1"
            "-Wl,-wrap=readdir"
            "-Wl,-wrap=dirfd"
            "-Wl,-wrap=sendmsg"
            "-Wl,-wrap=recvmsg"
            "-Wl,-wrap=sendmmsg"
            "-Wl,-wrap=recvmmsg"
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
            "-Wl,-wrap=tmpfile"
            "-Wl,-wrap=tmpfile64"
            "-Wl,-wrap=popen"
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
            "-Wl,-wrap=fork"
            "-Wl,-wrap=vfork"
            "-Wl,-wrap=clone"
        )
        if(HAVE___XSTAT_IN_LIBC)
            list(APPEND TESTS_LINK_OPTIONS_STATIC
                "-Wl,-wrap=__xstat"
            )
        endif()
        if(HAVE___FXSTAT_IN_LIBC)
            list(APPEND TESTS_LINK_OPTIONS_STATIC
                "-Wl,-wrap=__fxstat"
            )
        endif()
        if(HAVE___LXSTAT_IN_LIBC)
            list(APPEND TESTS_LINK_OPTIONS_STATIC
                "-Wl,-wrap=__lxstat"
            )
        endif()
        if(HAVE___FXSTATAT_IN_LIBC)
            list(APPEND TESTS_LINK_OPTIONS_STATIC
                "-Wl,-wrap=__fxstatat"
            )
        endif()
        if(HAVE_STAT_IN_LIBC)
            list(APPEND TESTS_LINK_OPTIONS_STATIC
                "-Wl,-wrap=stat"
            )
        endif()
        if(HAVE_FSTAT_IN_LIBC)
            list(APPEND TESTS_LINK_OPTIONS_STATIC
                "-Wl,-wrap=fstat"
            )
        endif()
        if(HAVE_LSTAT_IN_LIBC)
            list(APPEND TESTS_LINK_OPTIONS_STATIC
                "-Wl,-wrap=lstat"
            )
        endif()
        if(HAVE_FSTATAT_IN_LIBC)
            list(APPEND TESTS_LINK_OPTIONS_STATIC
                "-Wl,-wrap=fstatat"
            )
        endif()
    endif()

    if(WITH_POSIX_AIO)
        list(APPEND TESTS_LINK_OPTIONS "-lrt")
        list(APPEND TESTS_LINK_OPTIONS_STATIC
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
            "-Wl,-wrap=shm_open"
        )
    endif()

    if(WITH_MPI_IO)
        find_package(MPI REQUIRED)
        if(MPI_C_FOUND)
            include_directories(${MPI_C_INCLUDE_DIRS})
        else()
            message(SEND_ERROR "Missing MPI-Package.")
        endif()

        list(APPEND TESTS_LINK_OPTIONS MPI::MPI_C)
        list(APPEND TESTS_LINK_OPTIONS_STATIC
            "-Wl,-wrap=MPI_File_open"
        )
    endif()

    if(WITH_DL_IO)
        list(APPEND TESTS_LINK_OPTIONS "-ldl")
        list(APPEND TESTS_LINK_OPTIONS_STATIC
            "-Wl,-wrap=dlopen"
            "-Wl,-wrap=dlmopen"
        )
    endif()

    if(WITH_ALLOC)
        list(APPEND TESTS_LINK_OPTIONS_STATIC
            "-Wl,-wrap=malloc"
            "-Wl,-wrap=free"
            "-Wl,-wrap=calloc"
            "-Wl,-wrap=realloc"
            "-Wl,-wrap=reallocarray"
            "-Wl,-wrap=posix_memalign"
            "-Wl,-wrap=brk"
            "-Wl,-wrap=sbrk"
        )
    endif()

    list(APPEND TESTS_LINK_OPTIONS
        pthread
    )

    if (FILENAME_RESOLUTION_ENABLED)
        list(APPEND TESTS_LINK_OPTIONS "atomichash-object")
    endif()

# -----------------------------------------------------------------------------
# CMake targets
# -----------------------------------------------------------------------------
    if (STRACING_ENABLED)
        list(APPEND TESTS_LINK_OPTIONS "libringbuf_object")

    # - Programs for testing tracing  -
        # test with: IOTRACE_LOG_NAME=stracing_trace_pthread_fork_shared_test1 LD_PRELOAD=../src/libiotrace.so ./stracing_trace_pthread_fork_shared [--loop] [--fork | --pthread]
        add_executable(stracing_trace_pthread_fork_shared stracing/trace_pthread_fork.c)
        target_link_libraries(stracing_trace_pthread_fork_shared pthread)


        # test with: IOTRACE_LOG_NAME=stracing_trace_manual_syscall_shared_test1 LD_PRELOAD=../src/libiotrace.so ./stracing_trace_manual_syscall_shared [--use-glibc-wrapper] [--perform-open-in-pthread]
        add_executable(stracing_trace_manual_syscall_shared stracing/trace_manual_syscall.c)
        target_compile_options(stracing_trace_manual_syscall_shared PRIVATE -D_GNU_SOURCE)
        target_link_libraries(stracing_trace_manual_syscall_shared pthread)

        add_library(stracing_trace_manual_syscall_object OBJECT stracing/trace_manual_syscall.c)
        target_link_libraries(stracing_trace_manual_syscall_object pthread)

        # test with: IOTRACE_LOG_NAME=stracing_trace_manual_syscall_static_test1 ./stracing_trace_manual_syscall_static [--use-glibc-wrapper]
        add_executable(stracing_trace_manual_syscall_static
            $<TARGET_OBJECTS:stracing_trace_manual_syscall_object>
            $<TARGET_OBJECTS:iotrace-object>
        )
        target_link_libraries(stracing_trace_manual_syscall_static ${TESTS_LINK_OPTIONS_STATIC})
        target_link_libraries(stracing_trace_manual_syscall_static ${TESTS_LINK_OPTIONS} pthread)
    endif()


    ##########################################################################################################

    if(WITH_POSIX_IO)
        # test with: IOTRACE_LOG_NAME=read_byte_shared_test1 LD_PRELOAD=../src/libiotrace.so ./read_byte_shared
        add_executable(read_byte_shared read_byte.c)
        target_link_libraries(read_byte_shared ${TESTS_LINK_OPTIONS})
        #add_test ("Read" read_byte)

        add_library(read_byte_object OBJECT read_byte.c)

        # test with: IOTRACE_LOG_NAME=read_byte_static_test1 ./read_byte_static
        add_executable(read_byte_static
            $<TARGET_OBJECTS:read_byte_object>
            $<TARGET_OBJECTS:iotrace-object>
        )
        target_link_libraries(read_byte_static ${TESTS_LINK_OPTIONS_STATIC})
        target_link_libraries(read_byte_static ${TESTS_LINK_OPTIONS})

    endif()
    
    ##########################################################################################################

    if(WITH_POSIX_IO)
        # test with: IOTRACE_LOG_NAME=stat_shared_test1 LD_PRELOAD=../src/libiotrace.so ./stat_shared
        add_executable(stat_shared stat.c)
        target_compile_options(stat_shared PRIVATE -D_GNU_SOURCE)
        target_link_libraries(stat_shared ${TESTS_LINK_OPTIONS})

        add_library(stat_object OBJECT stat.c)
        target_compile_options(stat_object PRIVATE -D_GNU_SOURCE)

        # test with: IOTRACE_LOG_NAME=stat_static_test1 ./stat_static
        add_executable(stat_static
            $<TARGET_OBJECTS:stat_object>
            $<TARGET_OBJECTS:iotrace-object>
        )
        target_link_libraries(stat_static ${TESTS_LINK_OPTIONS_STATIC})
        target_link_libraries(stat_static ${TESTS_LINK_OPTIONS})

    endif()

    ##########################################################################################################

    if(WITH_POSIX_IO)
        # test with: IOTRACE_LOG_NAME=read_stream_shared_test1 LD_PRELOAD=../src/libiotrace.so ./read_stream_shared
        add_executable(read_stream_shared read_stream.c)
        target_compile_options(read_stream_shared PRIVATE -D_GNU_SOURCE)
        target_link_libraries(read_stream_shared ${TESTS_LINK_OPTIONS})

        add_library(read_stream_object OBJECT read_stream.c)
        target_compile_options(read_stream_object PRIVATE -D_GNU_SOURCE)

        # test with: IOTRACE_LOG_NAME=read_stream_static_test1 ./read_stream_static
        add_executable(read_stream_static
            $<TARGET_OBJECTS:read_stream_object>
            $<TARGET_OBJECTS:iotrace-object>
        )
        target_link_libraries(read_stream_static ${TESTS_LINK_OPTIONS_STATIC})
        target_link_libraries(read_stream_static ${TESTS_LINK_OPTIONS})
    endif()

    ##########################################################################################################

    if(WITH_POSIX_IO)
        # test with: IOTRACE_LOG_NAME=read_byte_and_stream_shared_test1 LD_PRELOAD=../src/libiotrace.so ./read_byte_and_stream_shared
        add_executable(read_byte_and_stream_shared read_byte_and_stream.c)
        target_link_libraries(read_byte_and_stream_shared ${TESTS_LINK_OPTIONS})
        target_link_libraries(read_byte_and_stream_shared iotrace_control)

        add_library(read_byte_and_stream_object OBJECT read_byte_and_stream.c)

        # test with: IOTRACE_LOG_NAME=read_byte_and_stream_static_test1 ./read_byte_and_stream_static
        add_executable(read_byte_and_stream_static
            $<TARGET_OBJECTS:read_byte_and_stream_object>
            $<TARGET_OBJECTS:iotrace-object>
        )
        target_link_libraries(read_byte_and_stream_static ${TESTS_LINK_OPTIONS_STATIC})
        target_link_libraries(read_byte_and_stream_static ${TESTS_LINK_OPTIONS})
    endif()

    ##########################################################################################################

    if(WITH_POSIX_IO)
        # test with: IOTRACE_LOG_NAME=write_stream_shared_test1 LD_PRELOAD=../src/libiotrace.so ./write_stream_shared
        add_executable(write_stream_shared write_stream.c)
        target_link_libraries(write_stream_shared ${TESTS_LINK_OPTIONS})

        add_library(write_stream_object OBJECT write_stream.c)

        # test with: IOTRACE_LOG_NAME=write_stream_static_test1 ./write_stream_static
        add_executable(write_stream_static
            $<TARGET_OBJECTS:write_stream_object>
            $<TARGET_OBJECTS:iotrace-object>
        )
        target_link_libraries(write_stream_static ${TESTS_LINK_OPTIONS_STATIC})
        target_link_libraries(write_stream_static ${TESTS_LINK_OPTIONS})
    endif()

    ##########################################################################################################

    if(WITH_POSIX_IO)
        # test with: IOTRACE_LOG_NAME=send_recv_shared_test1 LD_PRELOAD=../src/libiotrace.so ./send_recv_shared
        add_executable(send_recv_shared send_recv.c)
        target_compile_options(send_recv_shared PRIVATE "-D_GNU_SOURCE")
        target_link_libraries(send_recv_shared ${TESTS_LINK_OPTIONS})

        add_library(send_recv_object OBJECT send_recv.c)

        # test with: IOTRACE_LOG_NAME=send_recv_static_test1 ./send_recv_static
        add_executable(send_recv_static
            $<TARGET_OBJECTS:send_recv_object>
            $<TARGET_OBJECTS:iotrace-object>
        )

        target_compile_options(send_recv_static PRIVATE "-D_GNU_SOURCE")
        target_link_libraries(send_recv_static ${TESTS_LINK_OPTIONS_STATIC})
        target_link_libraries(send_recv_static ${TESTS_LINK_OPTIONS})
    endif()

    ##########################################################################################################

    if(WITH_POSIX_IO)
        #test with: IOTRACE_LOG_NAME=OpenMP_read_stream_shared_test1 LD_PRELOAD=../src/libiotrace.so OMP_NUM_THREADS=2 ./OpenMP_read_stream_shared
        add_executable(OpenMP_read_stream_shared OpenMP_read_stream.c)

        find_package(OpenMP)
        if(OpenMP_C_FOUND)
            target_link_libraries(OpenMP_read_stream_shared PUBLIC OpenMP::OpenMP_C)
        endif()
    endif()

    ##########################################################################################################

    if(WITH_POSIX_IO)
        # test with: IOTRACE_LOG_NAME=mmap_shared_test1 LD_PRELOAD=../src/libiotrace.so ./mmap_io
        add_executable(mmap_io mmap_io.c)
    endif()

    ##########################################################################################################

    if(WITH_POSIX_IO)
        # test with: IOTRACE_LOG_NAME=pipe_ipc_shared_test1 LD_PRELOAD=../src/libiotrace.so ./pipe_ipc
        add_executable(pipe_ipc pipe_ipc.c)
    endif()

    ##########################################################################################################

    if(WITH_POSIX_IO)
        # test with: IOTRACE_LOG_NAME=open_var_args_shared_test1 LD_PRELOAD=../src/libiotrace.so ./open_var_args
        add_executable(open_var_args open_var_args.c)
    endif()

    ##########################################################################################################

    if(WITH_POSIX_AIO)
        # test with: IOTRACE_LOG_NAME=read_async_byte_shared_test1 LD_PRELOAD=../src/libiotrace.so ./read_async_byte_shared
        add_executable(read_async_byte_shared read_async_byte.c)
        target_link_libraries(read_async_byte_shared ${TESTS_LINK_OPTIONS})

        add_library(read_async_byte_object OBJECT read_async_byte.c)

        # test with: IOTRACE_LOG_NAME=read_async_byte_static_test1 ./read_async_byte_static
        add_executable(read_async_byte_static
            $<TARGET_OBJECTS:read_async_byte_object>
            $<TARGET_OBJECTS:iotrace-object>
        )
        target_link_libraries(read_async_byte_static ${TESTS_LINK_OPTIONS_STATIC})
        target_link_libraries(read_async_byte_static ${TESTS_LINK_OPTIONS})
    endif()

    ##########################################################################################################

    if(WITH_MPI_IO)
        # test with: mpirun -np 4 -x IOTRACE_LOG_NAME=MPI_read_test1 -x LD_PRELOAD=../src/libiotrace.so ./MPI_read
        add_executable(MPI_read MPI_read.c)
        target_link_libraries(MPI_read PRIVATE ${TESTS_LINK_OPTIONS})
    endif()

    if(WITH_MPI_IO)
        # test with: mpirun -np 4 -x IOTRACE_LOG_NAME=MPI_read_test2 -x LD_PRELOAD=../src/libiotrace.so ./MPI_parallel_IO_single
        add_executable(MPI_parallel_IO_single MPI_parallel_IO_single.c)
        target_link_libraries(MPI_parallel_IO_single PRIVATE ${TESTS_LINK_OPTIONS})
    endif()

    if(WITH_MPI_IO)
        # test with: mpirun -np 4 -x IOTRACE_LOG_NAME=MPI_read_test3 -x LD_PRELOAD=../src/libiotrace.so ./MPI_parallel_read
        add_executable(MPI_parallel_read MPI_parallel_read.c)
        target_link_libraries(MPI_parallel_read PRIVATE ${TESTS_LINK_OPTIONS})
    endif()

    if(WITH_MPI_IO)
        # test with: mpirun -np 4 -x IOTRACE_LOG_NAME=MPI_parallel_iread -x LD_PRELOAD=../src/libiotrace.so ./MPI_parallel_iread
        add_executable(MPI_parallel_iread MPI_parallel_iread.c)
        target_link_libraries(MPI_parallel_iread PRIVATE ${TESTS_LINK_OPTIONS})
    endif()

    if(WITH_MPI_IO)
        # test with: mpirun -np 4 -x IOTRACE_LOG_NAME=MPI_immediate_test -x LD_PRELOAD=../src/libiotrace.so ./MPI_immediate_test
        add_executable(MPI_immediate_test MPI_immediate_test.c)
        target_link_libraries(MPI_immediate_test PRIVATE ${TESTS_LINK_OPTIONS})
    endif()

    if(WITH_MPI_IO)
        # whitelist should look like (without initial "# " per line):
        # MPI_File_open
        # MPI_File_seek
        # MPI_File_write
        # MPI_File_close

        # test with: rm mpi_file_io.txt && mpirun -np 4 -x IOTRACE_LOG_NAME=mpi_file_io_test1 -x IOTRACE_DATABASE_IP=127.0.0.1 -x IOTRACE_DATABASE_PORT=8086 -x IOTRACE_INFLUX_ORGANIZATION=hse -x IOTRACE_INFLUX_BUCKET=hsebucket -x IOTRACE_INFLUX_TOKEN=OXBWllU1poZotgyBlLlo2XQ_u4AYGYKQmdxvJJeotKRyvdn5mwjEhCXyOjyldpMmNt_9YY4k3CK-f5Eh1bN0Ng== -x IOTRACE_WHITELIST=./whitelist -x LD_PRELOAD=../src/libiotrace.so mpi_file_io 10
        add_executable(mpi_file_io mpi_file_io.c)
        target_link_libraries(mpi_file_io PRIVATE ${TESTS_LINK_OPTIONS})

	#add_executable(mpi_file_io_sleep mpi_file_io_sleep.c)
	#target_link_libraries(mpi_file_io_sleep PRIVATE ${TESTS_LINK_OPTIONS})

        add_executable(mpi_file_io_clear_caches mpi_file_io.c)
        target_compile_options(mpi_file_io_clear_caches PRIVATE "-DWANT_CLEAR_CACHES")
        target_link_libraries(mpi_file_io_clear_caches PRIVATE ${TESTS_LINK_OPTIONS})

        find_package(OpenMP)

        # test with: rm mpi_file_io.txt && mpirun -np 4 -x OMP_NUM_THREADS=2 -x IOTRACE_LOG_NAME=mpi_file_io_test1 -x IOTRACE_DATABASE_IP=127.0.0.1 -x IOTRACE_DATABASE_PORT=8086 -x IOTRACE_INFLUX_ORGANIZATION=hse -x IOTRACE_INFLUX_BUCKET=hsebucket -x IOTRACE_INFLUX_TOKEN=OXBWllU1poZotgyBlLlo2XQ_u4AYGYKQmdxvJJeotKRyvdn5mwjEhCXyOjyldpMmNt_9YY4k3CK-f5Eh1bN0Ng== -x IOTRACE_WHITELIST=./whitelist -x LD_PRELOAD=../src/libiotrace.so mpi_file_io 10
        add_executable(mpi_file_io_openmp mpi_file_io.c)
        target_compile_options(mpi_file_io_openmp PRIVATE "-DWANT_OPENMP")
        target_link_libraries(mpi_file_io_openmp PRIVATE ${TESTS_LINK_OPTIONS})
        if(OpenMP_C_FOUND)
            target_link_libraries(mpi_file_io_openmp PUBLIC OpenMP::OpenMP_C)
        endif()

        add_executable(mpi_file_io_openmp_clear_caches mpi_file_io.c)
        target_compile_options(mpi_file_io_openmp_clear_caches PRIVATE "-DWANT_OPENMP -DWANT_CLEAR_CACHES")
        target_link_libraries(mpi_file_io_openmp_clear_caches PRIVATE ${TESTS_LINK_OPTIONS})
        if(OpenMP_C_FOUND)
            target_link_libraries(mpi_file_io_openmp_clear_caches PUBLIC OpenMP::OpenMP_C)
        endif()
    endif()

    ##########################################################################################################

    if(WITH_MPI_IO AND WITH_POSIX_IO)
	find_package(OpenMP)

        # test with: rm -f mpi_random_file_io_*.txt && mpirun -np 4 -x OMP_NUM_THREADS=2 -x IOTRACE_LOG_NAME=mpi_random_file_io_test1 -x IOTRACE_DATABASE_IP=127.0.0.1 -x IOTRACE_DATABASE_PORT=8086 -x IOTRACE_INFLUX_ORGANIZATION=hse -x IOTRACE_INFLUX_BUCKET=hsebucket -x IOTRACE_INFLUX_TOKEN=OXBWllU1poZotgyBlLlo2XQ_u4AYGYKQmdxvJJeotKRyvdn5mwjEhCXyOjyldpMmNt_9YY4k3CK-f5Eh1bN0Ng== -x LD_PRELOAD=../src/libiotrace.so mpi_random_file_io 20 1 2 120 10 200 1234567
        add_executable(mpi_random_file_io mpi_random_file_io.c)
        target_compile_options(mpi_random_file_io PRIVATE "-DWANT_OPENMP")
        target_link_libraries(mpi_random_file_io PRIVATE ${TESTS_LINK_OPTIONS})
        target_link_libraries(mpi_random_file_io PRIVATE m)
	if(OpenMP_C_FOUND)
            target_link_libraries(mpi_random_file_io PUBLIC OpenMP::OpenMP_C)
        endif()

    endif()

    ##########################################################################################################

    if(WITH_MPI_IO)
        # whitelist should look like (without initial "# " per line):
        # MPI_File_open
        # MPI_File_seek
        # MPI_File_write
        # MPI_File_close

        # test with: rm mpi_file_io.txt && mpirun -np 4 -x IOTRACE_LOG_NAME=mpi_file_io_test1 -x IOTRACE_DATABASE_IP=127.0.0.1 -x IOTRACE_DATABASE_PORT=8086 -x IOTRACE_INFLUX_ORGANIZATION=hse -x IOTRACE_INFLUX_BUCKET=hsebucket -x IOTRACE_INFLUX_TOKEN=OXBWllU1poZotgyBlLlo2XQ_u4AYGYKQmdxvJJeotKRyvdn5mwjEhCXyOjyldpMmNt_9YY4k3CK-f5Eh1bN0Ng== -x IOTRACE_WHITELIST=./whitelist -x LD_PRELOAD=../src/libiotrace.so mpi_file_io_2 10
        # test with: rm mpi_file_io.txt && mpirun -np 4 -x IOTRACE_LOG_NAME=mpi_file_io_test1 -x LD_PRELOAD=../src/libiotrace.so mpi_file_io_2 10
        add_executable(mpi_file_io_2 mpi_file_io_2.c)
        target_link_libraries(mpi_file_io_2 PRIVATE ${TESTS_LINK_OPTIONS})

        find_package(OpenMP)

        # test with: rm mpi_file_io.txt && mpirun -np 4 -x OMP_NUM_THREADS=2 -x IOTRACE_LOG_NAME=mpi_file_io_test1 -x IOTRACE_DATABASE_IP=127.0.0.1 -x IOTRACE_DATABASE_PORT=8086 -x IOTRACE_INFLUX_ORGANIZATION=hse -x IOTRACE_INFLUX_BUCKET=hsebucket -x IOTRACE_INFLUX_TOKEN=OXBWllU1poZotgyBlLlo2XQ_u4AYGYKQmdxvJJeotKRyvdn5mwjEhCXyOjyldpMmNt_9YY4k3CK-f5Eh1bN0Ng== -x IOTRACE_WHITELIST=./whitelist -x LD_PRELOAD=../src/libiotrace.so mpi_file_io_2 10
        add_executable(mpi_file_io_2_openmp mpi_file_io_2.c)
        target_compile_options(mpi_file_io_2_openmp PRIVATE "-DWANT_OPENMP")
        target_link_libraries(mpi_file_io_2_openmp PRIVATE ${TESTS_LINK_OPTIONS})
        if(OpenMP_C_FOUND)
            target_link_libraries(mpi_file_io_2_openmp PUBLIC OpenMP::OpenMP_C)
        endif()
    endif()

    ##########################################################################################################

    if(WITH_MPI_IO)
        # whitelist should look like (without initial "# " per line):
        # MPI_File_open
        # MPI_File_seek
        # MPI_File_write
        # MPI_File_close

        # test with: rm mpi_file_io.txt && mpirun -np 4 -x IOTRACE_LOG_NAME=mpi_file_io_test1 -x IOTRACE_DATABASE_IP=127.0.0.1 -x IOTRACE_DATABASE_PORT=8086 -x IOTRACE_INFLUX_ORGANIZATION=hse -x IOTRACE_INFLUX_BUCKET=hsebucket -x IOTRACE_INFLUX_TOKEN=OXBWllU1poZotgyBlLlo2XQ_u4AYGYKQmdxvJJeotKRyvdn5mwjEhCXyOjyldpMmNt_9YY4k3CK-f5Eh1bN0Ng== -x IOTRACE_WHITELIST=./whitelist -x LD_PRELOAD=../src/libiotrace.so posix_file_io_random 10
        # test with: rm mpi_file_io.txt && mpirun -np 4 -x IOTRACE_LOG_NAME=mpi_file_io_test1 -x LD_PRELOAD=../src/libiotrace.so posix_file_io_random 10
        add_executable(posix_file_io_random posix_file_io_random.c)
        target_link_libraries(posix_file_io_random PRIVATE ${TESTS_LINK_OPTIONS})

        find_package(OpenMP)

        # test with: rm mpi_file_io.txt && mpirun -np 4 -x OMP_NUM_THREADS=2 -x IOTRACE_LOG_NAME=mpi_file_io_test1 -x IOTRACE_DATABASE_IP=127.0.0.1 -x IOTRACE_DATABASE_PORT=8086 -x IOTRACE_INFLUX_ORGANIZATION=hse -x IOTRACE_INFLUX_BUCKET=hsebucket -x IOTRACE_INFLUX_TOKEN=OXBWllU1poZotgyBlLlo2XQ_u4AYGYKQmdxvJJeotKRyvdn5mwjEhCXyOjyldpMmNt_9YY4k3CK-f5Eh1bN0Ng== -x IOTRACE_WHITELIST=./whitelist -x LD_PRELOAD=../src/libiotrace.so mpi_file_io_2 10
        add_executable(posix_file_io_random_openmp posix_file_io_random.c)
        target_compile_options(posix_file_io_random_openmp PRIVATE "-DWANT_OPENMP")
        target_link_libraries(posix_file_io_random_openmp PRIVATE ${TESTS_LINK_OPTIONS})
        if(OpenMP_C_FOUND)
            target_link_libraries(posix_file_io_random_openmp PUBLIC OpenMP::OpenMP_C)
        endif()
    endif()

    ##########################################################################################################

    if(WITH_POSIX_IO)
        # test with: IOTRACE_LOG_NAME=clone_test1 LD_PRELOAD=../src/libiotrace.so ./clone_shared
        add_executable(clone_shared clone.c)
        target_compile_options(clone_shared PRIVATE "-D_GNU_SOURCE")
        target_link_libraries(clone_shared ${TESTS_LINK_OPTIONS})

        add_library(clone_object OBJECT clone.c)
        target_compile_options(clone_object PRIVATE "-D_GNU_SOURCE")

        # test with: IOTRACE_LOG_NAME=clone_static_test1 ./clone_static
        add_executable(clone_static
            $<TARGET_OBJECTS:clone_object>
            $<TARGET_OBJECTS:iotrace-object>
        )
        target_compile_options(clone_static PRIVATE "-D_GNU_SOURCE")
        target_link_libraries(clone_static ${TESTS_LINK_OPTIONS_STATIC})
        target_link_libraries(clone_static ${TESTS_LINK_OPTIONS})
    endif()

    ##########################################################################################################

    if(WITH_POSIX_IO)
        # test with: rm -f mptf_*.txt && IOTRACE_LOG_NAME=multi_test1 LD_PRELOAD=../src/libiotrace.so OMP_NUM_THREADS=4 ./multi_shared 20 4
        # rm -f mptf_*.txt && IOTRACE_LOG_NAME=multi_test1 IOTRACE_DATABASE_IP=127.0.0.1 IOTRACE_DATABASE_PORT=8086 IOTRACE_INFLUX_ORGANIZATION=hse IOTRACE_INFLUX_BUCKET=hsebucket IOTRACE_INFLUX_TOKEN=OXBWllU1poZotgyBlLlo2XQ_u4AYGYKQmdxvJJeotKRyvdn5mwjEhCXyOjyldpMmNt_9YY4k3CK-f5Eh1bN0Ng== LD_PRELOAD=../src/libiotrace.so OMP_NUM_THREADS=4 ./multi_shared 20 4

        find_package(OpenMP)

        add_executable(multi_shared multi_processes_threads_files.c)
        target_compile_options(multi_shared PRIVATE "-D_GNU_SOURCE")
        target_link_libraries(multi_shared PRIVATE ${LINK_OPTIONS})
        if(OpenMP_C_FOUND)
            target_link_libraries(multi_shared PUBLIC OpenMP::OpenMP_C)
        endif()
    endif()

    ##########################################################################################################

    if (WITH_ALLOC)
        find_package(OpenMP REQUIRED)

        # test with: `IOTRACE_LOG_NAME=alloc_trace_shared_test1 LD_PRELOAD=../src/libiotrace.so ./alloc_trace`
        add_executable(alloc_trace alloc_trace.c)
        target_link_libraries(alloc_trace PUBLIC OpenMP::OpenMP_C)

        # test with: `IOTRACE_LOG_NAME=alloc_s_brk_shared_test1 LD_PRELOAD=../src/libiotrace.so ./alloc_s_brk`
        add_executable(alloc_s_brk alloc_s_brk.c)
    endif()
    
    ##########################################################################################################

    add_executable(dlsym dlsym.c)
    target_compile_options(dlsym PRIVATE "-D_GNU_SOURCE")
    target_link_libraries(dlsym PRIVATE "-ldl")

    ##########################################################################################################

    add_executable(influxdb_mock influxdb_mock.c)
    target_compile_options(influxdb_mock PRIVATE "-D_GNU_SOURCE")
    target_link_libraries(influxdb_mock PRIVATE ${LINK_OPTIONS})
    target_link_libraries(influxdb_mock pthread)

    ##########################################################################################################

    # test with: IOTRACE_LOG_NAME=init_thread_shared_test1 LD_PRELOAD=../src/libiotrace.so ./init_thread_shared
    add_executable(init_thread_shared init_thread_test.c)
    target_link_libraries(init_thread_shared ${TESTS_LINK_OPTIONS})

    add_library(init_thread_object OBJECT init_thread_test.c)

    # test with: IOTRACE_LOG_NAME=init_thread_static_test1 ./init_thread_static
    add_executable(init_thread_static
        $<TARGET_OBJECTS:init_thread_object>
        $<TARGET_OBJECTS:iotrace-object>
    )
    target_compile_options(init_thread_static PRIVATE "-D_GNU_SOURCE")
    target_link_libraries(init_thread_static ${TESTS_LINK_OPTIONS_STATIC})
    target_link_libraries(init_thread_static ${TESTS_LINK_OPTIONS})

    ##########################################################################################################



    # Build libiotrace as follows: `rm -rf build && mkdir build &&  (cd build && cmake DCMAKE_BUILD_TYPE=Debug  -DENABLE_OUTPUT=LOGFILE -DWITH_MPI_IO=OFF -DENABLE_REMOTE_CONTROL=OFF ..  &&  make -j6)`
    # test w/: `IOTRACE_LOG_NAME=test_cpp_linkage_shared_test1 LD_PRELOAD=../src/libiotrace.so ./test_cpp_linkage_shared`
    add_executable(test_cpp_linkage_shared test_cpp_linkage.cpp)

    # TODO: Apparently currently broken  ??!
    #add_executable(test_cpp_linkage_static test_cpp_linkage.cpp)
    #target_link_libraries(test_cpp_linkage_static $<TARGET_OBJECTS:iotrace-object>)
endif()
