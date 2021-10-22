#include <string.h>

#include "CUnit/CUnitCI.h"

#define JSMN_STRICT
#include "../../src/jsmn.h"

/* libiotrace_include_function.h uses POSIX functions. The functions
 * are wrapped if WITH_POSIX_IO is defined. In this test file only
 * the structures and no wrappers are tested. So use of wrappers is
 * disabled via undef of WITH_POSIX_IO. It's the same for
 * WITH_ALLOC */
#undef WITH_POSIX_IO
#undef WITH_ALLOC
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

static void fill_string(char *dest, const ssize_t len, const char repeat) {
	for (ssize_t i = 0; i < (len - 1); i++) {
		*dest = repeat;
		dest++;
	}
	*dest = '\0';
}

static void fill_number(void *number, const ssize_t len) {
	const unsigned char filler = -1;
	for (ssize_t i = 0; i < len; i++) {
		*(((unsigned char*) number) + i) = filler;
	}
}

static void check_basic_copy(const struct basic *data, const struct basic *copy) {
	CU_ASSERT_FATAL(data->hostname != copy->hostname);
	CU_ASSERT_FATAL(0 == strcmp(data->hostname, copy->hostname));
	CU_ASSERT_FATAL(data->process_id == copy->process_id);
	CU_ASSERT_FATAL(data->thread_id == copy->thread_id);
	CU_ASSERT_FATAL(0 == strcmp(data->function_name, copy->function_name));
	CU_ASSERT_FATAL(data->time_start == copy->time_start);
	CU_ASSERT_FATAL(data->time_end == copy->time_end);
#ifdef IOTRACE_ENABLE_INFLUXDB
	CU_ASSERT_FATAL(data->time_diff == copy->time_diff);
#endif
	CU_ASSERT_FATAL(data->return_state == copy->return_state);
	CU_ASSERT_FATAL(
			data->return_state_detail->errno_value
					== copy->return_state_detail->errno_value);
	CU_ASSERT_FATAL(
			data->return_state_detail->errno_text
					!= copy->return_state_detail->errno_text);
	CU_ASSERT_FATAL(
			0
					== strcmp(data->return_state_detail->errno_text,
							copy->return_state_detail->errno_text));
	if (NULL == data->stacktrace_symbols) {
		CU_ASSERT_FATAL(NULL == copy->stacktrace_symbols);
	}
	if (NULL == data->stacktrace_pointer) {
		CU_ASSERT_FATAL(NULL == copy->stacktrace_pointer);
	}
#ifdef LOG_WRAPPER_TIME
	CU_ASSERT_FATAL(data->wrapper.time_start == copy->wrapper.time_start);
	CU_ASSERT_FATAL(data->wrapper.time_end == copy->wrapper.time_end);
#endif
	if (NULL == data->file_type) {
		CU_ASSERT_FATAL(NULL == copy->file_type);
	} else {
		CU_ASSERT_FATAL(
				data->void_p_enum_file_type == copy->void_p_enum_file_type);
		if (void_p_enum_file_type_file_descriptor
				== data->void_p_enum_file_type) {
			CU_ASSERT_FATAL(
					((struct file_descriptor* )(data->file_type))->descriptor
							== ((struct file_descriptor* )(copy->file_type))->descriptor);
		}
	}
	if (NULL == data->function_data) {
		CU_ASSERT_FATAL(NULL == copy->function_data);
	} else {
		CU_ASSERT_FATAL(
				data->void_p_enum_function_data
						== copy->void_p_enum_function_data);
		if (void_p_enum_function_data_mpi_waitall
				== data->void_p_enum_function_data) {
			CU_ASSERT_FATAL(
					((struct mpi_waitall* )(data->function_data))->requests
							== ((struct mpi_waitall* )(copy->function_data))->requests);
		}
	}
}

static void check_json_string(const char *print_buf, const jsmntok_t *token,
		const char *string) {
	CU_ASSERT_FATAL(token->type == JSMN_STRING);
	CU_ASSERT_FATAL(token->start >= 0);
	CU_ASSERT_FATAL(token->end >= 0);
	CU_ASSERT_FATAL(token->start <= token->end);
	CU_ASSERT_FATAL(strlen(string) == (size_t)token->end - (size_t)token->start);
	CU_ASSERT_FATAL(
			0
					== strncmp(print_buf + token->start, string,
							token->end - token->start));
}

