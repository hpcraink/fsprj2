#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <fcntl.h>



void test_not_filebacked_shared_mem_fork(const size_t mmap_num_arr_elements) {
    puts("\n\n--- `mmap`: Not file backed SHARED memory w/ `fork` ---\n");

    int* not_file_backed_shared_mem;
    const size_t not_file_backed_shared_mem_len = mmap_num_arr_elements * sizeof(*not_file_backed_shared_mem);


    /* - 0. `mmap` - */
    if (MAP_FAILED == (not_file_backed_shared_mem = mmap(NULL, not_file_backed_shared_mem_len, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0))) {
        perror("`mmap`'ing file failed: ");
        exit(1);
    } else {
        printf("-> `mmap`'ed memory [address=%p, length=%zu]\n", not_file_backed_shared_mem, not_file_backed_shared_mem_len);
    }

    /* - 1. Init w/ values - */
    puts("Initial values in the `mmap`'ed memory region:");
    for (size_t i = 0; i < mmap_num_arr_elements; i++){
        not_file_backed_shared_mem[i] = (i + 1);
        printf("\t%d", not_file_backed_shared_mem[i]);
    }
    puts("");


    /* - 2. `fork` - */
    pid_t pid;
    if (0 == (pid = fork())) {
    /* - 3. Child: Update values - */
        puts("\nCHILD: I'm updating the values in the meantime ...");
        for (size_t i = 0; i < mmap_num_arr_elements; i++) {
            not_file_backed_shared_mem[i] *= (not_file_backed_shared_mem[i] << 1);
        }
        puts("CHILD: I'm done, exiting now ...");
        exit(0);
    } else {
    /* - 4. Parent: Wait for child  ->  show updated values - */
        waitpid(pid, NULL, 0);
        puts("\nPARENT:");

        puts("Updated values in the mmap'ed memory region:");
        for (size_t i = 0; i < mmap_num_arr_elements; i++) {
            printf("\t%d", not_file_backed_shared_mem[i] );
        }
        puts("");
    }

    /* - 5. `munmap` - */
    if (-1 == munmap(not_file_backed_shared_mem, not_file_backed_shared_mem_len)) {
        perror("`munmap`'ing failed: ");
        exit(1);
    } else {
        printf("-> `munmap`'ed file in memory [address=%p, length=%zu]\n", not_file_backed_shared_mem, not_file_backed_shared_mem_len);
    }
}

void test_filebacked_anon_mem(const char* const file_to_be_mmaped, const size_t num_bytes) {
    puts("\n\n--- `mmap`: File backed anonymous memory ---\n");


    /* - 0. `open` to be mapped file - */
    int fd;
    if (-1 == (fd = open(file_to_be_mmaped, O_RDONLY))) {
        perror("`open`'ing file failed");
        exit(1);
    } else {
        printf("-> `open`'ed file [filename=%s, fd=%d]\n", file_to_be_mmaped, fd);
    }

    /* - 1. `mmap` file in memory - */
    char* file_backed_anon_mem;
    const size_t file_backed_anon_mem_len = num_bytes * sizeof(*file_backed_anon_mem);
    if (MAP_FAILED == (file_backed_anon_mem = mmap(NULL, file_backed_anon_mem_len, PROT_READ, MAP_FILE | MAP_PRIVATE, fd, 0))) {
        perror("`mmap`'ing file failed");
        exit(1);
    } else {
        printf("-> `mmap`'ed file in memory [address=%p, length=%zu]\n", file_backed_anon_mem, file_backed_anon_mem_len);
    }

    /* - 2. `printf` mmap'ed file contents (to stdout) - */
    for (size_t i=0; (i < num_bytes) && ('\0' != file_backed_anon_mem[i]); i++) {
        printf("%c", file_backed_anon_mem[i]);
    }

    /* - 3. cleanup (`munmap` + `close`) - */
    if (-1 == munmap(file_backed_anon_mem, file_backed_anon_mem_len)) {
        perror("`munmap`'ing failed");
        exit(1);
    } else {
        printf("-> `munmap`'ed file in memory [address=%p, length=%zu]\n", file_backed_anon_mem, file_backed_anon_mem_len);
    }

    if (-1 == close(fd)) {
        perror("`close`'ing file failed");
        exit(1);
    } else {
        printf("-> `close`'ed file [filename=%s, fd=%d]\n", file_to_be_mmaped, fd);
    }
}



int main(void) {

    test_filebacked_anon_mem("/etc/fstab", 100);
    test_not_filebacked_shared_mem_fork(100);

    return 0;
}
