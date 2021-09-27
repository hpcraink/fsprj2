/**
* Tests whether `open` w/ var-args is traced
*/

#include <stdio.h>
#include <fcntl.h>      /* `open` */
#include <unistd.h>     /* `close` */
#include <stdlib.h>     /* `exit` */

/* - Globals - */
const char* const TESTFILE_1 = "/etc/fstab";
const char* const TESTFILE_2 = "/tmp/test2";        /* Should be automatically cleaned up after reboot */



void validate_and_print_open_result(int returned_fd, const char* passed_filename) {
    if (-1 == returned_fd) {
        perror("`open`'ing file failed");
        exit(1);
    } else {
        printf("-> `open`'ed file [filename=%s, fd=%d]\n", passed_filename, returned_fd);
    }
}


int main(void) {

    int fd;
/* -- `open` w/ 2 args -- */
    puts("-- Calling `open` w/ 2 args --");
    fd = open(TESTFILE_1, O_RDONLY);
    validate_and_print_open_result(fd, TESTFILE_1);
    close(fd);


/* -- `open` w/ 3 args -- */
    puts("-- Calling `open` w/ 3 args --");
    fd = open(TESTFILE_2, O_CREAT, 0666);         /* NOTE: Prefix `0` since octal value; ? actual permissions end up being differt due to `umask` ? */
    validate_and_print_open_result(fd, TESTFILE_2);
    close(fd);


    return 0;
}
