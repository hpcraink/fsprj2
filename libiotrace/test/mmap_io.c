#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "common/error.h"


// ---------------- ---------------- ---------------- ---------------- ---------------- ---------------- ---------------- ---------------- ---------------- ----------------
void test_not_filebacked_shared_mem_fork(const int mmap_num_arr_elements) {
    puts("\n\n--- `mmap`: Not file backed SHARED memory w/ `fork` ---\n");

    int* not_file_backed_shared_mem;
    const int not_file_backed_shared_mem_len = mmap_num_arr_elements * sizeof(*not_file_backed_shared_mem);

/* - 0. `mmap`(2) - */
    if (MAP_FAILED == (not_file_backed_shared_mem =
            mmap(
                    NULL,
                    not_file_backed_shared_mem_len,
                    PROT_READ | PROT_WRITE,
                    MAP_SHARED | MAP_ANONYMOUS,
                    -1,
                    0)
            )) {
        LOG_ERROR_AND_DIE("`mmap`'ing file failed: ");
    }
    printf("-> `mmap`'ed memory [address=%p, length=%d]\n", (void*)not_file_backed_shared_mem, not_file_backed_shared_mem_len);

/* - 1. Init w/ values - */
    puts("Initial values in the `mmap`'ed memory region:");
    for (int i = 0; i < mmap_num_arr_elements; i++){
        not_file_backed_shared_mem[i] = (i + 1);
        printf("\t%d", not_file_backed_shared_mem[i]);
    }
    puts("");


/* - 2. `fork`(2) - */
    pid_t child_pid = DIE_WHEN_ERRNO( fork() );
    /* - 2.1. Child: Update values - */
    if (0 == child_pid) {
        puts("\nCHILD: I'm reversing the values in the meantime ...");
        for (int i = 0; i < mmap_num_arr_elements / 2; ++i) {
            int tmp = not_file_backed_shared_mem[i];
            int j = mmap_num_arr_elements - i - 1;
            not_file_backed_shared_mem[i] = not_file_backed_shared_mem[j];
            not_file_backed_shared_mem[j] = tmp;
        }
        puts("CHILD: I'm done, exiting now ...");
        exit(0);

        /* - 2.2. Parent: Wait for child  ->  show updated values - */
    } else {
        waitpid(child_pid, NULL, 0);
        puts("\nPARENT:");

        puts("Updated values in the mmap'ed memory region:");
        for (int i = 0; i < mmap_num_arr_elements; i++) {
            printf("\t%d", not_file_backed_shared_mem[i] );
        }
        puts("");
    }

/* - 3. `munmap` - */
    DIE_WHEN_ERRNO( munmap(not_file_backed_shared_mem, not_file_backed_shared_mem_len) );
    printf("-> `munmap`'ed file in memory [address=%p, length=%d]\n", (void*)not_file_backed_shared_mem, not_file_backed_shared_mem_len);
}

// ---------------- ---------------- ---------------- ---------------- ---------------- ---------------- ---------------- ---------------- ---------------- ----------------

void test_filebacked_anon_mem(const char* const file_to_be_mmaped) {
    puts("\n\n--- `mmap`: File backed anonymous memory ---\n");

/* - 0. `open` to be mapped file + retrieve size in bytes in `fstat` - */
    int fd = DIE_WHEN_ERRNO( open(file_to_be_mmaped, O_RDONLY) );
    struct stat stat_info;
    DIE_WHEN_ERRNO( fstat(fd, &stat_info) );

/* - 1. `mmap` file in memory - */
    char* file_backed_anon_mem;
    const int file_backed_anon_mem_len = stat_info.st_size;
    printf("-> To be `mmap`ed file [filename=%s, fd=%d, size in bytes=%d]\n", file_to_be_mmaped, fd, file_backed_anon_mem_len);
    if (MAP_FAILED == (file_backed_anon_mem =
            mmap(
                    NULL,
                    file_backed_anon_mem_len,
                    PROT_READ,
                    MAP_FILE | MAP_PRIVATE,
                    fd,
                    0)
                )) {
        LOG_ERROR_AND_DIE("`mmap`'ing file failed: ");
    }
    printf("-> `mmap`'ed file in memory [address=%p, length=%d]\n", (void*)file_backed_anon_mem, file_backed_anon_mem_len);

/* - 2. `printf` mmap'ed file contents (to stdout) - */
    puts("Contents:");
    for (int i = 0; (i < file_backed_anon_mem_len) && ('\0' != file_backed_anon_mem[i]); i++) {
        printf("%c", file_backed_anon_mem[i]);
    }

/* - 3. cleanup (`munmap` + `close`) - */
    DIE_WHEN_ERRNO( munmap(file_backed_anon_mem, file_backed_anon_mem_len) );
    printf("-> `munmap`'ed file in memory [address=%p, length=%d]\n", (void*)file_backed_anon_mem, file_backed_anon_mem_len);

    close(fd);
    printf("-> `close`'ed file [filename=%s, fd=%d]\n", file_to_be_mmaped, fd);
}
// ---------------- ---------------- ---------------- ---------------- ---------------- ---------------- ---------------- ---------------- ---------------- ----------------



int main(void) {
    test_filebacked_anon_mem("/etc/fstab");
    test_not_filebacked_shared_mem_fork(100);

    return 0;
}
