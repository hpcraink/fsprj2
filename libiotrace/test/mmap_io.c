#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <fcntl.h>



// Q: `mmap` not file-backed => Basically same as using `malloc` ?
void test_not_filebacked_shared_mem(const size_t mmap_num_arr_elements) {
    puts("\n\n--- mmap: Not file backed SHARED memory ---\n");

    int* not_file_backed_shared_mem;
    const size_t not_file_backed_shared_mem_len = mmap_num_arr_elements * sizeof(*not_file_backed_shared_mem);


    /* - 0. `mmap` - */
    if (MAP_FAILED == (not_file_backed_shared_mem = mmap(NULL, not_file_backed_shared_mem_len, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0))) {
        perror("`mmap`'ing file failed: ");
        exit(1);
    }

    /* - 1. Init w/ values - */
    puts("Initial values in the `mmap`'ed memory region:");
    for (int i = 0; i < mmap_num_arr_elements; i++){
        not_file_backed_shared_mem[i] = (i + 1);
        printf("\t%d", not_file_backed_shared_mem[i]);
    }
    puts("");


    /* - 2. `fork` - */
    pid_t child_pid;
    if (0 == (child_pid = fork())) {
        /* - 3. Child: Update values - */
        puts("\nCHILD: I'm updating the values in the meantime ...");
        for (int i = 0; i < mmap_num_arr_elements; i++) {
            not_file_backed_shared_mem[i] *= (not_file_backed_shared_mem[i] << 1);
        }
    }
    else {
        /* - 4. Parent: Wait for child  ->  show updated values - */
        waitpid(child_pid, NULL, 0);
        puts("\nPARENT:");

        puts("Updated values in the mmap'ed memory region:");
        for (int i = 0; i < mmap_num_arr_elements; i++) {
            printf("\t%d", not_file_backed_shared_mem[i] );
        }
        puts("");
    }

    /* - 5. `munmap` - */
    if (-1 == munmap(not_file_backed_shared_mem, not_file_backed_shared_mem_len)) {
        perror("`munmap`'ing failed: ");
        exit(1);
    }
}

void test_filebacked_anon_mem(const char* const file_to_be_mmaped, const size_t num_bytes) {
    puts("\n\n--- mmap: File backed anonymous memory ---\n");


    /* - 0. `open` to be mapped file - */
    int fd;
    if (-1 == (fd = open(file_to_be_mmaped, O_RDONLY))) {
        perror("`open`'ing file failed: ");
        exit(1);
    }

    /* - 1. `mmap` file in memory - */
    char* file_backed_anon_mem;
    const size_t file_backed_anon_mem_len = num_bytes * sizeof(*file_backed_anon_mem);
    if (MAP_FAILED == (file_backed_anon_mem = mmap(NULL, file_backed_anon_mem_len, PROT_READ, MAP_FILE | MAP_PRIVATE, fd, 0))) {
        perror("`mmap`'ing file failed: ");
        exit(1);
    }

    /* - 2. `printf` mmap'ed file contents (to stdout) - */
    for (int i=0; (i < num_bytes) && ('\0' != file_backed_anon_mem[i]); i++) {
        printf("%c", file_backed_anon_mem[i]);
    }

    /* - 3. cleanup (`munmap` + `close`) - */
    if (-1 == munmap(file_backed_anon_mem, file_backed_anon_mem_len)) {
        perror("`munmap`'ing failed: ");
        exit(1);
    }

    if (-1 == close(fd)) {
        perror("`close`'ing file failed: ");
        exit(1);
    }
}



int main(void) {

    test_filebacked_anon_mem("/etc/fstab", 700 - 34);
    test_not_filebacked_shared_mem(633 + 33);

    return 0;
}