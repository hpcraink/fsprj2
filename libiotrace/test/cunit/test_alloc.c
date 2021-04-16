#include <string.h>

#include "CUnit/CUnitCI.h"

#include "../../src/alloc.h"
#include "../../src/libiotrace_include_function.h"
#include "../../src/utils.h"

extern void* WRAP(malloc)(size_t size);

struct wrapper_status active_wrapper_status;
char init_done = 0;
static const int id = 1010;
static char hostname[] = "test-hostname";

extern char alloc_init_done;

static struct basic *cached_data;

void init_process() {
	// is called from wrappers if init_done = 0
}

// is called from wrappers if active_wrapper_status.<wrapper> = 1
void write_into_buffer(struct basic *data) {
	CU_ASSERT_FATAL(NULL != data);

	size_t len = libiotrace_struct_sizeof_basic(data);
	CU_ASSERT_FATAL(0 < len);

	cached_data = CALL_REAL_ALLOC_SYNC(calloc)(1, len);
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

void free_memory(struct basic *data) {
//	libiotrace_struct_free_basic(data);
}

void check_basic(const struct basic *data, const char *function_name, u_int64_t test_start, u_int64_t test_end) {
	CU_ASSERT_FATAL(id == data->process_id);
	CU_ASSERT_FATAL(id == data->thread_id);
	CU_ASSERT_FATAL(0 == strcmp(hostname, data->hostname));

	CU_ASSERT_FATAL(NULL == data->file_type);

	CU_ASSERT_FATAL(0 == strcmp(function_name, data->function_name));

	CU_ASSERT_FATAL(data->time_end >= data->time_start);
	CU_ASSERT_FATAL(test_start <= data->time_start);
	CU_ASSERT_FATAL(test_end >= data->time_end);
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

static void test_malloc(void) {
	char function_name[] = "malloc";
	void *mem;
	size_t size;
	u_int64_t test_start;
	u_int64_t test_end;

	toggle_alloc_wrapper(function_name, 1);
	CU_ASSERT_FATAL(1 == active_wrapper_status.malloc);

	size = 5;
	test_start = gettime();
	mem = __test_malloc(size);
	test_end = gettime();
	CU_ASSERT_FATAL(NULL != mem);
	memset(mem, 'm', size);
	CALL_REAL_ALLOC_SYNC(free)(mem);

	CU_ASSERT_FATAL(NULL != cached_data);

	CU_ASSERT_FATAL(0 == strcmp(function_name, cached_data->function_name));
	check_basic(cached_data, function_name, test_start, test_end);

	CU_ASSERT_FATAL(void_p_enum_function_data_alloc_function == cached_data->void_p_enum_function_data)
	CU_ASSERT_FATAL(size == ((struct alloc_function*)(cached_data->function_data))->size)

	CALL_REAL_ALLOC_SYNC(free)(cached_data);
}

CUNIT_CI_RUN("Suite_1", CUNIT_CI_TEST(test_toggle_alloc_wrapper),
		CUNIT_CI_TEST(test_alloc_init), CUNIT_CI_TEST(test_malloc));
