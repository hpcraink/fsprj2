check_include_files("stdbool.h" HAVE_STDBOOL_H)
check_include_files("stdlib.h" HAVE_STDLIB_H)
check_include_files("stdint.h" HAVE_STDINT_H)
check_include_files("sys/fcntl.h" HAVE_SYS_FCNTL_H)
check_include_files("sys/types.h" HAVE_SYS_TYPES_H)
check_include_files("sys/stat.h" HAVE_SYS_STAT_H)
check_include_files("sys/syscall.h" HAVE_SYS_SYSCALL_H)
check_include_files("unistd.h" HAVE_UNISTD_H)
check_include_files("pthread.h" HAVE_PTHREAD_H)

# Please keep in alphabetical order
set(SOURCE
    event.c
    init.c
    os.c
    posix_io.c
)

add_library(iotrace_shared SHARED ${SOURCE})

add_library(iotrace_static STATIC ${SOURCE})

add_library(iotrace_object OBJECT ${SOURCE})

# needed for dlsym-function
target_link_libraries(iotrace_shared PRIVATE ${CMAKE_DL_LIBS})

# needed for the RTLD_NEXT macro
target_compile_options(iotrace_shared PRIVATE -D_GNU_SOURCE)
target_compile_options(iotrace_static PRIVATE -D_GNU_SOURCE)
target_compile_options(iotrace_object PRIVATE -D_GNU_SOURCE)

# object for static linking needs
target_compile_options(iotrace_object PRIVATE -DIO_LIB_STATIC)

#install(TARGETS iotrace_static
#        ARCHIVE DESTINATION lib
#)
