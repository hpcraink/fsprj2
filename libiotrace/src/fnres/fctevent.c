/**
 * TODOS:
 *  - Remove memory-mappings after `fork`
 *    - Save `mmap` mapping type (`MAP_SHARED` flag) in struct (requires changing `CASE_MMAP`)
 *    - Implement `CASE_MADVISE`
 *    - Implement callable hook `reset_on_fork` (which sets flag `got_forked` to indicate in next wrapper call a necessary removal of map values; called by `reset_values_in_forked_process` in event.c)
 *  - Support multiple traced files in `basic` struct (some function-events affect multiple files) -> `sync`, `floseall`, `copy_write_data` and `MPI_Waitall`
 *      - Note regarding `copy_write_data`: Implemented using 2 calls to this module -> Check enum during call whether read or write data + assemble it
 *  - Fildes 'extracted' from Streams (via `fileno`) must be deleted as soon as the original stream gets closed (e.g., via `fclose`) for more precise tracing (currently not traced function creating same fildes may result in wrong `traced_filename`)
 *  - DL_IO, POSIX_AIO
 *    - Things to keep in mind regarding DL_IO:
 *      - Additionally tracing DL_IO might incur a SIGNIFICANT increase in open files -> fnmap's  initial size of 100 might NOT be sufficient
 *      - Currently, `dlsym`, `dlclose`, ... are NOT wrapped, therefore, ...
 *        - only setting traced_filename (while not adding to fnmap) is atm sufficient
 *        - the fnmap will never be cleaned up -> memory leak ?
 *
 * KNOWN ISSUES:
 *  - `fnmap_destroy` currently LEAKS MEMORY since `__del_hook` isn't executed for each item in fnmap (not a very serious issue though since the function will only be called once the observed program exits, i.e., the OS will cleanup)
 *  - On an `exec*` call, the global file-map (of the process) will be overwritten, removing fildes which WILL BE inherited (since they weren't opened w/ `O_CLOEXEC` flag) by the new (replaced) executable
 */
#include "fctevent.h"
#include "internal/fctnconsts.h"
#include "internal/fnmap.h"
#include "../error.h"

#include <string.h>
#include <stdbool.h>        /* Be careful: Insertion order might cause issues w/ "../libiotrace_include_struct.h" */

// #include <unistd.h>    // DEBUGGING


/* -- Globals -- */
static bool got_init = false;


/* -- Function prototypes for helper functions -- */
static void create_fnmap_key_using_vals(file_handle_type_t type, void *id, size_t mmap_length, fnmap_key_t *new_key);
static void create_fnmap_key_using_fctevent_file_type(struct basic *fctevent, fnmap_key_t *new_key);
static void create_fnmap_key_using_fctevent_function_data(struct basic *fctevent, fnmap_key_t *new_key1, fnmap_key_t *new_key2);

static const char* get_file_name_from_fctevent_function_data(struct basic *fctevent);


/* -- Macros -- */
#define SET_TRACED_FNAME_FOR_FCTEVENT(fctevent, traced_fname) do {                                                                            \
  (fctevent)->traced_filename[sizeof((fctevent)->traced_filename) - 1] = '\0'; /* ensure there is a terminating '\0' after strncpy */         \
  strncpy((fctevent)->traced_filename, traced_fname, sizeof((fctevent)->traced_filename) - 1 /* save space for terminating '\0' */);          \
  /* LIBIOTRACE_DEBUG("Set for fctevent [%s] traced_filename: %s", (fctevent)->function_name, (fctevent)->traced_filename);    // DEBUGGING */\
} while(0)

#define RETURN_IF_FCTEVENT_FAILED(fctevent) do {                                        \
  if (error == (fctevent)->return_state) {                                              \
    /* LIBIOTRACE_DEBUG("Failed fectevent, not adding to trace ...");    // DEBUGGING */\
    return;                                                                             \
  }                                                                                     \
} while(0)

#define ADD_OR_UPDATE_FNAME_IN_TRACE_USING_FCTEVENT_FILE_TYPE(fctevent, filename) do {\
  fnmap_key_t insert_key;                                                             \
  create_fnmap_key_using_fctevent_file_type(fctevent, &insert_key);                   \
  fnmap_add_or_update(&insert_key, filename);                                         \
} while(0)

#define ADD_OR_UPDATE_FNAME_IN_TRACE_USING_FCTEVENT_FUNCTION_DATA(fctevent, filename) do {           \
  fnmap_key_t insert_key1, insert_key2;                                                              \
  create_fnmap_key_using_fctevent_function_data(fctevent, &insert_key1, &insert_key2);               \
  fnmap_add_or_update(&insert_key1, filename);                                                       \
                                                                                                     \
  if (__void_p_enum_function_data_file_pair == (fctevent)->__void_p_enum_function_data ||            \
        __void_p_enum_function_data_socketpair_function == (fctevent)->__void_p_enum_function_data) {\
    fnmap_add_or_update(&insert_key2, filename);                                                     \
  }                                                                                                  \
} while(0)

