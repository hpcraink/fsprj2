#include <string.h>
#include <stdint.h>

#include "CUnit/CUnitCI.h"

#include "../../src/fnres/fctevent.h"
#include "../../src/libiotrace_include_struct.h"


/* -- Constants -- */
#define DEFAULT_FMAP_MAX_SIZE 100

#define STRINGS_ARE_EQUAL 0

/* -- Globals -- */
int64_t next_dummy_id = 3;     /* Avoid colliding ids */


/* -- Hooks -- */
/* run at the start of the suite */
CU_SUITE_SETUP() {
    fnres_init(DEFAULT_FMAP_MAX_SIZE);

	return CUE_SUCCESS;
}

/* run at the end of the suite */
CU_SUITE_TEARDOWN() {
    fnres_fin();            /* Note: As mentioned in 'fctevent.c', 'fnres_fin' doesn't clean up dyn. allocated filenames (i.e., leaks memory) */

	return CUE_SUCCESS;
}

/* run at the start of each test */
CU_TEST_SETUP() {}

/* run at the end of each test */
CU_TEST_TEARDOWN() {}


/* -- Tests -- */
/**
 * Tests whether ...
 *   - basic (open / close) fctevent's are added to / removed from trace
 *   - traceable(s) get the correct 'traced_filename' set
 */
void test_posix_fildes_open_read_close(void) {
    /* -- Create test data -- */
    struct file_descriptor tmp_af1_file_type = { .descriptor = (int)next_dummy_id++ };
    struct open_function tmp_af1_function_data = { .file_name = "/etc/resolv.conf" };
    struct basic af1_open = {
            .function_name = "open",
            .return_state = ok,
            .file_type = &tmp_af1_file_type, .void_p_enum_file_type = void_p_enum_file_type_file_descriptor,
            .function_data = &tmp_af1_function_data, .void_p_enum_function_data = void_p_enum_function_data_open_function
    };

    struct basic bf1_read = {
            .function_name = "read",
            .return_state = ok,
            .file_type = &tmp_af1_file_type, .void_p_enum_file_type = void_p_enum_file_type_file_descriptor
    };

    struct basic cf1_close = {
            .function_name = "close",
            .return_state = ok,
            .file_type = &tmp_af1_file_type, .void_p_enum_file_type = void_p_enum_file_type_file_descriptor
    };
    /* -- Create test data -- */


    /* -- (1) Add (to trace) + set filename for traceable: 'open' -> 'read' -- */
    fnres_trace_fctevent(&af1_open);

    fnres_trace_fctevent(&bf1_read);     /* 'traced_filename' should be now set to previous 'open' */
    CU_ASSERT_FATAL(STRINGS_ARE_EQUAL == strcmp(bf1_read.traced_filename, ((struct open_function*)af1_open.function_data)->file_name));


    /* -- (2) Remove (from trace): 'close' -> 'read' -- */
    fnres_trace_fctevent(&cf1_close);

    fnres_trace_fctevent(&bf1_read);
    CU_ASSERT_FATAL(STRINGS_ARE_EQUAL == strcmp(bf1_read.traced_filename, FNAME_SPECIFIER_NOTFOUND));
}



/**
 * Tests fctevent w/ 2 ids (e.g., 'pipe2')
 */
void test_posix_pipe2_lseek(void) {
    /* -- Create test data -- */
    const int tmp_af1_function_data_fildes1 = (int)next_dummy_id++; const int tmp_af1_function_data_fildes2 = (int)next_dummy_id++;
    struct file_pair tmp_af1_function_data = { .descriptor1 = tmp_af1_function_data_fildes1, .descriptor2 = tmp_af1_function_data_fildes2 };
    struct basic af1_pipe2 = {
            .function_name = "pipe2",
            .return_state = ok,
            .function_data = &tmp_af1_function_data, .void_p_enum_function_data = void_p_enum_function_data_file_pair
    };

    struct file_descriptor tmp_bf1_file_type = { .descriptor = tmp_af1_function_data_fildes1 };
    struct basic bf1_lseek = {
            .function_name = "lseek",
            .return_state = ok,
            .file_type = &tmp_bf1_file_type, .void_p_enum_file_type = void_p_enum_file_type_file_descriptor
    };

    struct file_descriptor tmp_bf2_file_type = { .descriptor = tmp_af1_function_data_fildes2 };
    struct basic bf2_pread = {
            .function_name = "pread",
            .return_state = ok,
            .file_type = &tmp_bf2_file_type, .void_p_enum_file_type = void_p_enum_file_type_file_descriptor
    };
    /* -- Create test data -- */


    /* -- (0) Add to trace: 'pipe2' -- */
    fnres_trace_fctevent(&af1_pipe2);

    /* -- (1) Set filename for traceable: 'lseek' + 'pread' -- */
    fnres_trace_fctevent(&bf1_lseek);
    fnres_trace_fctevent(&bf2_pread);

    CU_ASSERT_FATAL(STRINGS_ARE_EQUAL == strcmp(bf1_lseek.traced_filename, FNAME_SPECIFIER_PSEUDO));
    CU_ASSERT_FATAL(STRINGS_ARE_EQUAL == strcmp(bf1_lseek.traced_filename, bf2_pread.traced_filename));
}