static void check_json_number(const char *print_buf, const jsmntok_t *token,
		const long long int number, char is_unsigned) {
	long long int value_number;

	CU_ASSERT_FATAL(token->type == JSMN_PRIMITIVE);

	// check if it's a number not a other primitive (like true/false or null)
	char start_char = *(print_buf + token->start);
	CU_ASSERT_FATAL(
			'-' == start_char || ('0' <= start_char && '9' >= start_char));

	// copy json number to new '\0' terminated string
	ssize_t len = token->end - token->start;
	char value_string[len + 1];
	strncpy(value_string, print_buf + token->start, len);
	value_string[len] = '\0';

	// convert string to long long int
	char *endptr;
	if (is_unsigned) {
		value_number = strtoull(value_string, &endptr, 10);
	} else {
		value_number = strtoll(value_string, &endptr, 10);
	}

	// check if each char in string was a valid digit
	CU_ASSERT_FATAL('\0' == *endptr);

	// is converted json number correct?
	CU_ASSERT_FATAL(number == value_number);
}

static void check_json_enum_read_write_state(const char *print_buf,
		const jsmntok_t *token, enum read_write_state read_write_state) {
	CU_ASSERT_FATAL(token->type == JSMN_STRING);

	char buf[libiotrace_struct_max_size_enum_read_write_state() + 1];
	libiotrace_struct_print_enum_read_write_state(buf, sizeof(buf),
			&read_write_state);

	// compare values with enclosing '"'
	CU_ASSERT_FATAL(
			0
					== strncmp(print_buf + token->start - 1, buf,
							token->end - token->start + 2));
}