#define RMV_FNAME_FROM_TRACE_USING_FCTEVENT_FILE_TYPE(fctevent) do {\
  fnmap_key_t delete_key;                                           \
  create_fnmap_key_using_fctevent_file_type(fctevent, &delete_key); \
  fnmap_remove(&delete_key);                                        \
} while(0)

#define IF_FOUND_FNAME_IN_TRACE_USING_FCTEVENT_FILE_TYPE(fctevent, search_key, search_found_fname)\
  create_fnmap_key_using_fctevent_file_type((fctevent), &(search_key));                           \
  if (!fnmap_get(&(search_key), &(search_found_fname)))

#define IF_FOUND_FNAME_IN_TRACE_USING_FCTEVENT_FUNCTION_DATA(fctevent, search_key, search_found_fname)\
  create_fnmap_key_using_fctevent_function_data((fctevent), &(search_key), NULL);                     \
  if (!fnmap_get(&(search_key), &(search_found_fname)))

#define ELSE_SET_FOR_TRACED_FNAME_NOT_FOUND(fctevent) else {        \
  SET_TRACED_FNAME_FOR_FCTEVENT(fctevent, FNAME_SPECIFIER_NOTFOUND);\
}


/* -- Functions -- */
/**
 * Initializes module; MUST be called prior usage  (by `init_process` in event.c)
 */
void fnres_init(long fnmap_max_size) {
    if (!got_init) {
        fnmap_create(fnmap_max_size);

        got_init = true;
        // LIBIOTRACE_DEBUG("Init done [fnmap_max_size = %zu]", fnmap_max_size);    // DEBUGGING
    } /*else {
        LIBIOTRACE_DEBUG("Got already init -> `fork`");    // DEBUGGING
    }*/
}

/**
 * Finalizes, i.e., "un"initializes module; should be called prior termination for cleaning up  (by `cleanup` in event.c)
 */
void fnres_fin(void) {
    fnmap_destroy();

    got_init = false;
    // LIBIOTRACE_DEBUG("Uninit done");    // DEBUGGING
}



