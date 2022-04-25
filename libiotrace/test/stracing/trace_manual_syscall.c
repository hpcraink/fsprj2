/**
 * Performs some syscalls manually (i.e., doesn't utilize glibc wrappers), thereby,
 * bypassing the libiotrace wrappers  -or-  via glibc wrappers
 * May be used to test libiotrace's filename resolution or syscall tracing capabilities
 */
#include <unistd.h>
#include <sys/syscall.h>
#include <fcntl.h>
#include <string.h>

#include "../common/error.h"


int main(int argc, char** argv) {

    if (! (2 == argc && !strcmp("--use-glibc-wrapper", argv[1])) ) {
        int fd = DIE_WHEN_ERRNO( (int)syscall(SYS_open, "/dev/stdout", O_WRONLY) );

        const char* const str_to_write = "Used syscall fct\n";
        DIE_WHEN_ERRNO( write(fd, str_to_write, strlen(str_to_write)) );

        DIE_WHEN_ERRNO( (int)syscall(SYS_close, fd) );


    } else {
        int fd = DIE_WHEN_ERRNO( open("/dev/stdout", O_WRONLY) );

        const char* const str_to_write = "Used glibc wrapper fct\n";
        DIE_WHEN_ERRNO( write(fd, str_to_write, strlen(str_to_write)) );

        close(fd);
    }

    return 0;
}
