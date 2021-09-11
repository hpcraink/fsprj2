#include "fctevent.h"
#include "internal/fctconsts.h"
#include "internal/map/fmap.h"
#include "internal/logging.h"

#include "../libiotrace_include_struct.h"

#include <string.h>
#include <stdbool.h>        /* Be careful: Insertion order might cause issues w/ "../libiotrace_include_struct.h" */


// !!!! TODO: Add Wrapper for 'MPI_Request_free' (https://www.mpich.org/static/docs/v3.3/www3/MPI_Request_free.html) in libiotrace !!!!
/**
 * TODOS:
 *  - 'fmap_destroy' currently LEAKS MEMORY since '__del_hook' isn't executed for each item in map (not a very serious issue though since the function will only be called once the observed program exits, i.e., the OS will cleanup)
 *  - Remove memory-mappings after fork
 *    - Save mapping type (public/private) -> save struct consisting of filename + whether public in map
 *    - Change 'CASE_MMAP' to save this additional information (see struct mentioned line above)
 *    - Implement CASE_MADVISE
 *    - Implement callable hook 'reset_on_fork' (which sets flag 'got_forked' to indicate in next wrapper call a necessary removal of map values; called by 'reset_values_in_forked_process' in event.c)
 *  - Cleanup MPI Immediate MPI_Request handles (--> requires wrapper for 'MPI_Request_free')
 *     -> Concern (of current implementation): Max buckets of fmap may NOT be sufficient when some things aren't removed after close
 *  - Support multiple traced files in 'basic'-struct (some function-events affect multiple files) -> 'sync', 'floseall', 'copy_write_data' or 'MPI_Waitall'
 *      - Note regarding copy_write_data: Implemented using 2 calls to this module; Check enum during call whether read or write data + assemble it
 *  - Optimization: Currently, each 'fmap_set' results in an 'malloc'. HOWEVER, some functions (like 'dup') create an 'id' referring to an already open file (which has already a filename-entry in the map)
 *    => Use pointer to this already allocated filename
 */


/* --- Function prototypes for helper functions --- */
static int __create_fmap_key_using_vals(id_type type, void* id, size_t mmap_length, fmap_key* new_key);
static int __create_fmap_key_using_fctevent_file_type(struct basic* fctevent, fmap_key* new_key);
static int __create_fmap_key_using_fctevent_function_data(struct basic* fctevent, fmap_key* new_key1, fmap_key* new_key2);

static const char* __get_file_name_from_fctevent_function_data(struct basic* fctevent);


/* --- Macros --- */
#define RETURN_IF_FCTEVENT_FAILED(fctevent) if (error == (fctevent)->return_state) { return; }

#define ADD_FNAME_TO_TRACE_USING_FCTEVENT_FILE_TYPE(fctevent, filename) {\
  fmap_key insert_key;\
  __create_fmap_key_using_fctevent_file_type(fctevent, &insert_key);\
  fmap_set(&insert_key, filename);\
}
#define ADD_FNAME_TO_TRACE_USING_FCTEVENT_FUNCTION_DATA(fctevent, filename) {\
  fmap_key insert_key1; fmap_key insert_key2;\
  __create_fmap_key_using_fctevent_function_data(fctevent, &insert_key1, &insert_key2);\
  fmap_set(&insert_key1, filename);\
  \
  if (void_p_enum_function_data_file_pair == (fctevent)->void_p_enum_function_data || \
    void_p_enum_function_data_socketpair_function == (fctevent)->void_p_enum_function_data) {\
    fmap_set(&insert_key2, filename);\
  }\
}

#define RMV_FNAME_FROM_TRACE_USING_FCTEVENT_FILE_TYPE(fctevent) {\
  fmap_key delete_key;\
  __create_fmap_key_using_fctevent_file_type(fctevent, &delete_key);\
  fmap_remove(&delete_key);\
}

#define IF_FOUND_FNAME_IN_TRACE_USING_FCTEVENT_FILE_TYPE(fctevent, search_key, search_found_fname)\
  __create_fmap_key_using_fctevent_file_type((fctevent), &(search_key));\
  if (!fmap_get(&(search_key), &(search_found_fname)))
#define IF_FOUND_FNAME_IN_TRACE_USING_FCTEVENT_FUNCTION_DATA(fctevent, search_key, search_found_fname)\
  __create_fmap_key_using_fctevent_function_data((fctevent), &(search_key), NULL);\
  if (!fmap_get(&(search_key), &(search_found_fname)))

#define SET_TRACED_FNAME_FOR_FCTEVENT(fctevent, traced_fname) strncpy((fctevent)->traced_filename, traced_fname, sizeof((fctevent)->traced_filename));

