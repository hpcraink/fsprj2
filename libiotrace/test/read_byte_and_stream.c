#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>
#include "libiotrace.h"

int main(void) {
    int fd;
    int ret;
    struct stat statbuf;
    char * buffer;
    char * p;
    long long toread;
    FILE * file;

    libiotrace_start_log();
    libiotrace_start_stacktrace_ptr();
    libiotrace_start_stacktrace_symbol();
    libiotrace_set_stacktrace_depth(0);

    fd = open("/etc/passwd", O_RDONLY);
    assert (0 <= fd);

    ret = fstat(fd, &statbuf);
    buffer = (char*) malloc(statbuf.st_size);
    assert (NULL != buffer);

    for (p=buffer, toread = statbuf.st_size; toread > 0; toread -= ret) {
        ret = read (fd, p, toread);
        assert (-1 != ret);
    }

    file = fdopen(fd, "r");
    assert (NULL != file);

    fclose(file);

    libiotrace_end_log();

    fd = open("/etc/passwd", O_RDONLY);
    assert (0 <= fd);

    libiotrace_start_log();
    libiotrace_set_stacktrace_depth(2);

    close(fd);
}
