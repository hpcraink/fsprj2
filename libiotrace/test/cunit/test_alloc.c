#include <string.h>

#include "CUnit/CUnitCI.h"

#include "../../src/alloc.h"
#include "../../src/libiotrace_functions.c"
#include "../../src/utils.h"
#include "../../src/gettime.h"

extern void* WRAP(malloc)(size_t size);
extern void WRAP(free)(void *ptr);
extern void* WRAP(calloc)(size_t nmemb, size_t size);
extern void* WRAP(realloc)(void *ptr, size_t size);
#ifdef HAVE_REALLOCARRAY
extern void* WRAP(reallocarray)(void *ptr, size_t nmemb, size_t size);
#endif

struct wrapper_status active_wrapper_status;
char init_done = 0;
static const int id = 1010;
static char hostname[] = "test-hostname";

extern char alloc_init_done;

static struct basic *cached_data = NULL;

void init_process(void) {
	// is called from wrappers if init_done = 0
}

// is called from wrappers if active_wrapper_status.<wrapper> = 1
void io_log_file_buffer_write(struct basic *data) {
	CU_ASSERT_FATAL(NULL != data);

	size_t len = libiotrace_struct_sizeof_basic(data);
	CU_ASSERT_FATAL(0 < len);

	cached_data = calloc(1, len);
	CU_ASSERT_FATAL(NULL != cached_data);

	void *pos = (void*) libiotrace_struct_copy_basic((void*) cached_data, data);
	CU_ASSERT_FATAL((char *)((void*)cached_data) + len == pos);
}

void get_basic(struct basic *data) {
	data->process_id = id;
	data->thread_id = id;

	data->hostname = hostname;

//	if (0 < stacktrace_depth && (stacktrace_ptr || stacktrace_symbol))
//	{
//		get_stacktrace(data);
//	}
//	else
//	{
	LIBIOTRACE_STRUCT_SET_MALLOC_STRING_ARRAY_NULL((*data), stacktrace_symbols)
	LIBIOTRACE_STRUCT_SET_MALLOC_PTR_ARRAY_NULL((*data), stacktrace_pointer)
//	}
}

void free_memory(struct basic *data ATTRIBUTE_UNUSED) {
//	libiotrace_struct_free_basic(data);
}

#ifdef WITH_FILENAME_RESOLUTION
static char filestr[] = "a file";
void fnres_trace_fctevent(struct basic *fctevent) {
	strcpy(fctevent->traced_filename, filestr);
}
#endif

void check_basic(const struct basic *data, const char *function_name, u_int64_t test_start, u_int64_t test_end) {
	CU_ASSERT_FATAL(id == data->process_id);
	CU_ASSERT_FATAL(id == data->thread_id);
	CU_ASSERT_FATAL(0 == strcmp(hostname, data->hostname));

	CU_ASSERT_FATAL(0 == strcmp(function_name, data->function_name));

	CU_ASSERT_FATAL(data->time_end >= data->time_start);
	CU_ASSERT_FATAL(test_start <= data->time_start);
	CU_ASSERT_FATAL(test_end >= data->time_end);
#ifdef IOTRACE_ENABLE_INFLUXDB
	CU_ASSERT_FATAL(data->time_diff >= data->time_end - data->time_start);
#endif
#ifdef WITH_FILENAME_RESOLUTION
	CU_ASSERT_FATAL(0 == strcmp(filestr, data->traced_filename));
#endif
}

/* run at the start of the suite */
CU_SUITE_SETUP() {
	return CUE_SUCCESS;
}

/* run at the end of the suite */
CU_SUITE_TEARDOWN() {
	return CUE_SUCCESS;
}

/* run at the start of each test */
CU_TEST_SETUP() {
}

/* run at the end of each test */
CU_TEST_TEARDOWN() {
}