/**
 * Tests duplicate fctevents (i.e., finding already existing filename in trace + adding it again; e.g. 'fileno')
 */
void test_posix_stream_fildes_fileno_fwrite(void) {
    /* -- Create test data -- */
    struct file_stream tmp_af1_file_type = { .stream = (FILE*)next_dummy_id++ };
    struct open_function tmp_af1_function_data = { .file_name = "/etc/fstab" };
    struct basic af1_fopen = {
            .function_name = "fopen",
            .return_state = ok,
            .function_data = &tmp_af1_function_data, .void_p_enum_function_data = void_p_enum_function_data_open_function,
            .file_type = &tmp_af1_file_type, .void_p_enum_file_type = void_p_enum_file_type_file_stream
    };

    struct fileno_function tmp_af2_function_data = { .file_descriptor = (int)next_dummy_id++ };
    struct basic af2_fileno = {
            .function_name = "fileno",
            .return_state = ok,
            .file_type = &tmp_af1_file_type, .void_p_enum_file_type = void_p_enum_file_type_file_stream,
            .function_data = &tmp_af2_function_data, .void_p_enum_function_data = void_p_enum_function_data_fileno_function
    };

    struct file_descriptor tmp_bf1_file_type = { .descriptor = tmp_af2_function_data.file_descriptor };
    struct basic bf1_write = {
            .function_name = "write",
            .return_state = ok,
            .file_type = &tmp_bf1_file_type, .void_p_enum_file_type = void_p_enum_file_type_file_descriptor
    };

    struct basic bf2_fgetpos = {
            .function_name = "fgetpos",
            .return_state = ok,
            .file_type = &tmp_af1_file_type, .void_p_enum_file_type = void_p_enum_file_type_file_stream
    };
    /* -- Create test data -- */


    /* -- (0) Add to trace: 'fopen' -> 'fileno' -- */
    fnres_trace_fctevent(&af1_fopen);
    fnres_trace_fctevent(&af2_fileno);

    /* -- (1) Set filename for traceable: 'write' + 'fgetpos' -- */
    fnres_trace_fctevent(&bf2_fgetpos);
    fnres_trace_fctevent(&bf1_write);

    CU_ASSERT_FATAL(STRINGS_ARE_EQUAL == strcmp(bf1_write.traced_filename, bf2_fgetpos.traced_filename));         /* 'write' / fgetpos': Pertains to previous 'fileno' / 'fopen' */
    CU_ASSERT_FATAL(STRINGS_ARE_EQUAL == strcmp(bf1_write.traced_filename, tmp_af1_function_data.file_name));
}


/**
 * Tests memory-mapping
 */
