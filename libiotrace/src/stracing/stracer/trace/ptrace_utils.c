#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <sys/ptrace.h>

#include "ptrace_utils.h"

#include "../common/error.h"


/* -- Functions -- */
char *ptrace_read_string(pid_t tid, unsigned long addr) {
    size_t read_bytes = 0;
    unsigned long ptrace_read_word;

/* Allocate memory as buffer for string to be read */
    char *read_str;
    size_t read_str_size_bytes = 2048;
    if (!(read_str = malloc(read_str_size_bytes))) {
        LOG_ERROR_AND_EXIT("`malloc`: Failed to allocate memory");
    }

/* Read string using ptrace */
    while (1) {
    /* Increase buffer size if too small */
        if (read_bytes + sizeof(ptrace_read_word) > read_str_size_bytes) {
            read_str_size_bytes *= 2;
            if (!(read_str = realloc(read_str, read_str_size_bytes))) {
                LOG_ERROR_AND_EXIT("`realloc`: Failed to allocate memory");
            }
        }

    /* Read from tracee (each time one word) */
        errno = 0;
        ptrace_read_word = ptrace(PTRACE_PEEKDATA, tid, addr + read_bytes);
        if (errno) {
            read_str[read_bytes] = '\0';
            break;
        }
    /* Append read word to buffer */
        memcpy(read_str + read_bytes, &ptrace_read_word, sizeof(ptrace_read_word));
    /* Read end of string ? */
        if (memchr(&ptrace_read_word, 0, sizeof(ptrace_read_word)) != NULL) {
            break;
        }
    /* Update read_bytes counter */
        read_bytes += sizeof(ptrace_read_word);
    }
    return read_str;
}
