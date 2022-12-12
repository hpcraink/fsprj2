#!/usr/bin/env python

"""
Assists in generating constants of function names for the file name resolution (fnres) module
"""

import sys
from string_utils import str_hash_djb2


FUNCTION_NAMES = [
    # --- POSIX ---
    # - Relevant for tracing + traceable -
    "open", "open64",
    "creat", "creat64",

    "openat",

    "mkostemp", "mkostemps",        # Note: Filenames are unknown (hence generic)
    "mkstemp", "mkstemps",

    "tmpfile", "tmpfile64",         # Note: Return stream

    "fopen", "fopen64",
    "freopen", "freopen64",

    "fdopen", "fileno",             # Special cases: Conversion / Duplication of fildes
    "dup", "dup2", "dup3",
    "fcntl",

    "sbrk",
    "mmap", "mmap64",               # Partially relevant 4 tracing (when NOT not file backed -> MAP_ANONYMOUS)
    "mremap",
    "msync",
    "madvise", "posix_madvise",     # Note: Only for debugging (map will be cloned AND will (in child) contain mem-mappings, which weren't forked by kernel (but those are currently NOT removed, see TODOs))


    "close", "fclose",              # Note: Only for debugging (application using already closed file will crash anyways)
    "fcloseall",                    # GNU extension
    "munmap",

    "dirfd",

    "__open", "__open64", "__open_2", "__open64_2",                      # Hardened functions (`-D_FORTIFY_SOURCE=2`)


    # - Pseudo-files -
    "epoll_create", "epoll_create1",
    "eventfd",
    "inotify_init", "inotify_init1",
    "memfd_create",
    "socket",

    "accept", "accept4",

    "pipe", "pipe2",                # Note: Create 2 fildes
    "socketpair",


    # ???
    # "readdir", "dirfd",
    # "popen",


    # - Traceable -
    "read", "write",
    "readv", "writev",                         # NOTE: v-suffix, e.g. 'readv': Vector-IO
    "pread", "pread64", "pwrite", "pwrite64",  # NOTE: p-prefix, e.g. 'pread': Doesn't advance file offset (vs. 'read'; hence avoids 'seek' calls) -> Uses as arg passed offset
    "preadv", "preadv64", "pwritev", "pwritev64",
    "preadv2", "preadv64v2", "pwritev2", "pwritev64v2",

    "lseek", "lseek64",

    "syncfs",
    "fsync", "fdatasync",


    "copy_file_range",              # Note: Takes in input- + output-fildes

    "select",


    "flockfile", "ftrylockfile", "funlockfile",
    "fwide",
    "fputc", "fputc_unlocked",
    "putw", "fputwc", "fputwc_unlocked",
    "fputs", "fputs_unlocked",
    "fputws", "fputws_unlocked",

    "getc", "getc_unlocked",
    "fgetc", "fgetc_unlocked", "getw", "fgetwc", "fgetwc_unlocked",
    "fgets", "fgets_unlocked", "fgetws", "fgetws_unlocked",
    "ungetc", "ungetwc",

    "getline", "getdelim",

    "fread", "fread_unlocked", "fwrite", "fwrite_unlocked",
    "fprintf", "fwprintf", "vfprintf", "vfwprintf",
    "fscanf", "fwscanf", "vfscanf", "vfwscanf",

    "feof", "feof_unlocked",
    "ferror", "ferror_unlocked",
    "clearerr", "clearerr_unlocked",

    "ftell", "ftello", "ftello64",
    "fseek", "fseeko", "fseeko64",
    "rewind",
    "fgetpos", "fgetpos64",
    "fsetpos", "fsetpos64",

    "fflush", "fflush_unlocked",
    "setvbuf", "setbuf", "setbuffer",
    "setlinebuf",

    "__freadable", "__fwritable", "__fsetlocking",      # GNU extensions

    "readdir",

    "connect",
    "bind",


    # - Inaccurate (since private mappings are NOT removed) -
    "sync",                         # Note: 'sync' doesn't take in fildes


    # - Not implemented yet -
    "sendmsg", "recvmsg",           # IPC: Not feasible anyways

    # - Irrelevant -
    "fork", "vfork",                # Handeled by libiotrace
    "execl", "execlp", "execle", "execv", "execvp", "execvpe",
    "pthread_create",



    # --- MPI ---
    # - Functions relevant for tracing + traceable -
    "MPI_File_open",
    "MPI_File_close",               # Note: Only for debugging (application using already closed file will crash anyways)

    "MPI_File_iread",               # Immediate functions use MPI_Request handle -> must be traced, otherwise traceable functions using the handle won't be traceable
    "MPI_File_iread_all",           # NOTE: No function for cleaning up immediate files --> BE CAREFUL !!
    "MPI_File_iread_at",
    "MPI_File_iread_at_all",
    "MPI_File_iwrite",
    "MPI_File_iwrite_all",
    "MPI_File_iwrite_at",
    "MPI_File_iwrite_at_all",


    # - Traceable -
    "MPI_File_read",                # Direct
    "MPI_File_read_all",
    "MPI_File_read_all_begin",
    "MPI_File_read_at",
    "MPI_File_read_at_all",
    "MPI_File_write",
    "MPI_File_write_all",
    "MPI_File_write_at",
    "MPI_File_write_at_all",
    "MPI_File_seek",
    "MPI_File_delete",
    "MPI_File_set_view",

    "MPI_Request_free",             # Immediate
    "MPI_Wait",
    "MPI_Waitall",



    # --- POSIX AIO ---
    "shm_open",
    "aio_init",
    "aio_cancel", "aio_cancel64",
    "aio_suspend", "aio_suspend64",
    "aio_fsync", "aio_fsync64",
    "aio_return", "aio_return64",
    "aio_error", "aio_error64",
    "lio_listio", "lio_listio64",
    "aio_write", "aio_write64",
    "aio_read", "aio_read64",



    # --- STDIO ---
    # - (nonexistent) fcts -
    "open_std_fd", "open_std_file",



    # --- Dynamic linking loader ---
    "dlopen",
    "dlmopen",



    # --- Dynamically allocated mem ---
    "malloc",
    "calloc",
    "realloc",
    "reallocarray",
    "free",



    # --- libiotrace internal (nonexistent) fcts ---
    "cleanup_process", "init_on_load"
]

PRINT_HASHES_IN_HEX = True

CONST_PREFIX = "CASE"


if __name__ == '__main__':
    hashed_fct_names = str_hash_djb2(FUNCTION_NAMES, PRINT_HASHES_IN_HEX)

    if hashed_fct_names != None:
        for string, hash in hashed_fct_names.items():
            print(f"#define {CONST_PREFIX}_{string.upper()} {hash}", end='\n')
        sys.exit(0)

    sys.exit(1)