void test_posix_creat_mmap_msync_mremap_munmap(void) {
    /* -- Create test data -- */
    struct file_descriptor tmp_af1_file_type = { .descriptor = (int)next_dummy_id++ };
    struct open_function tmp_af1_function_data = { .file_name = "/var/tmp/ramdisk.tmp" };
    struct basic af1_creat = {
            .function_name = "creat",
            .return_state = ok,
            .file_type = &tmp_af1_file_type, .void_p_enum_file_type = void_p_enum_file_type_file_descriptor,
            .function_data = &tmp_af1_function_data, .void_p_enum_function_data = void_p_enum_function_data_open_function
    };



    struct memory_map_function tmp_af2_function_data = { .address = (void*)next_dummy_id++, .length = 32, .map_flags = { .anonymous = 0 } };
    struct basic af2_mmap = {
            .function_name = "mmap",
            .return_state = ok,
            .file_type = &tmp_af1_file_type, .void_p_enum_file_type = void_p_enum_file_type_file_descriptor,
            .function_data = &tmp_af2_function_data, .void_p_enum_function_data = void_p_enum_function_data_memory_map_function
    };

    struct file_memory tmp_bf1_file_type = { .address = tmp_af2_function_data.address, .length = tmp_af2_function_data.length };
    struct basic bf1_msync = {
            .function_name = "msync",
            .return_state = ok,
            .file_type = &tmp_bf1_file_type, .void_p_enum_file_type = void_p_enum_file_type_file_memory
    };

    struct memory_remap_function tmp_bf2_function_data = { .new_address = (void*)next_dummy_id++, .new_length = tmp_af2_function_data.length };
    struct basic bf2_mremap = {
            .function_name = "mremap",
            .return_state = ok,
            .file_type = &tmp_bf1_file_type, .void_p_enum_file_type = void_p_enum_file_type_file_memory,
            .function_data = &tmp_bf2_function_data, .void_p_enum_function_data = void_p_enum_function_data_memory_remap_function
    };

    struct basic bf3_msync_old;
    memcpy(&bf3_msync_old, &bf1_msync, sizeof(bf1_msync));

    struct file_memory tmp_bf4_file_type = { .address = tmp_bf2_function_data.new_address, .length = tmp_bf2_function_data.new_length };
    struct basic bf4_msync_new;
    memcpy(&bf4_msync_new, &bf1_msync, sizeof(bf1_msync));
    bf4_msync_new.file_type = &tmp_bf4_file_type;

    struct basic cf1_munmap = {
            .function_name = "munmap",
            .return_state = ok,
            .file_type = &tmp_bf4_file_type, .void_p_enum_file_type = void_p_enum_file_type_file_memory,
    };
    /* -- Create test data -- */



    /* -- (0) Add to trace: 'creat' -> 'mmap' -- */
    fnres_trace_fctevent(&af1_creat);
    fnres_trace_fctevent(&af2_mmap);

    /* -- (1) Set filename for traceable: 'msync' -- */
    fnres_trace_fctevent(&bf1_msync);
    CU_ASSERT_FATAL(STRINGS_ARE_EQUAL == strcmp(bf1_msync.traced_filename, tmp_af1_function_data.file_name));

    /* -- (2) (Re)Add to trace: mremap -> 'msync' w/ old address (verify whether removed) + updated 'msync' (w/ new address) -- */
    fnres_trace_fctevent(&bf2_mremap);

    fnres_trace_fctevent(&bf3_msync_old);
    CU_ASSERT_FATAL(STRINGS_ARE_EQUAL == strcmp(bf3_msync_old.traced_filename, FNAME_SPECIFIER_NOTFOUND));

    fnres_trace_fctevent(&bf4_msync_new);
    CU_ASSERT_FATAL(STRINGS_ARE_EQUAL == strcmp(bf4_msync_new.traced_filename, tmp_af1_function_data.file_name));

    /* -- (3) Remove mem-mapping: 'munmap' -> 'msync' -- */
    fnres_trace_fctevent(&cf1_munmap);

    fnres_trace_fctevent(&bf4_msync_new);
    CU_ASSERT_FATAL(STRINGS_ARE_EQUAL == strcmp(bf4_msync_new.traced_filename, FNAME_SPECIFIER_NOTFOUND));
}


/**
 * Tests MPI stuff pertaining ...
 *   - 'MPI_File' (e.g., 'MPI_File_open', 'MPI_File_set_view')
 *   - 'MPI_Request', i.e., immediate functions (e.g., 'MPI_File_iwrite', 'MPI_Wait')
 *   - close functions (e.g., 'MPI_Request_free', 'MPI_File_close')
 */
