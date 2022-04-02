/**
 * Performs manual syscall (i.e., doesn't use glibc wrappers), bypassing the libiotrace wrappers
 * May be used to test filename resolution or syscall tracing
 */
#include <unistd.h>
#include <sys/syscall.h>
#include <fcntl.h>
#include <string.h>

#include "../common/error.h"


int main(void) {
    int fd = DIE_WHEN_ERRNO( (int)syscall(SYS_open, "/dev/stdout", O_WRONLY) );

    const char* const str_to_write = "blablablablalba\n";
    DIE_WHEN_ERRNO( write(fd, str_to_write, strlen(str_to_write)) );

    DIE_WHEN_ERRNO( (int)syscall(SYS_close, fd) );

    return 0;
}