/* --- Globals --- */
bool got_init = false;


/* --- Public functions --- */
/* - 'Hooks' - */
/**
 * Initializes module; MUST be called prior usage  (by 'init_process' in event.c)
 */
void fnres_init(size_t fmap_max_size) {
    fmap_create(fmap_max_size);

    got_init = true;
    LOG_DEBUG("Init done ['fmap_max_size = %zu']", fmap_max_size)
}

/**
 * Finalizes, i.e., "un"initializes module; should be called prior termination for cleaning up  (by 'cleanup' in event.c)
 */
void fnres_fin(void) {
    fmap_destroy();

    got_init = false;
    LOG_DEBUG("Uninit done")
}



void fnres_trace_fctevent(struct basic *fctevent) {
    if (!got_init) { LOG_ERROR_AND_EXIT("fnres: Got no init prior usage") }

    char* const extracted_fctname = fctevent->function_name;
    printf("\n\nCALLED W/ %s\n", extracted_fctname);        // DEBUGGING
    SWITCH_FCTNAME(extracted_fctname) {

        /* --- Functions relevant for tracing + traceable --- */
        case CASE_OPEN_STD_FD:
            printf("\tFD-ID = %d\n", ((struct file_descriptor*)fctevent->file_type)->descriptor);        // DEBUGGING
            goto continue_debug;

        case CASE_OPEN_STD_FILE:
            printf("\tSTREAM = %p\n", ((struct file_stream*)fctevent->file_type)->stream);        // DEBUGGING
        continue_debug:
            SET_TRACED_FNAME_FOR_FCTEVENT(fctevent, FNAME_SPECIFIER_STD)
            puts("\t0");          // DEBUGGING

            RETURN_IF_FCTEVENT_FAILED(fctevent)
            puts("\t1");          // DEBUGGING
            ADD_FNAME_TO_TRACE_USING_FCTEVENT_FILE_TYPE(fctevent, FNAME_SPECIFIER_STD)
            puts("\t2");          // DEBUGGING
            return;

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
            const char* const extracted_fname = __get_file_name_from_fctevent_function_data(fctevent);

            SET_TRACED_FNAME_FOR_FCTEVENT(fctevent, extracted_fname)

            RETURN_IF_FCTEVENT_FAILED(fctevent)
            ADD_FNAME_TO_TRACE_USING_FCTEVENT_FILE_TYPE(fctevent, extracted_fname)

            printf("\n\nAfter: %s\n\n", fctevent->traced_filename);         // DEBUGGING
            return;
        }

        case CASE_ACCEPT:
        case CASE_ACCEPT4:
        case CASE_EPOLL_CREATE:
        case CASE_EPOLL_CREATE1:
        case CASE_EVENTFD:
        case CASE_INOTIFY_INIT:
        case CASE_INOTIFY_INIT1:
        case CASE_MEMFD_CREATE:
        case CASE_SOCKET:
            SET_TRACED_FNAME_FOR_FCTEVENT(fctevent, FNAME_SPECIFIER_PSEUDO)

            RETURN_IF_FCTEVENT_FAILED(fctevent)
            ADD_FNAME_TO_TRACE_USING_FCTEVENT_FILE_TYPE(fctevent, FNAME_SPECIFIER_PSEUDO)
            return;

        case CASE_PIPE:
        case CASE_PIPE2:
        case CASE_SOCKETPAIR:
            SET_TRACED_FNAME_FOR_FCTEVENT(fctevent, FNAME_SPECIFIER_PSEUDO)

            RETURN_IF_FCTEVENT_FAILED(fctevent)
            ADD_FNAME_TO_TRACE_USING_FCTEVENT_FUNCTION_DATA(fctevent, FNAME_SPECIFIER_PSEUDO)
            return;


        case CASE_FREOPEN:
        case CASE_FREOPEN64:
        {
            const char* const extracted_fname = __get_file_name_from_fctevent_function_data(fctevent);

            if (NULL == extracted_fname) {     /* Note: filename == NULL means 'reopen SAME file again' */
                fmap_key search_key; char* search_found_fname;
                IF_FOUND_FNAME_IN_TRACE_USING_FCTEVENT_FILE_TYPE(fctevent,
                                                                 search_key, search_found_fname) {
                    SET_TRACED_FNAME_FOR_FCTEVENT(fctevent, search_found_fname)
                } else { SET_TRACED_FNAME_FOR_FCTEVENT(fctevent, FNAME_SPECIFIER_NOTFOUND) }
                return;
            }

            SET_TRACED_FNAME_FOR_FCTEVENT(fctevent, extracted_fname)      // SEARCH FILENAME IN MAP ...
            RETURN_IF_FCTEVENT_FAILED(fctevent)
            RMV_FNAME_FROM_TRACE_USING_FCTEVENT_FILE_TYPE(fctevent)
            ADD_FNAME_TO_TRACE_USING_FCTEVENT_FILE_TYPE(fctevent, extracted_fname)          /* Add under same key but w/ different filename */
            return;
        }

        case CASE_MREMAP:
        {
            fmap_key search_key; char* search_found_fname;
            IF_FOUND_FNAME_IN_TRACE_USING_FCTEVENT_FILE_TYPE(fctevent,
                                                             search_key, search_found_fname) {
                SET_TRACED_FNAME_FOR_FCTEVENT(fctevent, search_found_fname)

                RETURN_IF_FCTEVENT_FAILED(fctevent)
                ADD_FNAME_TO_TRACE_USING_FCTEVENT_FUNCTION_DATA(fctevent, search_found_fname)
                RMV_FNAME_FROM_TRACE_USING_FCTEVENT_FILE_TYPE(fctevent)       /* Remove old mapping */
            } else { SET_TRACED_FNAME_FOR_FCTEVENT(fctevent, FNAME_SPECIFIER_NOTFOUND) }
            return;
        }


        case CASE_MMAP:
        case CASE_MMAP64:
        {
            if (((struct memory_map_flags)((struct memory_map_function*)fctevent->function_data)->map_flags).anonymous) {      /* Not file backed ('mmap' will ignore its arg 'fd') */
                SET_TRACED_FNAME_FOR_FCTEVENT(fctevent, FNAME_SPECIFIER_MEMMAP)

                RETURN_IF_FCTEVENT_FAILED(fctevent)
                ADD_FNAME_TO_TRACE_USING_FCTEVENT_FILE_TYPE(fctevent, FNAME_SPECIFIER_MEMMAP)
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
            fmap_key search_key; char *search_found_fname;
            IF_FOUND_FNAME_IN_TRACE_USING_FCTEVENT_FILE_TYPE(fctevent,
                                                             search_key, search_found_fname) {
                SET_TRACED_FNAME_FOR_FCTEVENT(fctevent, search_found_fname)

                RETURN_IF_FCTEVENT_FAILED(fctevent)
                ADD_FNAME_TO_TRACE_USING_FCTEVENT_FUNCTION_DATA(fctevent, search_found_fname)
            } else { SET_TRACED_FNAME_FOR_FCTEVENT(fctevent, FNAME_SPECIFIER_NOTFOUND) }
            return;
        }

        case CASE_FCNTL:
            if (void_p_enum_cmd_data_dup_function ==
                ((struct fcntl_function*)fctevent->function_data)->void_p_enum_cmd_data) { goto case_dup; }
            return;

        case CASE_FDOPEN:       /* Fildes -> Stream */
        {
            fmap_key search_key; char *search_found_fname;
            IF_FOUND_FNAME_IN_TRACE_USING_FCTEVENT_FUNCTION_DATA(fctevent,
                                                                 search_key, search_found_fname) {
                SET_TRACED_FNAME_FOR_FCTEVENT(fctevent, search_found_fname)

                RETURN_IF_FCTEVENT_FAILED(fctevent)
                ADD_FNAME_TO_TRACE_USING_FCTEVENT_FILE_TYPE(fctevent, search_found_fname)
            } else { SET_TRACED_FNAME_FOR_FCTEVENT(fctevent, FNAME_SPECIFIER_NOTFOUND) }
            return;
        }


        case CASE_SENDMSG:
        case CASE_RECVMSG:
            /* ?? Note: Fildes can be renamed when they're already open fildes in receiver w/ same int ?? */
            goto not_implemented_yet;


        case CASE_CLOSE:
        case CASE_FCLOSE:
        case CASE_MUNMAP:

        case CASE_MPI_REQUEST_FREE:
        case CASE_MPI_FILE_CLOSE: {
            fmap_key search_key; char *search_found_fname;
            IF_FOUND_FNAME_IN_TRACE_USING_FCTEVENT_FILE_TYPE(fctevent,
                                                             search_key, search_found_fname) {
                SET_TRACED_FNAME_FOR_FCTEVENT(fctevent, search_found_fname)

                RETURN_IF_FCTEVENT_FAILED(fctevent)
                RMV_FNAME_FROM_TRACE_USING_FCTEVENT_FILE_TYPE(fctevent)
            } else { SET_TRACED_FNAME_FOR_FCTEVENT(fctevent, FNAME_SPECIFIER_NOTFOUND) }
            return;
        }



        case CASE_MADVISE:
        case CASE_POSIX_MADVISE:
            goto not_implemented_yet;


        case CASE_FCLOSEALL:            /* GNU extension */
            goto not_implemented_yet;


        case CASE_FORK:         /* Handled by hook 'reset_on_fork', which is automatically called on 'fork' */
        case CASE_VFORK:
            return;



        /* --- Traceable --- */
        case CASE_CLEANUP:          /* Internal libiotrace functions (which will be written to trace) */
        case CASE_INIT_ON_LOAD:
            SET_TRACED_FNAME_FOR_FCTEVENT(fctevent, FNAME_SPECIFIER_INTERNAL)
            return;


        case CASE_MPI_FILE_DELETE:
            SET_TRACED_FNAME_FOR_FCTEVENT(fctevent, __get_file_name_from_fctevent_function_data(fctevent))
            return;


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

        case CASE_MPI_WAIT:
        {
            fmap_key search_key; char* search_found_fname;
            IF_FOUND_FNAME_IN_TRACE_USING_FCTEVENT_FILE_TYPE(fctevent,
                                                             search_key, search_found_fname) {
                SET_TRACED_FNAME_FOR_FCTEVENT(fctevent, search_found_fname)
            } else { SET_TRACED_FNAME_FOR_FCTEVENT(fctevent, FNAME_SPECIFIER_NOTFOUND) }
            return;
        }




    /* TODO: Functions affecting multiple files -> currently not feasible w/ char* trace_fname in 'basic' struct */
        case CASE_COPY_FILE_RANGE:    /* Special case: This module will be called twice w/ same 'basic' struct, BUT first w/ 'copy_read_data' and finally w/ 'copy_write_data' for 'function_data' */
            goto not_implemented_yet;


        case CASE_MPI_WAITALL:

        case CASE_SYNC:
            goto not_implemented_yet;



        /* ---------------------------------------------------------------------------------- */
        default:
            SET_TRACED_FNAME_FOR_FCTEVENT(fctevent, FNAME_SPECIFIER_UNHANDELED_FCT)
            LOG_DEBUG("Unhandled case for function '%s'", extracted_fctname)
            return;

        not_implemented_yet:
            SET_TRACED_FNAME_FOR_FCTEVENT(fctevent, FNAME_SPECIFIER_UNSUPPORTED_FCT)
            LOG_DEBUG("Not implemented yet function '%s'", extracted_fctname)
            return;
    }
}



