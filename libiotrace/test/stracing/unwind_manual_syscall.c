/**
 * Performs manual syscall (i.e., doesn't use glibc wrappers), bypassing the libiotrace wrappers
 */
#include <unistd.h>
#include <sys/syscall.h>
#include <fcntl.h>


int main(void) {
    int fd = (int)syscall(SYS_open, "/etc/hosts", O_RDONLY);
    syscall(SYS_close, fd);

    return 0;
}
