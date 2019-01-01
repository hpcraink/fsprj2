check_include_files("stdbool.h" HAVE_STDBOOL_H)
check_include_files("stdlib.h" HAVE_STDLIB_H)
check_include_files("stdint.h" HAVE_STDINT_H)
check_include_files("sys/types.h" HAVE_SYS_TYPES_H)
check_include_files("sys/stat.h" HAVE_SYS_STAT_H)
check_include_files("unistd.h" HAVE_UNISTD_H)
check_include_files("pthread.h" HAVE_PTHREAD_H)

# Please keep in alphabetical order
add_library(iotrace STATIC
    init.c
)

install(TARGETS iotrace
        ARCHIVE DESTINATION lib
)
