#include <ctype.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../common/error.h"
#include "trace/generated/syscallents.h"
#include "ptrace_utils.h"
#include "syscall_types.h"
#include "syscalls.h"
#include "common/utils.h"


/* -- Function prototypes -- */
static long from_regs_struct_get_syscall_arg(struct user_regs_struct_full *regs, int which);
static void fprint_str_esc(FILE *stream, char *str, size_t str_len);


/* -- Functions -- */
const char *syscalls_get_name(long syscall_nr) {
    if (syscall_nr >= 0 && syscall_nr <= MAX_SYSCALL_NUM) {
        const syscall_entry_t* const scall = &syscalls[syscall_nr];
        if (scall->name) {  /* NOTE: Syscall-nrs may be non-consecutive (i.e., array has empty slots) */
            return scall->name;
        }
    }
    return NULL;
}

long syscalls_get_nr(char* syscall_name) {
    for (int i = 0; i < SYSCALLS_ARR_SIZE; i++) {
        const syscall_entry_t* const scall = &syscalls[i];
        if (scall->name && !strcmp(syscall_name, scall->name)) {  /* NOTE: Syscall-nrs may be non-consecutive (i.e., array has empty slots) */
            return i;
        }
    }
    return -1L;
}


int syscall_to_scevent(pid_t tid, struct user_regs_struct_full *read_regs_ptr, scevent_t* event_buf_ptr) {
    event_buf_ptr->ts_in_ns = gettime();

    const long syscall_no = USER_REGS_STRUCT_SC_NO( (*read_regs_ptr) );
    switch(syscall_no) {
        case __SNR_open:
        case __SNR_openat:
        {
            const unsigned long scall_arg_fname_addr = (__SNR_open == syscall_no) ? USER_REGS_STRUCT_SC_ARG0( (*read_regs_ptr) ) : USER_REGS_STRUCT_SC_ARG1( (*read_regs_ptr) );
            const long scall_rtn_val = USER_REGS_STRUCT_SC_RTNVAL( (*read_regs_ptr) );

            char* ptrace_read_fname_ptr;
            const size_t ptrace_read_fname_len = ptrace_read_string(tid, scall_arg_fname_addr, -1, &ptrace_read_fname_ptr);

            event_buf_ptr->succeeded = -1 != scall_rtn_val;
            event_buf_ptr->type = OPEN;
            event_buf_ptr->fd = (int)scall_rtn_val;
            strncpy(event_buf_ptr->filename, ptrace_read_fname_ptr, SCEVENT_FILENAME_MAX);  // !!!!!!!!!!!!!!!!!!   TODO: CHECK NUL BYTE BUFFER SIZE     !!!!!!!!!!!!!!!!!!
            event_buf_ptr->filename[SCEVENT_FILENAME_MAX -1] = '\0';        // Make sure always NUL-terminated
            free(ptrace_read_fname_ptr);
            event_buf_ptr->filename_len = ptrace_read_fname_len;
        }
            return 0;


        case __SNR_close:
            event_buf_ptr->succeeded = -1 != (long)USER_REGS_STRUCT_SC_RTNVAL( (*read_regs_ptr) );
            event_buf_ptr->type = CLOSE;
            event_buf_ptr->fd = USER_REGS_STRUCT_SC_ARG0( (*read_regs_ptr) );
            return 0;


    /* TODO: ADD 'CONVERSION'-SUPPORT MORE SYSCALLS */


        default:
            return -1;
    }
}


void syscalls_print_args(__attribute__((unused)) pid_t tid, struct user_regs_struct_full *regs) {   // `user_regs_struct_full *regs` only for efficiency's sake (not necessary, could be fetched again ...)
    const long syscall_nr = USER_REGS_STRUCT_SC_NO((*regs));

    const syscall_entry_t* ent = NULL;
    int nargs = SYSCALL_MAX_ARGS;

    if ((syscall_nr >= 0 && syscall_nr <= MAX_SYSCALL_NUM) && syscalls[syscall_nr].name) {
        ent = &syscalls[syscall_nr];
        nargs = ent->nargs;
    } else {
        LOG_WARN("Unknown syscall w/ nr %ld", syscall_nr);
    }

    for (int arg_nr = 0; arg_nr < nargs; arg_nr++) {
        long arg = from_regs_struct_get_syscall_arg(regs, arg_nr);
        long type = ent ? ent->args[arg_nr] : ARG_PTR;      /* Default to `ARG_PTR` */

        switch (type) {
            case ARG_INT:
                fprintf(stderr, "%ld", arg);
                break;
            case ARG_STR: {
                const long bytes_to_read = (__SNR_write == syscall_nr || __SNR_read == syscall_nr) ?        // TODO: REVISE
                                           (from_regs_struct_get_syscall_arg(regs, 2)) :
                                           (-1);

                char* ptrace_read_str;
                size_t ptrace_read_str_len = ptrace_read_string(tid, arg, bytes_to_read, &ptrace_read_str);

                // fprintf(stderr, "\"%s\"", strval);
                fprintf(stderr, "\""); fprint_str_esc(stderr, ptrace_read_str, ptrace_read_str_len); fprintf(stderr, "\"");

                free(ptrace_read_str);
                break;
            }
            default:    /* e.g., ARG_PTR */
                fprintf(stderr, "0x%lx", (unsigned long)arg);
                break;
        }
        if (arg_nr != nargs -1)
            fprintf(stderr, ", ");
    }
}

static long from_regs_struct_get_syscall_arg(struct user_regs_struct_full *regs, int which) {
    switch (which) {
        case 0: return USER_REGS_STRUCT_SC_ARG0( (*regs) );
        case 1: return USER_REGS_STRUCT_SC_ARG1( (*regs) );
        case 2: return USER_REGS_STRUCT_SC_ARG2( (*regs) );
        case 3: return USER_REGS_STRUCT_SC_ARG3( (*regs) );
        case 4: return USER_REGS_STRUCT_SC_ARG4( (*regs) );
        case 5: return USER_REGS_STRUCT_SC_ARG5( (*regs) );

        default: return -1L;        /* Invalid */
    }
}

/*
 * Prints ASCII control chars in `str` using a hex representation
 * Doesn't rely on NUL-terminator (since arbitrary binary data
 * might incl. also `\0`)
 */
static void fprint_str_esc(FILE *stream, char *str, size_t str_len) {
    setlocale(LC_ALL, "C");

    for (unsigned int i = 0; i < str_len; i++) {
        const char c = str[i];
        if (isprint(c) && c != '\\') {
            if ('"' == c) { fputc('\\', stream); }  /* Escape '"' */
            fputc(c, stream);
        } else {
            fprintf(stream, "\\x%02x", (unsigned char)c);
        }
    }
}