void fnres_trace_fctevent(struct basic *fctevent) {
    if (!got_init) {
        LIBIOTRACE_ERROR("Got no init prior usage");
    }

    char* const extracted_fctname = fctevent->function_name;
    // printf("\n[pid = %d] CALLED W/ %s\n", getpid(), extracted_fctname);    // DEBUGGING
    SWITCH_FCTNAME(extracted_fctname) {

    /* --- Functions relevant for tracing + traceable --- */
        case CASE_OPEN_STD_FD:
        case CASE_OPEN_STD_FILE:
            SET_TRACED_FNAME_FOR_FCTEVENT(fctevent, FNAME_SPECIFIER_STD);
            RETURN_IF_FCTEVENT_FAILED(fctevent);
            ADD_OR_UPDATE_FNAME_IN_TRACE_USING_FCTEVENT_FILE_TYPE(fctevent, FNAME_SPECIFIER_STD);
            return;

        case CASE___OPEN:
        case CASE___OPEN64:
        case CASE___OPEN_2:
        case CASE___OPEN64_2:

        case CASE_OPEN:
        case CASE_OPEN64:
        case CASE_CREAT:
        case CASE_CREAT64:
        case CASE_FOPEN:
        case CASE_FOPEN64:
        case CASE_MKSTEMP:              /* Functions 'MKxxxx': Use template for filename */
        case CASE_MKSTEMPS:
        case CASE_MKOSTEMP:
        case CASE_MKOSTEMPS:
        case CASE_TMPFILE:              /* Functions 'TMPxxxx': Have unknown filename */
        case CASE_TMPFILE64:

        case CASE_OPENAT:

        case CASE_MPI_FILE_OPEN:
        {
            const char* const extracted_fname = get_file_name_from_fctevent_function_data(fctevent);

            SET_TRACED_FNAME_FOR_FCTEVENT(fctevent, extracted_fname);

            RETURN_IF_FCTEVENT_FAILED(fctevent);
            ADD_OR_UPDATE_FNAME_IN_TRACE_USING_FCTEVENT_FILE_TYPE(fctevent, extracted_fname);
            return;
        }


        case CASE_EPOLL_CREATE:
        case CASE_EPOLL_CREATE1:
        case CASE_EVENTFD:
        case CASE_INOTIFY_INIT:
        case CASE_INOTIFY_INIT1:
        case CASE_MEMFD_CREATE:
        case CASE_SOCKET:
            SET_TRACED_FNAME_FOR_FCTEVENT(fctevent, FNAME_SPECIFIER_PSEUDO);

            RETURN_IF_FCTEVENT_FAILED(fctevent);
            ADD_OR_UPDATE_FNAME_IN_TRACE_USING_FCTEVENT_FILE_TYPE(fctevent, FNAME_SPECIFIER_PSEUDO);
            return;

        case CASE_ACCEPT:
        case CASE_ACCEPT4:
        case CASE_PIPE:
        case CASE_PIPE2:
        case CASE_SOCKETPAIR:
            SET_TRACED_FNAME_FOR_FCTEVENT(fctevent, FNAME_SPECIFIER_PSEUDO);

            RETURN_IF_FCTEVENT_FAILED(fctevent);
            ADD_OR_UPDATE_FNAME_IN_TRACE_USING_FCTEVENT_FUNCTION_DATA(fctevent, FNAME_SPECIFIER_PSEUDO);
            return;


        case CASE_FREOPEN:
        case CASE_FREOPEN64:
        {
            const char* const extracted_fname = get_file_name_from_fctevent_function_data(fctevent);

            if (NULL == extracted_fname) {     /* Note: filename == NULL means >> reopen SAME file again << */
                fnmap_key_t search_key; char* search_found_fname;
                IF_FOUND_FNAME_IN_TRACE_USING_FCTEVENT_FILE_TYPE(fctevent,
                                                                 search_key, search_found_fname) {
                    SET_TRACED_FNAME_FOR_FCTEVENT(fctevent, search_found_fname);
                } ELSE_SET_FOR_TRACED_FNAME_NOT_FOUND(fctevent)
                return;
            }

            SET_TRACED_FNAME_FOR_FCTEVENT(fctevent, extracted_fname);
            RETURN_IF_FCTEVENT_FAILED(fctevent);
            RMV_FNAME_FROM_TRACE_USING_FCTEVENT_FILE_TYPE(fctevent);
            ADD_OR_UPDATE_FNAME_IN_TRACE_USING_FCTEVENT_FILE_TYPE(fctevent, extracted_fname);  /* Add under same key but w/ different filename */
            return;
        }

        case CASE_MREMAP:
        {
            fnmap_key_t search_key; char* search_found_fname;
            IF_FOUND_FNAME_IN_TRACE_USING_FCTEVENT_FILE_TYPE(fctevent,
                                                             search_key, search_found_fname) {
                SET_TRACED_FNAME_FOR_FCTEVENT(fctevent, search_found_fname);

                RETURN_IF_FCTEVENT_FAILED(fctevent);
                ADD_OR_UPDATE_FNAME_IN_TRACE_USING_FCTEVENT_FUNCTION_DATA(fctevent, search_found_fname);
                RMV_FNAME_FROM_TRACE_USING_FCTEVENT_FILE_TYPE(fctevent);                       /* Remove old mapping */
            } ELSE_SET_FOR_TRACED_FNAME_NOT_FOUND(fctevent)
            return;
        }


        case CASE_MMAP:
        case CASE_MMAP64:
        {
            if (((struct memory_map_function*)(fctevent->__function_data))->map_flags.anonymous) {      /* Not file backed (`mmap` will ignore its arg `fd`) */
                SET_TRACED_FNAME_FOR_FCTEVENT(fctevent, FNAME_SPECIFIER_MEMMAP);

                RETURN_IF_FCTEVENT_FAILED(fctevent);
                ADD_OR_UPDATE_FNAME_IN_TRACE_USING_FCTEVENT_FUNCTION_DATA(fctevent, FNAME_SPECIFIER_MEMMAP);
            } else {
                goto case_dup;
            }
            return;
        }

        case CASE_DUP:
        case CASE_DUP2:
        case CASE_DUP3:
        case_dup:

        case CASE_FILENO:       /* Stream -> Fildes */

        case CASE_MPI_FILE_IREAD:
        case CASE_MPI_FILE_IREAD_ALL:
        case CASE_MPI_FILE_IREAD_AT:
        case CASE_MPI_FILE_IREAD_AT_ALL:
        case CASE_MPI_FILE_IWRITE:
        case CASE_MPI_FILE_IWRITE_ALL:
        case CASE_MPI_FILE_IWRITE_AT:
        case CASE_MPI_FILE_IWRITE_AT_ALL:
        {
            fnmap_key_t search_key; char *search_found_fname;
            IF_FOUND_FNAME_IN_TRACE_USING_FCTEVENT_FILE_TYPE(fctevent,
                                                             search_key, search_found_fname) {
                SET_TRACED_FNAME_FOR_FCTEVENT(fctevent, search_found_fname);

                RETURN_IF_FCTEVENT_FAILED(fctevent);
                ADD_OR_UPDATE_FNAME_IN_TRACE_USING_FCTEVENT_FUNCTION_DATA(fctevent, search_found_fname);
            } ELSE_SET_FOR_TRACED_FNAME_NOT_FOUND(fctevent)
            return;
        }

        case CASE_FCNTL:
            if (__void_p_enum_cmd_data_dup_function ==
                ((struct fcntl_function*)fctevent->__function_data)->__void_p_enum_cmd_data) { goto case_dup; }
            goto case_fcntl_no_dup;

        case CASE_FDOPEN:       /* Fildes -> Stream */
        {
            fnmap_key_t search_key; char *search_found_fname;
            IF_FOUND_FNAME_IN_TRACE_USING_FCTEVENT_FUNCTION_DATA(fctevent,
                                                                 search_key, search_found_fname) {
                SET_TRACED_FNAME_FOR_FCTEVENT(fctevent, search_found_fname);

                RETURN_IF_FCTEVENT_FAILED(fctevent);
                ADD_OR_UPDATE_FNAME_IN_TRACE_USING_FCTEVENT_FILE_TYPE(fctevent, search_found_fname);
            } ELSE_SET_FOR_TRACED_FNAME_NOT_FOUND(fctevent)
            return;
        }


        case CASE_SENDMSG:
        case CASE_RECVMSG:
            /* ?? Note: Fildes can be renamed when they're already open fildes in receiver w/ same int ?? */
            goto not_implemented_yet;


        case CASE_CLOSE:
        case CASE_FCLOSE:
        case CASE_MUNMAP:

        case CASE_MPI_WAIT:
        case CASE_MPI_REQUEST_FREE:

        case CASE_MPI_FILE_CLOSE:
        {
            fnmap_key_t search_key; char *search_found_fname;
            IF_FOUND_FNAME_IN_TRACE_USING_FCTEVENT_FILE_TYPE(fctevent,
                                                             search_key, search_found_fname) {
                SET_TRACED_FNAME_FOR_FCTEVENT(fctevent, search_found_fname);

                RETURN_IF_FCTEVENT_FAILED(fctevent);
                RMV_FNAME_FROM_TRACE_USING_FCTEVENT_FILE_TYPE(fctevent);
            } ELSE_SET_FOR_TRACED_FNAME_NOT_FOUND(fctevent)
            return;
        }

        case CASE_MPI_WAITALL:              /* ... TODO: Removes only requests from fnmap (to prevent leak), but doesn't set traced_filename(s) ...  */
        {
            const struct mpi_waitall* fctevent_function_data = (struct mpi_waitall*)fctevent->__function_data;

            if (NULL == fctevent_function_data->__requests) { return; }
            for (size_t i = 0; i < fctevent_function_data->__size_requests; i++) {
                int* req_id = &((*((fctevent_function_data->__requests) + i))->request_id);

                fnmap_key_t delete_key;
                create_fnmap_key_using_vals(R_MPI, req_id, 0, &delete_key);
                fnmap_remove(&delete_key);
            }

            goto not_implemented_yet;
        }


        case CASE_MADVISE:
        case CASE_POSIX_MADVISE:
            goto not_implemented_yet;


        case CASE_DIRFD:
            goto not_implemented_yet;   /* TODO: Ask no wrapper for `opendir` (`dirent.h`) ? */


        case CASE_FCLOSEALL:            /* GNU extension */
            goto not_implemented_yet;



    /* --- Traceable --- */
        case CASE_CLEANUP:          /* Internal libiotrace functions (which are written to trace) */
        case CASE_INIT_ON_LOAD:

        case CASE_PTHREAD_CREATE:
        case CASE_FORK:             /* Handled by hook `reset_on_fork` in event.c, which is automatically called on `fork` */
        case CASE_VFORK:
            SET_TRACED_FNAME_FOR_FCTEVENT(fctevent, FNAME_SPECIFIER_NAF);
            return;

        case CASE_EXECL:            /* TODO: ASK -> Old (but still inherited) fildes will be gone */
        case CASE_EXECLP:
        case CASE_EXECLE:
        case CASE_EXECV:
        case CASE_EXECVP:
        case CASE_EXECVPE:
            LIBIOTRACE_WARN("exec detected: Internal mappings pertinent for filename tracing will be overwritten (which may affect filename tracing)");
            SET_TRACED_FNAME_FOR_FCTEVENT(fctevent, FNAME_SPECIFIER_NAF);
            return;


        case CASE_MPI_FILE_DELETE:
            SET_TRACED_FNAME_FOR_FCTEVENT(fctevent, get_file_name_from_fctevent_function_data(fctevent));
            return;


        case_fcntl_no_dup:
        case CASE_READ:
        case CASE_WRITE:
        case CASE_READV:
        case CASE_WRITEV:
        case CASE_PREAD:
        case CASE_PREAD64:
        case CASE_PWRITE:
        case CASE_PWRITE64:
        case CASE_PREADV:
        case CASE_PREADV64:
        case CASE_PWRITEV:
        case CASE_PWRITEV64:
        case CASE_PREADV2:
        case CASE_PREADV64V2:
        case CASE_PWRITEV2:
        case CASE_PWRITEV64V2:
        case CASE_LSEEK:
        case CASE_LSEEK64:
        case CASE_SYNCFS:
        case CASE_FSYNC:
        case CASE_MSYNC:
        case CASE_FDATASYNC:
        case CASE_FLOCKFILE:
        case CASE_FTRYLOCKFILE:
        case CASE_FUNLOCKFILE:
        case CASE_FWIDE:
        case CASE_FPUTC:
        case CASE_FPUTC_UNLOCKED:
        case CASE_PUTW:
        case CASE_FPUTWC:
        case CASE_FPUTWC_UNLOCKED:
        case CASE_FPUTS:
        case CASE_FPUTS_UNLOCKED:
        case CASE_FPUTWS:
        case CASE_FPUTWS_UNLOCKED:
        case CASE_GETC:
        case CASE_GETC_UNLOCKED:
        case CASE_FGETC:
        case CASE_FGETC_UNLOCKED:
        case CASE_GETW:
        case CASE_FGETWC:
        case CASE_FGETWC_UNLOCKED:
        case CASE_FGETS:
        case CASE_FGETS_UNLOCKED:
        case CASE_FGETWS:
        case CASE_FGETWS_UNLOCKED:
        case CASE_UNGETC:
        case CASE_UNGETWC:
        case CASE_GETLINE:
        case CASE_GETDELIM:
        case CASE_FREAD:
        case CASE_FREAD_UNLOCKED:
        case CASE_FWRITE:
        case CASE_FWRITE_UNLOCKED:
        case CASE_FPRINTF:
        case CASE_FWPRINTF:
        case CASE_VFPRINTF:
        case CASE_VFWPRINTF:
        case CASE_FSCANF:
        case CASE_FWSCANF:
        case CASE_VFSCANF:
        case CASE_VFWSCANF:
        case CASE_FEOF:
        case CASE_FEOF_UNLOCKED:
        case CASE_FERROR:
        case CASE_FERROR_UNLOCKED:
        case CASE_CLEARERR:
        case CASE_CLEARERR_UNLOCKED:
        case CASE_FTELL:
        case CASE_FTELLO:
        case CASE_FTELLO64:
        case CASE_FSEEK:
        case CASE_FSEEKO:
        case CASE_FSEEKO64:
        case CASE_REWIND:
        case CASE_FGETPOS:
        case CASE_FGETPOS64:
        case CASE_FSETPOS:
        case CASE_FSETPOS64:
        case CASE_FFLUSH:
        case CASE_FFLUSH_UNLOCKED:
        case CASE_SETBUF:
        case CASE_SETVBUF:
        case CASE_SETBUFFER:
        case CASE_SETLINEBUF:

        case CASE_BIND:
        case CASE_CONNECT:

        case CASE___FREADABLE:          /* GNU extensions */
        case CASE___FWRITABLE:
        case CASE___FSETLOCKING:

        case CASE_MPI_FILE_READ:
        case CASE_MPI_FILE_READ_ALL:
        case CASE_MPI_FILE_READ_ALL_BEGIN:
        case CASE_MPI_FILE_READ_AT:
        case CASE_MPI_FILE_READ_AT_ALL:
        case CASE_MPI_FILE_WRITE:
        case CASE_MPI_FILE_WRITE_ALL:
        case CASE_MPI_FILE_WRITE_AT:
        case CASE_MPI_FILE_WRITE_AT_ALL:
        case CASE_MPI_FILE_SEEK:
        case CASE_MPI_FILE_SET_VIEW:
        {
            fnmap_key_t search_key; char* search_found_fname;
            IF_FOUND_FNAME_IN_TRACE_USING_FCTEVENT_FILE_TYPE(fctevent,
                                                             search_key, search_found_fname) {
                SET_TRACED_FNAME_FOR_FCTEVENT(fctevent, search_found_fname);
            } ELSE_SET_FOR_TRACED_FNAME_NOT_FOUND(fctevent)
            return;
        }


        case CASE_READDIR:
            goto not_implemented_yet;   /* TODO: No wrapper for `opendir` (`dirent.h`) ? */


        /* TODO: Functions affecting multiple files -> currently not feasible w/ char* trace_fname in `basic` struct */
        case CASE_COPY_FILE_RANGE:    /* Special case: This module will be called twice w/ same `basic` struct, BUT first w/ `copy_read_data` and finally w/ `copy_write_data` for `function_data` */
            goto not_implemented_yet;


        case CASE_SELECT:
            goto not_implemented_yet;


        case CASE_SYNC:
            goto not_implemented_yet;




    /* --- IO-types which are currently not / only partially implemented --- */
    /* - Dynamic linking loader - */
        case CASE_DLOPEN:
        case CASE_DLMOPEN: {
            const char* const extracted_fname = get_file_name_from_fctevent_function_data(fctevent);                  /* Note: `file_name` may be NULL (causes `dlopen` to return pointer to running program (i.e., itself)) */
            SET_TRACED_FNAME_FOR_FCTEVENT(fctevent, (extracted_fname) ? (extracted_fname) : ("MAIN PROGRAM"));
            return;
        }

    /* - Dynamically allocated mem. - */
        case CASE_MALLOC:
        case CASE_CALLOC:
        case CASE_REALLOC:
        case CASE_REALLOCARRAY:
        case CASE_FREE:

    /* - POSIX-AIO */
        case CASE_SHM_OPEN:
        case CASE_AIO_INIT:
        case CASE_AIO_CANCEL:
        case CASE_AIO_CANCEL64:
        case CASE_AIO_SUSPEND:
        case CASE_AIO_SUSPEND64:
        case CASE_AIO_FSYNC:
        case CASE_AIO_FSYNC64:
        case CASE_AIO_RETURN:
        case CASE_AIO_RETURN64:
        case CASE_AIO_ERROR:
        case CASE_AIO_ERROR64:
        case CASE_LIO_LISTIO:
        case CASE_LIO_LISTIO64:
        case CASE_AIO_WRITE:
        case CASE_AIO_WRITE64:
        case CASE_AIO_READ:
        case CASE_AIO_READ64:
            goto not_implemented_yet;



    /* ---------------------------------------------------------------------------------- */
        default:
            SET_TRACED_FNAME_FOR_FCTEVENT(fctevent, FNAME_SPECIFIER_UNHANDELED_FCT);
            LIBIOTRACE_DEBUG("Unhandled case for function `%s`", extracted_fctname);
            return;

        not_implemented_yet:
            SET_TRACED_FNAME_FOR_FCTEVENT(fctevent, FNAME_SPECIFIER_UNSUPPORTED_FCT);
            LIBIOTRACE_DEBUG("Not implemented yet function `%s`", extracted_fctname);
            return;
    }
}



