include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)

check_include_files("stdlib.h" HAVE_STDLIB_H)
check_include_files("sys/types.h" HAVE_SYS_TYPES_H)
check_include_files("sys/stat.h" HAVE_SYS_STAT_H)
check_include_files("unistd.h" HAVE_UNISTD_H)

add_library(libiotrace STATIC
    init.c)

install(TARGETS libiotrace
        ARCHIVE DESTINATION lib
)
