#include <string.h>

#include "CUnit/CUnitCI.h"
#include "../../src/alloc.h"

#undef WRAPPER_NAME
#define WRAPPER_NAME(function_name) \
	toggle_alloc_wrapper(#function_name, 1); \
	CU_ASSERT_FATAL(1 == active_wrapper_status.function_name); \
	toggle_alloc_wrapper(#function_name, 0); \
	CU_ASSERT_FATAL(0 == active_wrapper_status.function_name); \
	toggle_alloc_wrapper(#function_name, 1); \
	CU_ASSERT_FATAL(1 == active_wrapper_status.function_name);

struct wrapper_status active_wrapper_status;
char init_done = 0;
char hostname[] = "test-hostname";

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
#include "../../src/alloc_wrapper.h"
}

CUNIT_CI_RUN("Suite_1",
             CUNIT_CI_TEST(test_toggle_alloc_wrapper)
            );