/* - Helper functions - */
static void create_fnmap_key_using_vals(file_handle_type_t type, void* id, size_t mmap_length, fnmap_key_t *new_key) {
    memset(new_key, 0, FNMAP_KEY_SIZE);      /* Avoid garbage in key's id union */

    new_key->type = type;
    new_key->mmap_length = 0;
    switch(type) {
        case F_DESCRIPTOR:
            new_key->id.fildes = *((int*) id);
            return;

        case F_STREAM:
            new_key->id.stream = (FILE*) id;
            return;

        case F_DIR:
            new_key->id.dir = id;
            return;

        case F_MEMORY:
            new_key->id.mmap_start = id;
            new_key->mmap_length = mmap_length;
            return;

        case F_MPI:
            new_key->id.mpi_id = *((int*) id);
            return;

        case R_MPI:
            new_key->id.mpi_req_id = *((int*) id);
            return;

        default:
            LIBIOTRACE_WARN("Unhandled `id_type` w/ enum-value %u", type);
    }
}

static void create_fnmap_key_using_fctevent_file_type(struct basic *fctevent, fnmap_key_t *new_key) {
    switch(fctevent->__void_p_enum_file_type) {
        case __void_p_enum_file_type_file_descriptor:
            create_fnmap_key_using_vals(F_DESCRIPTOR,
                                        &((struct file_descriptor *) fctevent->__file_type)->descriptor, 0, new_key);
            return;

        case __void_p_enum_file_type_file_stream:
            create_fnmap_key_using_vals(F_STREAM,
                                        ((struct file_stream *) fctevent->__file_type)->stream, 0, new_key);
            return;

        case __void_p_enum_file_type_file_dir:
            create_fnmap_key_using_vals(F_DIR,
                                        ((struct file_dir *) fctevent->__file_type)->directory_stream, 0, new_key);
            return;

        case __void_p_enum_file_type_file_memory:
            create_fnmap_key_using_vals(F_MEMORY,
                                        ((struct file_memory *) fctevent->__file_type)->address,
                                        ((struct file_memory *) fctevent->__file_type)->length, new_key);
            return;

        case __void_p_enum_file_type_file_mpi:
            create_fnmap_key_using_vals(F_MPI,
                                        &((struct file_mpi *) fctevent->__file_type)->mpi_file, 0, new_key);
            return;

        case __void_p_enum_file_type_request_mpi:
            create_fnmap_key_using_vals(R_MPI,
                                        &((struct request_mpi *) fctevent->__file_type)->request_id, 0, new_key);
            return;

        case __void_p_enum_file_type_file_async:
        case __void_p_enum_file_type_shared_library:
        case __void_p_enum_file_type_file_alloc:
        	LIBIOTRACE_DEBUG("Unhandled case for `fctevent->__void_p_enum_file_type` w/ value %u", fctevent->__void_p_enum_file_type);
        	return;
        default:
            LIBIOTRACE_WARN("Unknown case for `fctevent->__void_p_enum_file_type` w/ value %u", fctevent->__void_p_enum_file_type);
            return;                      /* Note: Currently NOT checked by caller (-> proceeding w/o checking return value might lead to nonsensical fnmap-key; reasoning: indicates incomplete / faulty tracing, hence only warning)  */
    }
}

