#!/usr/bin/python3

'''
Generates constants necessary for using switch on strings
'''

# --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
'''
C code for switch-statement:

// djb2 by Dan Bernstein
static u_int64_t hash_string(const char *str) {
    unsigned long hash = 5381;
    int c;

    while ((c = *str++)) hash = ((hash << 5) + hash) + c;
    return hash;
}

#define SWITCH_FNAME(function_name) switch (hash_string(function_name))

// ... Generated constants
'''

# djb2 by Dan Bernstein


def hash_djb2(string, hash_max_num):
    hash = 5381
    for char in string:
        hash = ((hash << 5) + hash) + ord(char)
    return (hash & hash_max_num)
# --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------


def print_constants_for_switch_strings(cases, hash_function, hash_max_num, print_hex):
    generated_hashes = []           # For checking hash-collisions

    for case in cases:
        hash = hash_function(case, hash_max_num)

        generated_hashes.append(hash)
        print(
            f"#define CASE_{case.upper()} {hex(hash) if print_hex else hash}", end='\n')

    if len(generated_hashes) != len(set(generated_hashes)):
        print("!! WARNING: POTENTIAL HASH COLLISION !!")


ULONG_MAX = 0xffffffffffffffff

case_strings = [
    # --- POSIX ---
    # - Relevants relevant for tracing + traceable -
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

    "mmap", "mmap64",               # Partially relevant 4 tracing (when NOT not file backed -> MAP_ANONYMOUS)
    "mremap",
    "msync",
    "madvise", "posix_madvise",     # Note: Only for debugging (map will be cloned AND will (in child) contain mem-mappings, which weren't forked by kernel (but those are currently NOT removed, see TODOs))


    "close", "fclose",              # Note: Only for debugging (application using already closed file will crash anyways)
    "fcloseall",
    "munmap",


    # - Pseudo-files -
    "accept", "accept4",
    "epoll_create", "epoll_create1",
    "eventfd",
    "inotify_init", "inotify_init1",
    "memfd_create",
    "socket",

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


    "flockfile", "ftrylockfile", "funlockfile",
    "fwide",
    "fputc", "fputc_unlocked",
    "putw", "fputwc", "fputwc_unlocked",
    "fputs", "fputs_unlocked",
    "fputws", "fputws_unlocked",

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


    # - Inaccurate (since private mappings are NOT removed) -
    "sync",                         # Note: 'sync' doesn't take in fildes


    # - Not implemented yet -
    "sendmsg", "recvmsg",           # IPC: Not feasible anyways

    # - Irrelevant -
    "fork", "vfork",                # Handeled by libiotrace


    # - libiotrace internal (nonexistent) fcts -
    "open_std_fd", "open_std_file",
    "cleanup", "init_on_load",



    # --- MPI ---
    # - Functions relevant for tracing + traceable -
    "MPI_File_open",
    "MPI_File_close",               # Note: Only for debugging (application using already closed file will crash anyways)
    "MPI_Request_free",

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

    "MPI_Wait",                     # Immediate
    "MPI_Waitall"
]

print_in_hex = True

print_constants_for_switch_strings(
    case_strings, hash_djb2, ULONG_MAX, print_in_hex)