/* - Helper functions - */
static int __create_fmap_key_using_vals(id_type type, void* id, size_t mmap_length, fmap_key *new_key) {
    memset(new_key, 0, FMAP_KEY_SIZE);      /* Avoid garbage in key's id union */

    new_key->type = type;
    new_key->mmap_length = 0;
    switch(type) {
        case F_DESCRIPTOR:
            new_key->id.fildes = *((int*) id);
            return 0;

        case F_STREAM:
            new_key->id.stream = (FILE*) id;
            return 0;

        case F_MEMORY:
            new_key->id.mmap_start = id;
            new_key->mmap_length = mmap_length;
            return 0;

        case F_MPI:
            new_key->id.mpi_id = *((int*) id);
            return 0;

        case R_MPI:
            new_key->id.mpi_req_id = *((int*) id);
            return 0;

        default:
            return 1;
    }
}
static int __create_fmap_key_using_fctevent_file_type(struct basic *fctevent, fmap_key *new_key) {
    switch(fctevent->void_p_enum_file_type) {
        case void_p_enum_file_type_file_descriptor:
            return __create_fmap_key_using_vals(F_DESCRIPTOR,
                           &((struct file_descriptor *) fctevent->file_type)->descriptor, 0, new_key);

        case void_p_enum_file_type_file_stream:
            return __create_fmap_key_using_vals(F_STREAM,
                           &((struct file_stream *) fctevent->file_type)->stream, 0, new_key);

        case void_p_enum_file_type_file_memory:
            return __create_fmap_key_using_vals(F_MEMORY,
                           ((struct file_memory *) fctevent->file_type)->address,
                           ((struct file_memory *) fctevent->file_type)->length, new_key);

        case void_p_enum_file_type_file_mpi:
            return __create_fmap_key_using_vals(F_MPI,
                           &((struct file_mpi *) fctevent->file_type)->mpi_file, 0, new_key);

        case void_p_enum_file_type_request_mpi:
            return __create_fmap_key_using_vals(R_MPI,
                                                &((struct request_mpi *) fctevent->file_type)->request_id, 0, new_key);

        default:
            LOG_DEBUG("Unhandled case for 'fctevent->void_p_enum_file_type' w/ value %d", fctevent->void_p_enum_file_type)
            return 1;
    }
}
static int __create_fmap_key_using_fctevent_function_data(struct basic* fctevent, fmap_key* new_key1, fmap_key* new_key2) {
    switch (fctevent->void_p_enum_function_data) {
        case void_p_enum_function_data_dup_function:
            return __create_fmap_key_using_vals(F_DESCRIPTOR, &((struct dup_function*)fctevent->function_data)->new_descriptor, 0, new_key1);

        case void_p_enum_function_data_dup3_function:
            return __create_fmap_key_using_vals(F_DESCRIPTOR, &((struct dup3_function*)fctevent->function_data)->new_descriptor, 0, new_key1);

        case void_p_enum_function_data_fileno_function:
            return __create_fmap_key_using_vals(F_DESCRIPTOR, &((struct fileno_function*)fctevent->function_data)->file_descriptor, 0, new_key1);

        case void_p_enum_function_data_fdopen_function:
            return __create_fmap_key_using_vals(F_DESCRIPTOR, &((struct fdopen_function*)fctevent->function_data)->descriptor, 0, new_key1);

        case void_p_enum_function_data_file_pair:
            __create_fmap_key_using_vals(F_DESCRIPTOR, &((struct file_pair*)fctevent->function_data)->descriptor1, 0, new_key1);
            return __create_fmap_key_using_vals(F_DESCRIPTOR, &((struct file_pair*)fctevent->function_data)->descriptor2, 0, new_key2);

        case void_p_enum_function_data_socketpair_function:
            __create_fmap_key_using_vals(F_DESCRIPTOR, &((struct socketpair_function*)fctevent->function_data)->descriptor1, 0, new_key1);
            return __create_fmap_key_using_vals(F_DESCRIPTOR, &((struct socketpair_function*)fctevent->function_data)->descriptor2, 0, new_key2);

        case void_p_enum_function_data_memory_map_function:
            return __create_fmap_key_using_vals(F_MEMORY, ((struct memory_map_function*)fctevent->function_data)->address, ((struct memory_map_function*) fctevent->function_data)->length, new_key1);

        case void_p_enum_function_data_memory_remap_function:
            return __create_fmap_key_using_vals(F_MEMORY, ((struct memory_remap_function*)fctevent->function_data)->new_address, ((struct memory_remap_function*) fctevent->function_data)->new_length, new_key1);

        case void_p_enum_function_data_mpi_immediate:
            return __create_fmap_key_using_vals(R_MPI, &((struct mpi_immediate*)fctevent->function_data)->request_id, 0, new_key1);

        case void_p_enum_function_data_mpi_immediate_at:
            return __create_fmap_key_using_vals(R_MPI, &((struct mpi_immediate_at*)fctevent->function_data)->request_id, 0, new_key1);

        case void_p_enum_function_data_mpi_open_function:
            return __create_fmap_key_using_vals(F_MPI, &((struct file_mpi*)fctevent->function_data)->mpi_file, 0, new_key1);


        case void_p_enum_function_data_copy_write_function:
            return __create_fmap_key_using_vals(F_DESCRIPTOR, &((struct copy_write_function*)fctevent->function_data)->from_file_descriptor, 0, new_key1);

        case void_p_enum_function_data_copy_read_function:
            return __create_fmap_key_using_vals(F_DESCRIPTOR, &((struct copy_read_function*)fctevent->function_data)->to_file_descriptor, 0, new_key1);


        default:
            LOG_DEBUG("Unhandled case for 'fctevent->void_p_enum_function_data' w/ value %d", fctevent->void_p_enum_function_data)
            return 1;
    }
}

static const char* __get_file_name_from_fctevent_function_data(struct basic* fctevent) {
    switch(fctevent->void_p_enum_function_data) {
        case void_p_enum_function_data_open_function:
            return ((struct open_function*)fctevent->function_data)->file_name;

        case void_p_enum_function_data_openat_function:
            return ((struct openat_function*)fctevent->function_data)->file_name;

        case void_p_enum_function_data_mpi_open_function:
            return ((struct mpi_open_function*)fctevent->function_data)->file_name;

        case void_p_enum_function_data_mpi_delete_function:
            return ((struct mpi_delete_function*)fctevent->function_data)->file_name;

        default:
            LOG_DEBUG("Unhandled case for 'fctevent->void_p_enum_function_data' w/ value %d", fctevent->void_p_enum_function_data)
            return NULL;
    }
}