static void create_fnmap_key_using_fctevent_function_data(struct basic* fctevent, fnmap_key_t* new_key1, fnmap_key_t* new_key2) {
    switch (fctevent->__void_p_enum_function_data) {
        case __void_p_enum_function_data_dup_function:
            create_fnmap_key_using_vals(F_DESCRIPTOR,
                                        &((struct dup_function *) fctevent->__function_data)->new_descriptor, 0,
                                        new_key1);
            return;

        case __void_p_enum_function_data_dup3_function:
            create_fnmap_key_using_vals(F_DESCRIPTOR,
                                        &((struct dup3_function *) fctevent->__function_data)->new_descriptor, 0,
                                        new_key1);
            return;

        case __void_p_enum_function_data_fileno_function:
            create_fnmap_key_using_vals(F_DESCRIPTOR,
                                        &((struct fileno_function *) fctevent->__function_data)->file_descriptor, 0,
                                        new_key1);
            return;

        case __void_p_enum_function_data_fdopen_function:
            create_fnmap_key_using_vals(F_DESCRIPTOR,
                                        &((struct fdopen_function *) fctevent->__function_data)->descriptor, 0,
                                        new_key1);
            return;

        case __void_p_enum_function_data_accept_function:
            create_fnmap_key_using_vals(F_DESCRIPTOR,
                                        &((struct accept_function *) fctevent->__function_data)->new_descriptor, 0,
                                        new_key1);
            return;

        case __void_p_enum_function_data_file_pair:               /* Note: Creates 2 keys (2 fildes) */
            create_fnmap_key_using_vals(F_DESCRIPTOR, &((struct file_pair *) fctevent->__function_data)->descriptor1, 0,
                                        new_key1);
            create_fnmap_key_using_vals(F_DESCRIPTOR, &((struct file_pair *) fctevent->__function_data)->descriptor2, 0,
                                        new_key2);
            return;

        case __void_p_enum_function_data_socketpair_function:     /* Note: Creates 2 keys (2 fildes) */
            create_fnmap_key_using_vals(F_DESCRIPTOR,
                                        &((struct socketpair_function *) fctevent->__function_data)->descriptor1, 0,
                                        new_key1);
            create_fnmap_key_using_vals(F_DESCRIPTOR,
                                        &((struct socketpair_function *) fctevent->__function_data)->descriptor2, 0,
                                        new_key2);
            return;

        case __void_p_enum_function_data_memory_map_function:
            create_fnmap_key_using_vals(F_MEMORY, ((struct memory_map_function *) fctevent->__function_data)->address,
                                        ((struct memory_map_function *) fctevent->__function_data)->length, new_key1);
            return;

        case __void_p_enum_function_data_memory_remap_function:
            create_fnmap_key_using_vals(F_MEMORY,
                                        ((struct memory_remap_function *) fctevent->__function_data)->new_address,
                                        ((struct memory_remap_function *) fctevent->__function_data)->new_length,
                                        new_key1);
            return;

        case __void_p_enum_function_data_mpi_immediate:
            create_fnmap_key_using_vals(R_MPI, &((struct mpi_immediate *) fctevent->__function_data)->request_id, 0,
                                        new_key1);
            return;

        case __void_p_enum_function_data_mpi_immediate_at:
            create_fnmap_key_using_vals(R_MPI, &((struct mpi_immediate_at *) fctevent->__function_data)->request_id, 0,
                                        new_key1);
            return;

        case __void_p_enum_function_data_mpi_open_function:
            create_fnmap_key_using_vals(F_MPI, &((struct file_mpi *) fctevent->__function_data)->mpi_file, 0, new_key1);
            return;

        case __void_p_enum_function_data_copy_write_function:
            create_fnmap_key_using_vals(F_DESCRIPTOR,
                                        &((struct copy_write_function *) fctevent->__function_data)->from_file_descriptor,
                                        0, new_key1);
            return;

        case __void_p_enum_function_data_copy_read_function:
            create_fnmap_key_using_vals(F_DESCRIPTOR,
                                        &((struct copy_read_function *) fctevent->__function_data)->to_file_descriptor,
                                        0, new_key1);
            return;

        case __void_p_enum_function_data_fork_function:
        case __void_p_enum_function_data_open_function:
        case __void_p_enum_function_data_openat_function:
        case __void_p_enum_function_data_information_function:
        case __void_p_enum_function_data_lock_mode_function:
        case __void_p_enum_function_data_orientation_mode_function:
        case __void_p_enum_function_data_write_function:
        case __void_p_enum_function_data_pwrite_function:
        case __void_p_enum_function_data_pwrite2_function:
        case __void_p_enum_function_data_read_function:
        case __void_p_enum_function_data_pread_function:
        case __void_p_enum_function_data_pread2_function:
        case __void_p_enum_function_data_unget_function:
        case __void_p_enum_function_data_position_function:
        case __void_p_enum_function_data_positioning_function:
        case __void_p_enum_function_data_lpositioning_function:
        case __void_p_enum_function_data_buffer_function:
        case __void_p_enum_function_data_bufsize_function:
        case __void_p_enum_function_data_memory_sync_function:
        case __void_p_enum_function_data_memory_madvise_function:
        case __void_p_enum_function_data_select_function:
        case __void_p_enum_function_data_memory_posix_madvise_function:
        case __void_p_enum_function_data_asynchronous_read_function:
        case __void_p_enum_function_data_asynchronous_write_function:
        case __void_p_enum_function_data_asynchronous_listio_function:
        case __void_p_enum_function_data_asynchronous_error_function:
        case __void_p_enum_function_data_asynchronous_return_function:
        case __void_p_enum_function_data_asynchronous_sync_function:
        case __void_p_enum_function_data_asynchronous_suspend_function:
        case __void_p_enum_function_data_asynchronous_cancel_function:
        case __void_p_enum_function_data_asynchronous_init_function:
        case __void_p_enum_function_data_dlopen_function:
        case __void_p_enum_function_data_fcntl_function:
        case __void_p_enum_function_data_readdir_function:
        case __void_p_enum_function_data_dirfd_function:
        case __void_p_enum_function_data_msg_function:
        case __void_p_enum_function_data_mmsg_function:
        case __void_p_enum_function_data_sockaddr_function:
        case __void_p_enum_function_data_socket_function:
        case __void_p_enum_function_data_mpi_wait:
        case __void_p_enum_function_data_mpi_delete_function:
        case __void_p_enum_function_data_mpi_waitall:
        case __void_p_enum_function_data_alloc_function:
#if defined(HAVE_DLMOPEN) && defined(WITH_DL_IO)
        case __void_p_enum_function_data_dlmopen_function:
#endif
        	LIBIOTRACE_DEBUG("Unhandled case for `fctevent->__void_p_enum_function_data` w/ value %u", fctevent->__void_p_enum_function_data);
        	return;
        default:
            LIBIOTRACE_WARN("Unknown case for `fctevent->__void_p_enum_function_data` w/ value %u", fctevent->__void_p_enum_function_data);
            return;
    }
}