void test_mpi_open_immediate_close(void) {
    /* -- Create test data -- */
    struct file_mpi tmp_af1_file_type = { .mpi_file = (int)next_dummy_id++ };
    struct mpi_open_function tmp_af1_function_data = { .file_name = "/tmp/com.google.Keystone" };
    struct basic af1_MPI_File_open = {
            .function_name = "MPI_File_open",
            .return_state = ok,
            .file_type = &tmp_af1_file_type, .void_p_enum_file_type = void_p_enum_file_type_file_mpi,
            .function_data = &tmp_af1_function_data, .void_p_enum_function_data = void_p_enum_function_data_mpi_open_function
    };

    struct basic bf1_MPI_File_set_view = {
            .function_name = "MPI_File_set_view",
            .return_state = ok,
            .file_type = &tmp_af1_file_type, .void_p_enum_file_type = void_p_enum_file_type_file_mpi
    };

    struct mpi_immediate tmp_bf2_function_data = { .request_id = (int)next_dummy_id++ };
    struct basic bf2_MPI_File_iwrite = {
            .function_name = "MPI_File_iwrite",
            .return_state = ok,
            .file_type = &tmp_af1_file_type, .void_p_enum_file_type = void_p_enum_file_type_file_mpi,
            .function_data = &tmp_bf2_function_data, .void_p_enum_function_data = void_p_enum_function_data_mpi_immediate
    };

    struct request_mpi tmp_bf3_file_type = { .request_id = tmp_bf2_function_data.request_id };
    struct basic bf3_MPI_Wait = {
            .function_name = "MPI_Wait",
            .return_state = ok,
            .file_type = &tmp_bf3_file_type, .void_p_enum_file_type = void_p_enum_file_type_request_mpi
    };

    struct basic cf1_MPI_Request_free = {
            .function_name = "MPI_Request_free",
            .return_state = ok,
            .file_type = &tmp_bf2_function_data, .void_p_enum_file_type = void_p_enum_file_type_request_mpi
    };

    struct basic cf2_MPI_File_close = {
            .function_name = "MPI_File_close",
            .return_state = ok,
            .file_type = &tmp_af1_file_type, .void_p_enum_file_type = void_p_enum_file_type_file_mpi
    };
    /* -- Create test data -- */


    /* -- (1) Add (to trace) + set filename for traceable: 'MPI_File_open' -> 'MPI_File_set_view' -- */
    fnres_trace_fctevent(&af1_MPI_File_open);
    fnres_trace_fctevent(&bf1_MPI_File_set_view);
    CU_ASSERT_FATAL(STRINGS_ARE_EQUAL == strcmp(bf1_MPI_File_set_view.traced_filename,
                                                ((struct mpi_open_function*)af1_MPI_File_open.function_data)->file_name));

    /* -- (2) Add immediate request (to trace) + set filename for traceable: 'MPI_File_iwrite' -> 'MPI_Wait' -- */
    fnres_trace_fctevent(&bf2_MPI_File_iwrite);
    fnres_trace_fctevent(&bf3_MPI_Wait);
    CU_ASSERT_FATAL(STRINGS_ARE_EQUAL == strcmp(bf3_MPI_Wait.traced_filename,
                                                ((struct mpi_open_function*)af1_MPI_File_open.function_data)->file_name));

    /* -- (3) Remove (from trace): 'MPI_Request_free' -> 'MPI_File_close' -- */
    fnres_trace_fctevent(&cf1_MPI_Request_free);
    fnres_trace_fctevent(&cf2_MPI_File_close);

    fnres_trace_fctevent(&bf2_MPI_File_iwrite);
    fnres_trace_fctevent(&bf3_MPI_Wait);
    CU_ASSERT_FATAL(STRINGS_ARE_EQUAL == strcmp(bf2_MPI_File_iwrite.traced_filename, FNAME_SPECIFIER_NOTFOUND));
    CU_ASSERT_FATAL(STRINGS_ARE_EQUAL == strcmp(bf3_MPI_Wait.traced_filename, FNAME_SPECIFIER_NOTFOUND));
}

/**
 * Tests memory-mapping
 */
void test_posix_freopen_same_file(void) {
    /* -- Create test data -- */
    struct file_descriptor tmp_af1_file_type = { .descriptor = (int)next_dummy_id++ };
    struct open_function tmp_af1_function_data = { .file_name = "/var/tmp/ramdisk.tmp" };
    struct basic af1_creat = {
            .function_name = "creat",
            .return_state = ok,
            .file_type = &tmp_af1_file_type, .void_p_enum_file_type = void_p_enum_file_type_file_descriptor,
            .function_data = &tmp_af1_function_data, .void_p_enum_function_data = void_p_enum_function_data_open_function
    };


    struct open_function tmp_af2_function_data = { .file_name = NULL };
    struct basic af2_freopen = {
            .function_name = "freopen",
            .return_state = ok,
            .file_type = &tmp_af1_file_type, .void_p_enum_file_type = void_p_enum_file_type_file_descriptor,
            .function_data = &tmp_af2_function_data, .void_p_enum_function_data = void_p_enum_function_data_open_function
    };
    /* -- Create test data -- */



    /* -- (0) Add to trace: 'creat' -> 'freopen' (reopen same file again) -- */
    fnres_trace_fctevent(&af1_creat);
    fnres_trace_fctevent(&af2_freopen);
    CU_ASSERT_FATAL(STRINGS_ARE_EQUAL == strcmp(af2_freopen.traced_filename, tmp_af1_function_data.file_name));
}



CUNIT_CI_RUN("Suite_1",
             CUNIT_CI_TEST(test_posix_fildes_open_read_close),
             CUNIT_CI_TEST(test_posix_pipe2_lseek),
             CUNIT_CI_TEST(test_posix_stream_fildes_fileno_fwrite),
             CUNIT_CI_TEST(test_posix_creat_mmap_msync_mremap_munmap),
             CUNIT_CI_TEST(test_mpi_open_immediate_close),
             CUNIT_CI_TEST(test_posix_freopen_same_file));