static void test_toggle_alloc_wrapper(void) {
#undef WRAPPER_NAME_TO_SOURCE
#define WRAPPER_NAME_TO_SOURCE WRAPPER_NAME_TO_LIBIOTRACE_STRUCT
#undef LIBIOTRACE_STRUCT_CHAR
#define LIBIOTRACE_STRUCT_CHAR(function_name) \
	toggle_alloc_wrapper(#function_name, 1); \
	CU_ASSERT_FATAL(1 == active_wrapper_status.function_name); \
	toggle_alloc_wrapper(#function_name, 0); \
	CU_ASSERT_FATAL(0 == active_wrapper_status.function_name); \
	toggle_alloc_wrapper(#function_name, 1); \
	CU_ASSERT_FATAL(1 == active_wrapper_status.function_name);
#include "../../src/alloc_wrapper.h"
}

static void test_alloc_init(void) {
	CU_ASSERT_FATAL(1 == alloc_init_done); // alloc_init was called per ATTRIBUTE_CONSTRUCTOR
}

void* call_and_check_malloc(size_t size, char *function_name) {
	void *mem;
	u_int64_t test_start;
	u_int64_t test_end;
	int ret_errno;

	test_start = gettime();
	mem = __test_malloc(size);
	ret_errno = errno;
	test_end = gettime();

	CU_ASSERT_FATAL(NULL != cached_data);

	CU_ASSERT_FATAL(NULL != cached_data->__file_type);

	check_basic(cached_data, function_name, test_start, test_end);

	CU_ASSERT_FATAL(__void_p_enum_function_data_alloc_function == cached_data->__void_p_enum_function_data)
	CU_ASSERT_FATAL(size == ((struct alloc_function*)(cached_data->__function_data))->size)

	errno = ret_errno;
	return mem;
}

void* call_and_check_calloc(size_t nmemb, size_t size, char *function_name) {
	void *mem;
	u_int64_t test_start;
	u_int64_t test_end;
	int ret_errno;

	test_start = gettime();
	mem = __test_calloc(nmemb, size);
	ret_errno = errno;
	test_end = gettime();

	CU_ASSERT_FATAL(NULL != cached_data);

	CU_ASSERT_FATAL(NULL != cached_data->__file_type);

	check_basic(cached_data, function_name, test_start, test_end);

	CU_ASSERT_FATAL(__void_p_enum_function_data_alloc_function == cached_data->__void_p_enum_function_data)
	CU_ASSERT_FATAL(size == ((struct alloc_function*)(cached_data->__function_data))->size)

	errno = ret_errno;
	return mem;
}

void* call_and_check_realloc(void *ptr, size_t size, char *function_name) {
	void *mem;
	u_int64_t test_start;
	u_int64_t test_end;
	int ret_errno;

	test_start = gettime();
	mem = __test_realloc(ptr, size);
	ret_errno = errno;
	test_end = gettime();

	CU_ASSERT_FATAL(NULL != cached_data);

	CU_ASSERT_FATAL(NULL != cached_data->__file_type);

	check_basic(cached_data, function_name, test_start, test_end);

	CU_ASSERT_FATAL(__void_p_enum_function_data_alloc_function == cached_data->__void_p_enum_function_data)
	CU_ASSERT_FATAL(size == ((struct alloc_function*)(cached_data->__function_data))->size)

	errno = ret_errno;
	return mem;
}

#ifdef HAVE_REALLOCARRAY
void* call_and_check_reallocarray(void *ptr, size_t nmemb, size_t size, char *function_name) {
	void *mem;
	u_int64_t test_start;
	u_int64_t test_end;
	int ret_errno;

	test_start = gettime();
	mem = __test_reallocarray(ptr, nmemb, size);
	ret_errno = errno;
	test_end = gettime();

	CU_ASSERT_FATAL(NULL != cached_data);

	CU_ASSERT_FATAL(NULL != cached_data->__file_type);

	check_basic(cached_data, function_name, test_start, test_end);

	CU_ASSERT_FATAL(__void_p_enum_function_data_alloc_function == cached_data->__void_p_enum_function_data)
	CU_ASSERT_FATAL(size == ((struct alloc_function*)(cached_data->__function_data))->size)

	errno = ret_errno;
	return mem;
}
#endif

void *malloc_ENOMEM(size_t size ATTRIBUTE_UNUSED) {
	errno = ENOMEM;
	return NULL;
}

void *calloc_ENOMEM(size_t nmemb ATTRIBUTE_UNUSED, size_t size ATTRIBUTE_UNUSED) {
	errno = ENOMEM;
	return NULL;
}

void *realloc_ENOMEM(void * ptr ATTRIBUTE_UNUSED, size_t nmemb ATTRIBUTE_UNUSED) {
	errno = ENOMEM;
	return NULL;
}

#ifdef HAVE_REALLOCARRAY
void *reallocarray_ENOMEM(void * ptr ATTRIBUTE_UNUSED, size_t nmemb ATTRIBUTE_UNUSED, size_t size ATTRIBUTE_UNUSED) {
	errno = ENOMEM;
	return NULL;
}
#endif

static void test_malloc(void) {
	char function_name[] = "malloc";
	void *mem;
	size_t size;
	int ret_errno;
	void *(*tmp_malloc)(size_t);

	toggle_alloc_wrapper(function_name, 1);
	CU_ASSERT_FATAL(1 == active_wrapper_status.malloc);

	// alloc with size 1

	size = 1;
	mem = call_and_check_malloc(size, function_name);
	CU_ASSERT_FATAL(NULL != mem);
	memset(mem, 'm', size);
	free(mem);

	CU_ASSERT_FATAL(ok == cached_data->return_state);
	CU_ASSERT_FATAL(NULL == cached_data->return_state_detail);

	free(cached_data);
	cached_data = NULL;

	// alloc with size 5

	size = 5;
	mem = call_and_check_malloc(size, function_name);
	CU_ASSERT_FATAL(NULL != mem);
	memset(mem, 'm', size);
	free(mem);

	CU_ASSERT_FATAL(ok == cached_data->return_state);
	CU_ASSERT_FATAL(NULL == cached_data->return_state_detail);

	free(cached_data);
	cached_data = NULL;

	// alloc with size 5 returns ENOMEM

	size = 5;
	tmp_malloc = __real_malloc;
	__real_malloc = malloc_ENOMEM;
	mem = call_and_check_malloc(size, function_name);
	ret_errno = errno;
	__real_malloc = tmp_malloc;
	CU_ASSERT_FATAL(NULL == mem);

	CU_ASSERT_FATAL(error == cached_data->return_state);
	CU_ASSERT_FATAL(NULL != cached_data->return_state_detail);
	CU_ASSERT_FATAL(ret_errno == cached_data->return_state_detail->errno_value);
	CU_ASSERT_FATAL(0 == strcmp(strerror(ret_errno), cached_data->return_state_detail->errno_text));

	free(cached_data);
	cached_data = NULL;
}

static void test_free(void) {
	void* mem;
	void* (*tmp_calloc)(size_t nmemb, size_t size);

	// free memory allocated from static calloc buffer

	tmp_calloc = __real_calloc;
	__real_calloc = NULL;
	mem = __test_calloc(1, 1);
	CU_ASSERT_FATAL(NULL != mem);
	memset(mem, 'm', 1);
	__test_free(mem); // should return without free
	__test_free(mem); // no double free error because nothing is freed
	__real_calloc = tmp_calloc;

	// free memory allocated with malloc wrapper

	mem = __test_malloc(1);
	CU_ASSERT_FATAL(NULL != mem);
	memset(mem, 'm', 1);
	__test_free(mem);

	// free memory allocated with malloc

	mem = malloc(1);
	CU_ASSERT_FATAL(NULL != mem);
	memset(mem, 'm', 1);
	__test_free(mem);
}

static void test_calloc(void) {
	char function_name[] = "calloc";
	char zeros[5];
	int i;
	int ret_errno;
	size_t size;
	void* mem[STATIC_CALLOC_BUFFER_SIZE + 1];
	void* (*tmp_calloc)(size_t nmemb, size_t size);

	memset(zeros, 0, sizeof(zeros));

	toggle_alloc_wrapper(function_name, 1);
	CU_ASSERT_FATAL(1 == active_wrapper_status.calloc);

	// allocate 0 bytes in static calloc buffer

	cached_data = NULL;
	tmp_calloc = __real_calloc;
	__real_calloc = NULL;
	mem[0] = __test_calloc(0, 1);
	CU_ASSERT_FATAL(NULL == mem[0]);
	CU_ASSERT_FATAL(NULL == cached_data);
	mem[0] = __test_calloc(1, 0);
	CU_ASSERT_FATAL(NULL == mem[0]);
	CU_ASSERT_FATAL(NULL == cached_data);
	__real_calloc = tmp_calloc;

	// overflow during allocation in static calloc buffer

	cached_data = NULL;
	tmp_calloc = __real_calloc;
	__real_calloc = NULL;
	mem[0] = __test_calloc(-1, 1);
	CU_ASSERT_FATAL(NULL == mem[0]);
	CU_ASSERT_FATAL(ENOMEM == errno);
	CU_ASSERT_FATAL(NULL == cached_data);
	mem[0] = __test_calloc(1, -1);
	CU_ASSERT_FATAL(NULL == mem[0]);
	CU_ASSERT_FATAL(ENOMEM == errno);
	CU_ASSERT_FATAL(NULL == cached_data);
	__real_calloc = tmp_calloc;

	// allocate all memory available in static calloc buffer

	cached_data = NULL;
	tmp_calloc = __real_calloc;
	__real_calloc = NULL;
	for (i = 0; i <= STATIC_CALLOC_BUFFER_SIZE; i++) {
		mem[i] = __test_calloc(1, 1);
		CU_ASSERT_FATAL(NULL == cached_data);
		if (NULL == mem[i]) {
			CU_ASSERT_FATAL(ENOMEM == errno);
			break; // all memory available was allocated
		} else {
			CU_ASSERT_FATAL('\0' == *((char *)mem[i]));
		}
	}
	if (STATIC_CALLOC_BUFFER_SIZE + 1 == i) {
		CU_FAIL_FATAL("It should not be possible to allocate more memory than STATIC_CALLOC_BUFFER_SIZE.");
	}
	for (int l = 0; l < i; l++) {
		__test_free(mem[l]);
		CU_ASSERT_FATAL(NULL == cached_data);
	}
	mem[0] = __test_calloc(1, 1);
	CU_ASSERT_FATAL(NULL == mem[0]);
	CU_ASSERT_FATAL(ENOMEM == errno);
	CU_ASSERT_FATAL(NULL == cached_data);
	__real_calloc = tmp_calloc;

	// alloc with size 1

	size = 1;
	mem[0] = call_and_check_calloc(1, size, function_name);
	CU_ASSERT_FATAL(NULL != mem[0]);
	CU_ASSERT_FATAL('\0' == *((char *)mem[0]));
	memset(mem[0], 'm', size);
	free(mem[0]);

	CU_ASSERT_FATAL(ok == cached_data->return_state);
	CU_ASSERT_FATAL(NULL == cached_data->return_state_detail);

	free(cached_data);
	cached_data = NULL;

	// alloc with size 5

	size = 5;
	mem[0] = call_and_check_calloc(1, size, function_name);
	CU_ASSERT_FATAL(NULL != mem[0]);
	CU_ASSERT_FATAL(0 == memcmp(mem[0], zeros, size));
	memset(mem[0], 'm', size);
	free(mem[0]);

	CU_ASSERT_FATAL(ok == cached_data->return_state);
	CU_ASSERT_FATAL(NULL == cached_data->return_state_detail);

	free(cached_data);
	cached_data = NULL;

	// alloc with size 5 returns ENOMEM

	size = 5;
	tmp_calloc = __real_calloc;
	__real_calloc = calloc_ENOMEM;
	mem[0] = call_and_check_calloc(1, size, function_name);
	ret_errno = errno;
	__real_calloc = tmp_calloc;
	CU_ASSERT_FATAL(NULL == mem[0]);

	CU_ASSERT_FATAL(error == cached_data->return_state);
	CU_ASSERT_FATAL(NULL != cached_data->return_state_detail);
	CU_ASSERT_FATAL(ret_errno == cached_data->return_state_detail->errno_value);
	CU_ASSERT_FATAL(0 == strcmp(strerror(ret_errno), cached_data->return_state_detail->errno_text));

	free(cached_data);
	cached_data = NULL;
}

static void test_realloc(void) {
	char function_name[] = "realloc";
	void *mem;
	void *tmp_mem;
	size_t size;
	int ret_errno;
	void *(*tmp_realloc)(void *, size_t);

	toggle_alloc_wrapper(function_name, 1);
	CU_ASSERT_FATAL(1 == active_wrapper_status.realloc);

	// new allocation with size 1

	size = 1;
	mem = call_and_check_realloc(NULL, size, function_name);
	CU_ASSERT_FATAL(NULL != mem);
	memset(mem, 'm', size);

	CU_ASSERT_FATAL(ok == cached_data->return_state);
	CU_ASSERT_FATAL(NULL == cached_data->return_state_detail);

	free(cached_data);
	cached_data = NULL;

	// resize allocation to size 5

	size = 5;
	mem = call_and_check_realloc(mem, size, function_name);
	CU_ASSERT_FATAL(NULL != mem);
	memset(mem, 'm', size);

	CU_ASSERT_FATAL(ok == cached_data->return_state);
	CU_ASSERT_FATAL(NULL == cached_data->return_state_detail);

	free(cached_data);
	cached_data = NULL;

	// resize allocation to size 3

	size = 3;
	mem = call_and_check_realloc(mem, size, function_name);
	CU_ASSERT_FATAL(NULL != mem);
	memset(mem, 'm', size);

	CU_ASSERT_FATAL(ok == cached_data->return_state);
	CU_ASSERT_FATAL(NULL == cached_data->return_state_detail);

	free(cached_data);
	cached_data = NULL;

	// resize allocation to size 10 returns ENOMEM

	size = 10;
	tmp_realloc = __real_realloc;
	__real_realloc = realloc_ENOMEM;
	mem = call_and_check_realloc(mem, size, function_name);
	ret_errno = errno;
	__real_realloc = tmp_realloc;
	CU_ASSERT_FATAL(NULL == mem);

	CU_ASSERT_FATAL(error == cached_data->return_state);
	CU_ASSERT_FATAL(NULL != cached_data->return_state_detail);
	CU_ASSERT_FATAL(ret_errno == cached_data->return_state_detail->errno_value);
	CU_ASSERT_FATAL(0 == strcmp(strerror(ret_errno), cached_data->return_state_detail->errno_text));

	free(cached_data);
	cached_data = NULL;

	// free memory

	size = 0;
	cached_data = NULL;
	tmp_mem = call_and_check_realloc(mem, size, function_name);
	CU_ASSERT_FATAL(NULL == mem || tmp_mem == mem);
	if (NULL != mem) {
		/* If the size of the space requested is zero, the behavior is
		 * implementation-defined: either a null pointer is returned,
		 * or the behavior is as if the size was some nonzero value,
		 * except that the returned pointer shall not be used to access
		 * an object (7.20.3.1 for C11, 7.22.3.1 for C1x) */
		free(mem);
	}

	CU_ASSERT_FATAL(ok == cached_data->return_state);
	CU_ASSERT_FATAL(NULL == cached_data->return_state_detail);

	free(cached_data);
	cached_data = NULL;
}

static void test_reallocarray(void) {
#ifdef HAVE_REALLOCARRAY
	char function_name[] = "reallocarray";
	void *mem;
	void *tmp_mem;
	size_t size;
	int ret_errno;
	void *(*tmp_reallocarray)(void *, size_t, size_t);

	toggle_alloc_wrapper(function_name, 1);
	CU_ASSERT_FATAL(1 == active_wrapper_status.reallocarray);

	// new allocation with size 1

	size = 1;
	mem = call_and_check_reallocarray(NULL, 1, size, function_name);
	CU_ASSERT_FATAL(NULL != mem);
	memset(mem, 'm', size);

	CU_ASSERT_FATAL(ok == cached_data->return_state);
	CU_ASSERT_FATAL(NULL == cached_data->return_state_detail);

	free(cached_data);
	cached_data = NULL;

	// resize allocation to size 5

	size = 5;
	mem = call_and_check_reallocarray(mem, 1, size, function_name);
	CU_ASSERT_FATAL(NULL != mem);
	memset(mem, 'm', size);

	CU_ASSERT_FATAL(ok == cached_data->return_state);
	CU_ASSERT_FATAL(NULL == cached_data->return_state_detail);

	free(cached_data);
	cached_data = NULL;

	// resize allocation to size 3

	size = 3;
	mem = call_and_check_reallocarray(mem, 1, size, function_name);
	CU_ASSERT_FATAL(NULL != mem);
	memset(mem, 'm', size);

	CU_ASSERT_FATAL(ok == cached_data->return_state);
	CU_ASSERT_FATAL(NULL == cached_data->return_state_detail);

	free(cached_data);
	cached_data = NULL;

	// resize allocation to size 10 returns ENOMEM

	size = 10;
	tmp_reallocarray = __real_reallocarray;
	__real_reallocarray = reallocarray_ENOMEM;
	mem = call_and_check_reallocarray(mem, 1, size, function_name);
	ret_errno = errno;
	__real_reallocarray = tmp_reallocarray;
	CU_ASSERT_FATAL(NULL == mem);

	CU_ASSERT_FATAL(error == cached_data->return_state);
	CU_ASSERT_FATAL(NULL != cached_data->return_state_detail);
	CU_ASSERT_FATAL(ret_errno == cached_data->return_state_detail->errno_value);
	CU_ASSERT_FATAL(0 == strcmp(strerror(ret_errno), cached_data->return_state_detail->errno_text));

	free(cached_data);
	cached_data = NULL;

	// free memory

	size = 0;
	cached_data = NULL;
	tmp_mem = call_and_check_reallocarray(mem, 1, size, function_name);
	CU_ASSERT_FATAL(NULL == mem || tmp_mem == mem);
	if (NULL != mem) {
		/* If the size of the space requested is zero, the behavior is
		 * implementation-defined: either a null pointer is returned,
		 * or the behavior is as if the size were some nonzero value,
		 * except that the returned pointer shall not be used to access
		 * an object (7.20.3.1 for C11, 7.22.3.1 for C1x) */
		free(mem);
	}

	CU_ASSERT_FATAL(ok == cached_data->return_state);
	CU_ASSERT_FATAL(NULL == cached_data->return_state_detail);

	free(cached_data);
	cached_data = NULL;
#endif
}

CUNIT_CI_RUN("Suite_1",
		CUNIT_CI_TEST(test_toggle_alloc_wrapper),
		CUNIT_CI_TEST(test_alloc_init),
		CUNIT_CI_TEST(test_malloc),
		CUNIT_CI_TEST(test_free),
		CUNIT_CI_TEST(test_calloc),
		CUNIT_CI_TEST(test_realloc),
		CUNIT_CI_TEST(test_reallocarray)
		)
