#include <string.h>

#include "CUnit/CUnitCI.h"

#include "../../src/alloc.h"

struct wrapper_status active_wrapper_status;
char init_done = 0;
char hostname[] = "test-hostname";

extern char alloc_init_done;

void init_process() {
	// is called from wrappers if init_done = 0
}

#ifdef IOTRACE_ENABLE_LOGFILE
void write_into_buffer(struct basic *data) {
	// is called from wrappers if active_wrapper_status.<wrapper> = 1
}
#endif
#ifdef IOTRACE_ENABLE_INFLUXDB
void write_into_influxdb(struct basic *data) {
	// is called from wrappers if active_wrapper_status.<wrapper> = 1
}
#endif

void get_basic(struct basic *data)
{
	data->process_id = 1010;
	data->thread_id = 1010;

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

void free_memory(struct basic *data)
{
//	libiotrace_struct_free_basic(data);
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

CUNIT_CI_RUN("Suite_1",
             CUNIT_CI_TEST(test_toggle_alloc_wrapper),
			 CUNIT_CI_TEST(test_alloc_init)
            );