static void check_basic_print(const struct basic *data, const char *print_buf,
		const int len) {
	jsmn_parser parser;
	jsmn_init(&parser);
	int token_count = jsmn_parse(&parser, print_buf, len, NULL, 0);

	CU_ASSERT_FATAL(0 < token_count);

	jsmntok_t tokens[token_count];
	jsmn_init(&parser);
	int ret = jsmn_parse(&parser, print_buf, len, tokens, token_count);

	CU_ASSERT_FATAL(token_count == ret);

	int i = 0;
	CU_ASSERT_FATAL(tokens[i++].type == JSMN_OBJECT);
#ifdef WITH_FILENAME_RESOLUTION
    check_json_string(print_buf, &tokens[i++], "traced_filename");
    check_json_string(print_buf, &tokens[i++], data->traced_filename);
#endif
	check_json_string(print_buf, &tokens[i++], "hostname");
	check_json_string(print_buf, &tokens[i++], data->hostname);
	check_json_string(print_buf, &tokens[i++], "process_id");
	check_json_number(print_buf, &tokens[i++], data->process_id, 0);
	check_json_string(print_buf, &tokens[i++], "thread_id");
	check_json_number(print_buf, &tokens[i++], data->thread_id, 0);
	check_json_string(print_buf, &tokens[i++], "function_name");
	check_json_string(print_buf, &tokens[i++], data->function_name);
	check_json_string(print_buf, &tokens[i++], "time_start");
	check_json_number(print_buf, &tokens[i++], data->time_start, 1);
	check_json_string(print_buf, &tokens[i++], "time_end");
	check_json_number(print_buf, &tokens[i++], data->time_end, 1);
#ifdef IOTRACE_ENABLE_INFLUXDB
	check_json_string(print_buf, &tokens[i++], "time_diff");
	check_json_number(print_buf, &tokens[i++], data->time_diff, 1);
#endif
	check_json_string(print_buf, &tokens[i++], "return_state");
	check_json_enum_read_write_state(print_buf, &tokens[i++],
			data->return_state);
	check_json_string(print_buf, &tokens[i++], "return_state_detail");
	CU_ASSERT_FATAL(tokens[i++].type == JSMN_OBJECT);
	CU_ASSERT_FATAL(tokens[i - 1].size == 2);
	check_json_string(print_buf, &tokens[i++], "errno_value");
	check_json_number(print_buf, &tokens[i++],
			data->return_state_detail->errno_value, 0);
	check_json_string(print_buf, &tokens[i++], "errno_text");
	check_json_string(print_buf, &tokens[i++],
			data->return_state_detail->errno_text);
#ifdef LOG_WRAPPER_TIME
	check_json_string(print_buf, &tokens[i++], "wrapper");
	CU_ASSERT_FATAL(tokens[i++].type == JSMN_OBJECT);
	CU_ASSERT_FATAL(tokens[i-1].size == 2);
	check_json_string(print_buf, &tokens[i++], "time_start");
	check_json_number(print_buf, &tokens[i++], data->wrapper.time_start, 1);
	check_json_string(print_buf, &tokens[i++], "time_end");
	check_json_number(print_buf, &tokens[i++], data->wrapper.time_end, 1);
#endif
	if (NULL != data->file_type) {
		check_json_string(print_buf, &tokens[i++], "file_type");
		CU_ASSERT_FATAL(tokens[i++].type == JSMN_OBJECT);
		if (void_p_enum_file_type_file_descriptor
				== data->void_p_enum_file_type) {
			CU_ASSERT_FATAL(tokens[i - 1].size == 1);
			check_json_string(print_buf, &tokens[i++], "descriptor");
			check_json_number(print_buf, &tokens[i++],
					((struct file_descriptor*) (data->file_type))->descriptor,
					0);
		}
	}
	if (NULL != data->function_data) {
		check_json_string(print_buf, &tokens[i++], "function_data");
		CU_ASSERT_FATAL(tokens[i++].type == JSMN_OBJECT);
		if (void_p_enum_function_data_mpi_waitall
				== data->void_p_enum_function_data) {
			if (NULL
					== ((struct mpi_waitall*) (data->function_data))->requests) {
				CU_ASSERT_FATAL(tokens[i - 1].size == 0);
			} else {
				CU_ASSERT_FATAL(tokens[i - 1].size == 1);
				check_json_string(print_buf, &tokens[i++], "requests");
			}
		}
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

	// initialize basic structure without substructures

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

	// copy basic structure without substructures

	char copy_buf[libiotrace_struct_sizeof_basic(&data)];
	memset(copy_buf, 0, sizeof(copy_buf));
	void *pos = (void*) libiotrace_struct_copy_basic((void*) copy_buf, &data);
	struct basic *copy = (struct basic*) (void*) copy_buf;

	// check copy of basic structure without substructures

	CU_ASSERT_FATAL(copy_buf + sizeof(copy_buf) == pos);
	check_basic_copy(&data, copy);

	// print basic structure without substructures as json

	char print_buf[libiotrace_struct_max_size_basic()];
	memset(print_buf, 0, sizeof(print_buf));
	size_t len = libiotrace_struct_print_basic(print_buf, sizeof(print_buf),
			&data);

	// check print of basic structure without substructures

	CU_ASSERT_FATAL(sizeof(print_buf) >= len);
	CU_ASSERT_FATAL(print_buf[0] == '{');
	CU_ASSERT_FATAL(print_buf[len - 1] == '}');
	check_basic_print(&data, print_buf, len);

	// add substructures to basic structure

	struct file_descriptor file_descriptor_data;
	struct mpi_waitall mpi_waitall_data;

	file_descriptor_data.descriptor = int_value;
	LIBIOTRACE_STRUCT_SET_VOID_P(data, file_type, file_descriptor,
			file_descriptor_data)
	LIBIOTRACE_STRUCT_SET_STRUCT_ARRAY_NULL(mpi_waitall_data, requests)
	LIBIOTRACE_STRUCT_SET_VOID_P(data, function_data, mpi_waitall,
			mpi_waitall_data)

	// copy basic structure with substructures

	char copy_buf2[libiotrace_struct_sizeof_basic(&data)];
	memset(copy_buf2, 0, sizeof(copy_buf2));
	pos = (void*) libiotrace_struct_copy_basic((void*) copy_buf2, &data);
	copy = (struct basic*) (void*) copy_buf2;

	// check copy of basic structure with substructures

	CU_ASSERT_FATAL(copy_buf2 + sizeof(copy_buf2) == pos);
	check_basic_copy(&data, copy);

	// print basic structure with substructures as json

	memset(print_buf, 0, sizeof(print_buf));
	len = libiotrace_struct_print_basic(print_buf, sizeof(print_buf), &data);

	// check print of basic structure with substructures

	CU_ASSERT_FATAL(sizeof(print_buf) >= len);
	CU_ASSERT_FATAL(print_buf[0] == '{');
	CU_ASSERT_FATAL(print_buf[len - 1] == '}');
	check_basic_print(&data, print_buf, len);
}

CUNIT_CI_RUN("Suite_1", CUNIT_CI_TEST(test_struct_basic));
