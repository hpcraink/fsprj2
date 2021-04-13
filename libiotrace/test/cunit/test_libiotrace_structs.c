#include <string.h>

#include "CUnit/CUnitCI.h"

/* One function from libiotrace_include_function.h uses
 * free(). In this context free could be wrapped as
 * __real_free. So __real_free() is necessary but should
 * only do what free() does (we don't need a free
 * wrapper to test the structures and the corresponding
 * functions. */
void __real_free(void *ptr) {
	free(ptr);
}

#include "../../src/libiotrace_include_struct.h"
#include "../../src/libiotrace_include_function.h"

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

static void fill_string(char *dest, ssize_t len, char repeat) {
	for (ssize_t i = 0; i < (len - 1); i++) {
		*dest = repeat;
		dest++;
	}
	*dest = '\0';
}

static void fill_number(void *number, ssize_t len) {
	const unsigned char filler = -1;
	for (ssize_t i = 0; i < len; i++) {
		*((unsigned char *)number) = filler;
	}
}

static void test_struct_basic(void) {
	struct basic data;
	struct errno_detail errno_detail_data;
	char hostname[HOST_NAME_MAX];
	char errno_text[MAX_ERROR_TEXT];
	pid_t pid_t_value;
	u_int64_t u_int64_t_value;
	int int_value;

	fill_string(hostname, sizeof(hostname), 'h');
	fill_string(errno_text, sizeof(errno_text), 'e');
	fill_number(&pid_t_value, sizeof(pid_t_value));
	fill_number(&u_int64_t_value, sizeof(u_int64_t_value));
	fill_number(&int_value, sizeof(int_value));

	errno_detail_data.errno_value = int_value;
	errno_detail_data.errno_text = errno_text;

	data.hostname = hostname;
	data.process_id = pid_t_value;
	data.thread_id = pid_t_value;
	fill_string(data.function_name, sizeof(data.function_name), 'f');
	data.time_start = u_int64_t_value;
	data.time_end = u_int64_t_value;
#ifdef IOTRACE_ENABLE_INFLUXDB
	data.time_diff = u_int64_t_value;
#endif
	data.return_state = unknown_read_write_state;
	data.return_state_detail = &errno_detail_data;
	LIBIOTRACE_STRUCT_SET_MALLOC_STRING_ARRAY_NULL(data, stacktrace_symbols)
	LIBIOTRACE_STRUCT_SET_MALLOC_PTR_ARRAY_NULL(data, stacktrace_pointer)
#ifdef LOG_WRAPPER_TIME
	data.wrapper.time_start = u_int64_t_value;
	data.wrapper.time_end = u_int64_t_value;
#endif
	LIBIOTRACE_STRUCT_SET_VOID_P_NULL(data, file_type)
	LIBIOTRACE_STRUCT_SET_VOID_P_NULL(data, function_data)

	char copy_buf[libiotrace_struct_sizeof_basic(&data)];
	memset(copy_buf, 0, sizeof(copy_buf));
	void *pos = (void *)libiotrace_struct_copy_basic((void *)copy_buf, &data);
	struct basic *copy = (struct basic *)(void *)copy_buf;

	CU_ASSERT_FATAL(copy_buf + sizeof(copy_buf) == pos);
	CU_ASSERT_FATAL(0 == strcmp(data.hostname, copy->hostname));
	CU_ASSERT_FATAL(data.process_id == copy->process_id);
	CU_ASSERT_FATAL(data.thread_id == copy->thread_id);
	CU_ASSERT_FATAL(0 == strcmp(data.function_name, copy->function_name));
	CU_ASSERT_FATAL(data.time_start == copy->time_start);
	CU_ASSERT_FATAL(data.time_end == copy->time_end);
#ifdef IOTRACE_ENABLE_INFLUXDB
	CU_ASSERT_FATAL(data.time_diff == copy->time_diff);
#endif
	CU_ASSERT_FATAL(data.return_state == copy->return_state);
	CU_ASSERT_FATAL(data.return_state_detail->errno_value == copy->return_state_detail->errno_value);
	CU_ASSERT_FATAL(0 == strcmp(data.return_state_detail->errno_text, copy->return_state_detail->errno_text));
}

CUNIT_CI_RUN("Suite_1",
             CUNIT_CI_TEST(test_struct_basic)
            );
