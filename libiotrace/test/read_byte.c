#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>

int main(void) {
    int fd;
    int ret;
    struct stat statbuf;
    char * buffer;
    char * p;
    long long toread;

    fd = open("/etc/passwd", O_RDONLY);
    assert (0 <= fd);

    ret = fstat(fd, &statbuf);
    buffer = (char*) malloc(statbuf.st_size);
    assert (NULL != buffer);

    for (p=buffer, toread = statbuf.st_size; toread > 0; toread -= ret) {
        ret = read (fd, p, toread);
        assert (-1 != ret);
    }
    printf ("Read %ld Bytes\n", statbuf.st_size);
 
    ret = close (fd);

    return ret;
}
