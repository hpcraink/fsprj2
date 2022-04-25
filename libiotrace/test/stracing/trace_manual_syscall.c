/**
 * Performs some syscalls manually (i.e., doesn't utilize glibc wrappers), thereby,
 * bypassing the libiotrace wrappers  -or-  via glibc wrappers
 * May be used to test libiotrace's filename resolution or syscall tracing capabilities
 */
#include <unistd.h>
#include <sys/syscall.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>

#include "../common/error.h"


/* -- Globals -- */
int g_fd = -1;


void* open_routine(void* arg) {
    const int use_glibc_wrapper = *(int*)arg;

    g_fd = (!use_glibc_wrapper) ?
            DIE_WHEN_ERRNO( (int)syscall(SYS_open, "/dev/stdout", O_WRONLY) ) :
            DIE_WHEN_ERRNO( open("/dev/stdout", O_WRONLY) );

    return NULL;
}



int main(int argc, char** argv) {

    int cli_use_glibc_wrapper = 0,
        cli_perform_open_in_pthread = 0;

    for (int i = 1; i < argc; ++i) {  // NOTE: Cli arg 0 = Program name
        if (!strcmp("--use-glibc-wrapper", argv[i])) {
            cli_use_glibc_wrapper = 1;
            continue;
        }
        if (!strcmp("--perform-open-in-pthread", argv[i])) {
            cli_perform_open_in_pthread = 1;
            continue;
        }
        fprintf(stderr, "Invalid cli arg\n");
        exit(1);
    }


/* Perform open */
    if (cli_perform_open_in_pthread) {
        puts("Performing open operation of file in separate thread");
        pthread_t open_thread;
        pthread_create(&open_thread, NULL, open_routine, &cli_use_glibc_wrapper);

        pthread_join(open_thread, NULL);
    } else {
        (void)open_routine(&cli_use_glibc_wrapper);
    }


/* Perform write -> close */
    if (! cli_use_glibc_wrapper) {
        const char* const str_to_write = "File was opened via `syscall`(2) \n";
        DIE_WHEN_ERRNO( write(g_fd, str_to_write, strlen(str_to_write)) );

        DIE_WHEN_ERRNO( (int)syscall(SYS_close, g_fd) );  g_fd = -1;

    } else {
        const char* const str_to_write = "File was opened via glibc wrapper fct. `open`(2)\n";
        DIE_WHEN_ERRNO( write(g_fd, str_to_write, strlen(str_to_write)) );

        close(g_fd);  g_fd = -1;
    }

    return 0;
}