static const char* get_file_name_from_fctevent_function_data(struct basic* fctevent) {
    switch(fctevent->__void_p_enum_function_data) {
        case __void_p_enum_function_data_open_function:
            return ((struct open_function*)fctevent->__function_data)->file_name;

        case __void_p_enum_function_data_openat_function:
            return ((struct openat_function*)fctevent->__function_data)->file_name;

        case __void_p_enum_function_data_mpi_open_function:
            return ((struct mpi_open_function*)fctevent->__function_data)->file_name;

        case __void_p_enum_function_data_mpi_delete_function:
            return ((struct mpi_delete_function*)fctevent->__function_data)->file_name;

        case __void_p_enum_function_data_dlopen_function:
            return ((struct dlopen_function*)fctevent->__function_data)->file_name;

#if defined(HAVE_DLMOPEN) && defined(WITH_DL_IO)
        case __void_p_enum_function_data_dlmopen_function:
            return ((struct dlmopen_function*)fctevent->__function_data)->file_name;
#endif

        case __void_p_enum_function_data_fork_function:
        case __void_p_enum_function_data_fdopen_function:
		case __void_p_enum_function_data_information_function:
		case __void_p_enum_function_data_lock_mode_function:
		case __void_p_enum_function_data_orientation_mode_function:
		case __void_p_enum_function_data_write_function:
		case __void_p_enum_function_data_pwrite_function:
		case __void_p_enum_function_data_pwrite2_function:
		case __void_p_enum_function_data_read_function:
		case __void_p_enum_function_data_pread_function:
		case __void_p_enum_function_data_pread2_function:
		case __void_p_enum_function_data_copy_read_function:
		case __void_p_enum_function_data_copy_write_function:
		case __void_p_enum_function_data_unget_function:
		case __void_p_enum_function_data_position_function:
		case __void_p_enum_function_data_positioning_function:
		case __void_p_enum_function_data_lpositioning_function:
		case __void_p_enum_function_data_buffer_function:
		case __void_p_enum_function_data_fileno_function:
		case __void_p_enum_function_data_bufsize_function:
		case __void_p_enum_function_data_memory_map_function:
		case __void_p_enum_function_data_memory_sync_function:
		case __void_p_enum_function_data_memory_remap_function:
		case __void_p_enum_function_data_memory_madvise_function:
		case __void_p_enum_function_data_select_function:
		case __void_p_enum_function_data_memory_posix_madvise_function:
		case __void_p_enum_function_data_asynchronous_read_function:
		case __void_p_enum_function_data_asynchronous_write_function:
		case __void_p_enum_function_data_asynchronous_listio_function:
		case __void_p_enum_function_data_asynchronous_error_function:
		case __void_p_enum_function_data_asynchronous_return_function:
		case __void_p_enum_function_data_asynchronous_sync_function:
		case __void_p_enum_function_data_asynchronous_suspend_function:
		case __void_p_enum_function_data_asynchronous_cancel_function:
		case __void_p_enum_function_data_asynchronous_init_function:
		case __void_p_enum_function_data_dup_function:
		case __void_p_enum_function_data_dup3_function:
		case __void_p_enum_function_data_fcntl_function:
		case __void_p_enum_function_data_file_pair:
		case __void_p_enum_function_data_readdir_function:
		case __void_p_enum_function_data_dirfd_function:
		case __void_p_enum_function_data_msg_function:
		case __void_p_enum_function_data_mmsg_function:
		case __void_p_enum_function_data_sockaddr_function:
		case __void_p_enum_function_data_accept_function:
		case __void_p_enum_function_data_socketpair_function:
		case __void_p_enum_function_data_socket_function:
		case __void_p_enum_function_data_mpi_immediate:
		case __void_p_enum_function_data_mpi_wait:
		case __void_p_enum_function_data_mpi_immediate_at:
		case __void_p_enum_function_data_mpi_waitall:
		case __void_p_enum_function_data_alloc_function:
			LIBIOTRACE_DEBUG("Unhandled case for `fctevent->__void_p_enum_function_data` w/ value %u", fctevent->__void_p_enum_function_data);
			return NULL;
        default:
            LIBIOTRACE_WARN("Unknown case for `fctevent->__void_p_enum_function_data` w/ value %u", fctevent->__void_p_enum_function_data);
            return NULL;                      /* Note: Currently NOT checked by all callers (ISSUE: Has sometimes special meaning, e.g., for `dlopen`; Note: Proceeding w/o checking return value might lead to SIGSEGV)  */
    }
}
