#include <string.h>

#include "CUnit/CUnitCI.h"

#define JSMN_STRICT
#include "../../src/libs/jsmn.h"

#include "../../src/libs/line_protocol_parser.h"

/* libiotrace_include_function.h uses POSIX functions. The functions
 * are wrapped if WITH_POSIX_IO is defined. In this test file only
 * the structures and no wrappers are tested. So use of wrappers is
 * disabled via undef of WITH_POSIX_IO. It's the same for
 * WITH_ALLOC */
#undef WITH_POSIX_IO
#undef WITH_ALLOC
#include "../../src/libiotrace_include_struct.h"
#include "../../src/libiotrace_functions.c"

pid_t pid = 0;
ATTRIBUTE_THREAD pid_t tid = 0;

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

static void check_fd_set_copy(const fd_set *data, const fd_set *copy) {
    for (int i = 0; i < FD_SETSIZE; i++) {
        CU_ASSERT_FATAL(FD_ISSET(i, data) == FD_ISSET(i, copy));
    }
}

static void check_fcntl_seal_copy(const struct fcntl_seal *data,
        const struct fcntl_seal *copy) {
    CU_ASSERT_FATAL(data->flags.seal_grow == copy->flags.seal_grow);
    CU_ASSERT_FATAL(data->flags.seal_seal == copy->flags.seal_seal);
    CU_ASSERT_FATAL(data->flags.seal_shrink == copy->flags.seal_shrink);
    CU_ASSERT_FATAL(data->flags.seal_write == copy->flags.seal_write);
}

static void check_mpi_delete_function_copy(
        const struct mpi_delete_function *data,
        const struct mpi_delete_function *copy) {
    CU_ASSERT_FATAL(data->file_name != copy->file_name);
    CU_ASSERT_FATAL(0 == strcmp(data->file_name, copy->file_name));
    CU_ASSERT_FATAL(data->id.device_id == copy->id.device_id);
    CU_ASSERT_FATAL(data->id.inode_nr == copy->id.inode_nr);
    if (0 == data->__size_file_hints) {
        CU_ASSERT_FATAL(copy->__size_file_hints == 0);
    } else {
        for (size_t i = 0; i < data->__size_file_hints; i++) {
            CU_ASSERT_FATAL(
                    data->__keys_file_hints[i] != copy->__keys_file_hints[i]);
            CU_ASSERT_FATAL(
                    0
                            == strcmp(data->__keys_file_hints[i],
                                    copy->__keys_file_hints[i]));
            CU_ASSERT_FATAL(
                    data->__values_file_hints[i]
                            != copy->__values_file_hints[i]);
            CU_ASSERT_FATAL(
                    0
                            == strcmp(data->__values_file_hints[i],
                                    copy->__values_file_hints[i]));
        }
    }
}

static void check_select_function_copy(const struct select_function *data,
        const struct select_function *copy) {
    CU_ASSERT_FATAL(data->timeout.sec == copy->timeout.sec);
    CU_ASSERT_FATAL(data->timeout.micro_sec == copy->timeout.micro_sec);
    check_fd_set_copy(data->files_waiting_for_read,
            copy->files_waiting_for_read);
    check_fd_set_copy(data->files_waiting_for_write,
            copy->files_waiting_for_write);
    check_fd_set_copy(data->files_waiting_for_except,
            copy->files_waiting_for_except);
    check_fd_set_copy(data->files_ready_for_read, copy->files_ready_for_read);
    check_fd_set_copy(data->files_ready_for_write, copy->files_ready_for_write);
    check_fd_set_copy(data->files_ready_for_except,
            copy->files_ready_for_except);
}

static void check_fcntl_hint_copy(const struct fcntl_hint *data,
        const struct fcntl_hint *copy) {
    CU_ASSERT_FATAL(data->hint == copy->hint);
}

static void check_msg_function_copy(const struct msg_function *data,
        const struct msg_function *copy) {
    CU_ASSERT_FATAL(data->sockaddr != copy->sockaddr);
    CU_ASSERT_FATAL(data->sockaddr->family == copy->sockaddr->family);
    CU_ASSERT_FATAL(data->sockaddr->address != copy->sockaddr->address);
    CU_ASSERT_FATAL(
            0 == strcmp(data->sockaddr->address, copy->sockaddr->address));
    CU_ASSERT_FATAL(data->__size_descriptors == copy->__size_descriptors);
    for (size_t i = 0; i < data->__size_descriptors; i++) {
        CU_ASSERT_FATAL(data->__descriptors[i] == copy->__descriptors[i]);
    }
}

static void check_mpi_waitall_copy(const struct mpi_waitall *data,
        const struct mpi_waitall *copy) {
    if (NULL == data->__requests) {
        CU_ASSERT_FATAL(NULL == copy->__requests);
    } else {
        CU_ASSERT_FATAL(data->__requests != copy->__requests);
        CU_ASSERT_FATAL(data->__size_requests == copy->__size_requests);

        for (size_t i = 0; i < data->__size_requests; i++) {
            CU_ASSERT_FATAL(data->__requests[i] != copy->__requests[i]);
            CU_ASSERT_FATAL(
                    data->__requests[i]->count_bytes
                            == copy->__requests[i]->count_bytes);
            CU_ASSERT_FATAL(
                    data->__requests[i]->request_id
                            == copy->__requests[i]->request_id);
            CU_ASSERT_FATAL(
                    data->__requests[i]->return_state
                            == copy->__requests[i]->return_state);
            CU_ASSERT_FATAL(
                    data->__requests[i]->return_state_detail
                            != copy->__requests[i]->return_state_detail);
            CU_ASSERT_FATAL(
                    0
                            == strcmp(
                                    data->__requests[i]->return_state_detail->errno_text,
                                    copy->__requests[i]->return_state_detail->errno_text));
            CU_ASSERT_FATAL(
                    data->__requests[i]->return_state_detail->errno_value
                            == copy->__requests[i]->return_state_detail->errno_value);
        }
    }
}

static void check_basic_copy(const struct basic *data, const struct basic *copy) {
    CU_ASSERT_FATAL(data->hostname != copy->hostname);
    CU_ASSERT_FATAL(0 == strcmp(data->hostname, copy->hostname));
    CU_ASSERT_FATAL(data->pid == copy->pid);
    CU_ASSERT_FATAL(data->tid == copy->tid);
    CU_ASSERT_FATAL(0 == strcmp(data->function_name, copy->function_name));
    CU_ASSERT_FATAL(data->time_start == copy->time_start);
    CU_ASSERT_FATAL(data->time_end == copy->time_end);
#ifdef IOTRACE_ENABLE_INFLUXDB
    CU_ASSERT_FATAL(data->time_diff == copy->time_diff);
#endif
    CU_ASSERT_FATAL(data->return_state == copy->return_state);
    if (NULL != data->return_state_detail) {
        CU_ASSERT_FATAL(data->return_state_detail != copy->return_state_detail);
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
    }
    if (NULL == data->__stacktrace_symbols) {
        CU_ASSERT_FATAL(NULL == copy->__stacktrace_symbols);
    } else {
        for (size_t i = copy->__start_stacktrace_symbols;
                i < copy->__size_stacktrace_symbols; i++) {
            CU_ASSERT_FATAL(
                    data->__stacktrace_symbols[i]
                            != copy->__stacktrace_symbols[i]);
            CU_ASSERT_FATAL(
                    0
                            == strcmp(data->__stacktrace_symbols[i],
                                    copy->__stacktrace_symbols[i]));
        }
    }
    if (NULL == data->__stacktrace_pointer) {
        CU_ASSERT_FATAL(NULL == copy->__stacktrace_pointer);
    }
#ifdef LOG_WRAPPER_TIME
    CU_ASSERT_FATAL(data->wrapper.time_start == copy->wrapper.time_start);
    CU_ASSERT_FATAL(data->wrapper.time_end == copy->wrapper.time_end);
#endif
    if (NULL == data->__file_type) {
        CU_ASSERT_FATAL(NULL == copy->__file_type);
    } else {
        CU_ASSERT_FATAL(
                data->__void_p_enum_file_type == copy->__void_p_enum_file_type);
        if (__void_p_enum_file_type_file_descriptor
                == data->__void_p_enum_file_type) {
            CU_ASSERT_FATAL(
                    ((struct file_descriptor* )(data->__file_type))->descriptor
                            == ((struct file_descriptor* )(copy->__file_type))->descriptor);
        }
    }
    if (NULL == data->__function_data) {
        CU_ASSERT_FATAL(NULL == copy->__function_data);
    } else {
        CU_ASSERT_FATAL(
                data->__void_p_enum_function_data
                        == copy->__void_p_enum_function_data);
        if (__void_p_enum_function_data_mpi_waitall
                == data->__void_p_enum_function_data) {
            CU_ASSERT_FATAL(
                    ((struct mpi_waitall* )(data->__function_data))->__requests
                            == ((struct mpi_waitall* )(copy->__function_data))->__requests);
        }
    }
}

void check_line_push(char line_buf[], const size_t line_buf_size,
        const size_t count_char_in_line_buf) {
    // prepend measurement
    line_buf[0] = 't';
    line_buf[1] = 'e';
    line_buf[2] = 's';
    line_buf[3] = 't';
    line_buf[4] = ' ';

    CU_ASSERT_FATAL(line_buf_size >= count_char_in_line_buf + 5);
    CU_ASSERT_FATAL(line_buf[count_char_in_line_buf + 5] == '\0');
    CU_ASSERT_FATAL(strlen(line_buf) == count_char_in_line_buf + 5);
    if (count_char_in_line_buf > 5) {
        // remove last comma
        line_buf[count_char_in_line_buf + 4] = '\0';
    }
}

void check_json_print(const char print_buf[], const size_t print_buf_size,
        const size_t count_char_in_print_buf) {
    CU_ASSERT_FATAL(print_buf_size >= count_char_in_print_buf);
    CU_ASSERT_FATAL(print_buf[count_char_in_print_buf] == '\0');
    CU_ASSERT_FATAL(strlen(print_buf) == count_char_in_print_buf);
    CU_ASSERT_FATAL(print_buf[0] == '{');
    CU_ASSERT_FATAL(print_buf[count_char_in_print_buf - 1] == '}');
}

static void check_json_string(const char *print_buf, const jsmntok_t *token,
        const char *string) {
    CU_ASSERT_FATAL(token->type == JSMN_STRING);
    CU_ASSERT_FATAL(token->start >= 0);
    CU_ASSERT_FATAL(token->end >= 0);
    CU_ASSERT_FATAL(token->start <= token->end);
    CU_ASSERT_FATAL(
            strlen(string) == (size_t )token->end - (size_t )token->start);
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

static void check_json_sa_family_t(const char *print_buf,
        const jsmntok_t *token, sa_family_t family) {
    check_json_number(print_buf, token, family, 1);
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

size_t check_fd_set(const char *print_buf, const jsmntok_t *tokens,
        const fd_set *fd_set_data, const char *fd_set_name) {
    size_t i = 0;

    if (NULL != fd_set_data) {

        check_json_string(print_buf, &tokens[i++], fd_set_name);
        CU_ASSERT_FATAL(tokens[i++].type == JSMN_ARRAY);

        size_t count_fd = 0;
        for (size_t l = 0; l < FD_SETSIZE; l++) {
            if (FD_ISSET(l, fd_set_data)) {
                check_json_number(print_buf, &tokens[i++], l, 1);
                count_fd++;
            }
        }

        CU_ASSERT_FATAL((size_t )(tokens[1].size) == count_fd);
    }

    return i;
}

static void check_mpi_delete_function_print(
        const struct mpi_delete_function *data, const char *print_buf,
        const int len) {
    jsmn_parser parser;
    jsmn_init(&parser);
    int token_count = jsmn_parse(&parser, print_buf, len, NULL, 0);

    CU_ASSERT_FATAL(0 < token_count);

    jsmntok_t tokens[token_count];
    jsmn_init(&parser);
    int ret = jsmn_parse(&parser, print_buf, len, tokens, token_count);

    CU_ASSERT_FATAL(token_count == ret);

    size_t i = 0;
    CU_ASSERT_FATAL(tokens[i++].type == JSMN_OBJECT);

    if (0 == data->__size_file_hints) {
        CU_ASSERT_FATAL((size_t )(tokens[i - 1].size) == 2);
    } else {
        CU_ASSERT_FATAL((size_t )(tokens[i - 1].size) == 3);
    }

    check_json_string(print_buf, &tokens[i++], "file_name");
    check_json_string(print_buf, &tokens[i++], data->file_name);
    check_json_string(print_buf, &tokens[i++], "id");
    CU_ASSERT_FATAL(tokens[i++].type == JSMN_OBJECT);
    CU_ASSERT_FATAL((size_t )(tokens[i - 1].size) == 2);
    check_json_string(print_buf, &tokens[i++], "device_id");
    check_json_number(print_buf, &tokens[i++], data->id.device_id, 1);
    check_json_string(print_buf, &tokens[i++], "inode_nr");
    check_json_number(print_buf, &tokens[i++], data->id.inode_nr, 1);

    if (0 != data->__size_file_hints) {
        check_json_string(print_buf, &tokens[i++], "file_hints");
        CU_ASSERT_FATAL(tokens[i++].type == JSMN_OBJECT);
        CU_ASSERT_FATAL(
                (size_t )(tokens[i - 1].size) == data->__size_file_hints);

        for (size_t l = 0; l < data->__size_file_hints; l++) {
            check_json_string(print_buf, &tokens[i++],
                    data->__keys_file_hints[l]);
            check_json_string(print_buf, &tokens[i++],
                    data->__values_file_hints[l]);
        }
    }
}

static void check_select_function_print(const struct select_function *data,
        const char *print_buf, const int len) {
    jsmn_parser parser;
    jsmn_init(&parser);
    int token_count = jsmn_parse(&parser, print_buf, len, NULL, 0);

    CU_ASSERT_FATAL(0 < token_count);

    jsmntok_t tokens[token_count];
    jsmn_init(&parser);
    int ret = jsmn_parse(&parser, print_buf, len, tokens, token_count);

    CU_ASSERT_FATAL(token_count == ret);

    size_t i = 0;
    CU_ASSERT_FATAL(tokens[i++].type == JSMN_OBJECT);

    CU_ASSERT_FATAL((size_t )(tokens[i - 1].size) == 7);

    check_json_string(print_buf, &tokens[i++], "timeout");
    CU_ASSERT_FATAL(tokens[i++].type == JSMN_OBJECT);
    check_json_string(print_buf, &tokens[i++], "sec");
    check_json_number(print_buf, &tokens[i++], data->timeout.sec, 0);
    check_json_string(print_buf, &tokens[i++], "micro_sec");
    check_json_number(print_buf, &tokens[i++], data->timeout.sec, 0);

    i += check_fd_set(print_buf, &tokens[i], data->files_waiting_for_read,
            "files_waiting_for_read");
    i += check_fd_set(print_buf, &tokens[i], data->files_waiting_for_write,
            "files_waiting_for_write");
    i += check_fd_set(print_buf, &tokens[i], data->files_waiting_for_except,
            "files_waiting_for_except");
    i += check_fd_set(print_buf, &tokens[i], data->files_ready_for_read,
            "files_ready_for_read");
    i += check_fd_set(print_buf, &tokens[i], data->files_ready_for_write,
            "files_ready_for_write");
    i += check_fd_set(print_buf, &tokens[i], data->files_ready_for_except,
            "files_ready_for_except");
}

static void check_fcntl_seal_print(const struct fcntl_seal *data,
        const char *print_buf, const int len) {
    jsmn_parser parser;
    jsmn_init(&parser);
    int token_count = jsmn_parse(&parser, print_buf, len, NULL, 0);

    CU_ASSERT_FATAL(0 < token_count);

    jsmntok_t tokens[token_count];
    jsmn_init(&parser);
    int ret = jsmn_parse(&parser, print_buf, len, tokens, token_count);

    CU_ASSERT_FATAL(token_count == ret);

    size_t i = 0;
    CU_ASSERT_FATAL(tokens[i++].type == JSMN_OBJECT);

    CU_ASSERT_FATAL((size_t )(tokens[i - 1].size) == 1);

    check_json_string(print_buf, &tokens[i++], "flags");
    CU_ASSERT_FATAL(tokens[i++].type == JSMN_ARRAY);

    size_t count_array_elements = 0;
    if (data->flags.seal_seal) {
        check_json_string(print_buf, &tokens[i++], "seal_seal");
        count_array_elements++;
    }
    if (data->flags.seal_shrink) {
        check_json_string(print_buf, &tokens[i++], "seal_shrink");
        count_array_elements++;
    }
    if (data->flags.seal_grow) {
        check_json_string(print_buf, &tokens[i++], "seal_grow");
        count_array_elements++;
    }
    if (data->flags.seal_write) {
        check_json_string(print_buf, &tokens[i++], "seal_write");
        count_array_elements++;
    }
    CU_ASSERT_FATAL(
            (size_t )(tokens[i - 1 - count_array_elements].size)
                    == count_array_elements);
}

static void check_fcntl_hint_print(const char *hint_as_str,
        const char *print_buf, const int len) {
    jsmn_parser parser;
    jsmn_init(&parser);
    int token_count = jsmn_parse(&parser, print_buf, len, NULL, 0);

    CU_ASSERT_FATAL(0 < token_count);

    jsmntok_t tokens[token_count];
    jsmn_init(&parser);
    int ret = jsmn_parse(&parser, print_buf, len, tokens, token_count);

    CU_ASSERT_FATAL(token_count == ret);

    size_t i = 0;
    CU_ASSERT_FATAL(tokens[i++].type == JSMN_OBJECT);

    CU_ASSERT_FATAL((size_t )(tokens[i - 1].size) == 1);

    check_json_string(print_buf, &tokens[i++], "hint");
    check_json_string(print_buf, &tokens[i++], hint_as_str);
}

static void check_msg_function_print(const struct msg_function *data,
        const char *print_buf, const int len) {
    jsmn_parser parser;
    jsmn_init(&parser);
    int token_count = jsmn_parse(&parser, print_buf, len, NULL, 0);

    CU_ASSERT_FATAL(0 < token_count);

    jsmntok_t tokens[token_count];
    jsmn_init(&parser);
    int ret = jsmn_parse(&parser, print_buf, len, tokens, token_count);

    CU_ASSERT_FATAL(token_count == ret);

    size_t i = 0;
    CU_ASSERT_FATAL(tokens[i++].type == JSMN_OBJECT);

    CU_ASSERT_FATAL((size_t )(tokens[i - 1].size) == 2);

    check_json_string(print_buf, &tokens[i++], "sockaddr");
    CU_ASSERT_FATAL(tokens[i++].type == JSMN_OBJECT);
    CU_ASSERT_FATAL((size_t )(tokens[i - 1].size) == 2);
    check_json_string(print_buf, &tokens[i++], "family");
    check_json_sa_family_t(print_buf, &tokens[i++], data->sockaddr->family);
    check_json_string(print_buf, &tokens[i++], "address");
    check_json_string(print_buf, &tokens[i++], data->sockaddr->address);

    check_json_string(print_buf, &tokens[i++], "descriptors");
    CU_ASSERT_FATAL(tokens[i++].type == JSMN_ARRAY);
    CU_ASSERT_FATAL((size_t )(tokens[i - 1].size) == MAX_MSG_FILE_DESCRIPTORS);
    for (int l = 0; l < MAX_MSG_FILE_DESCRIPTORS; l++) {
        check_json_number(print_buf, &tokens[i++], data->__descriptors[l], 0);
    }
}

static void check_mpi_waitall_print(const struct mpi_waitall *data,
        const char *print_buf, const int len) {
    jsmn_parser parser;
    jsmn_init(&parser);
    int token_count = jsmn_parse(&parser, print_buf, len, NULL, 0);

    CU_ASSERT_FATAL(0 < token_count);

    jsmntok_t tokens[token_count];
    jsmn_init(&parser);
    int ret = jsmn_parse(&parser, print_buf, len, tokens, token_count);

    CU_ASSERT_FATAL(token_count == ret);

    size_t i = 0;
    CU_ASSERT_FATAL(tokens[i++].type == JSMN_OBJECT);

    if (NULL == data->__requests) {
        CU_ASSERT_FATAL(tokens[i - 1].size == 0);
    } else {
        CU_ASSERT_FATAL((size_t )(tokens[i - 1].size) == 1);

        check_json_string(print_buf, &tokens[i++], "requests");
        CU_ASSERT_FATAL(tokens[i++].type == JSMN_ARRAY);
        CU_ASSERT_FATAL((size_t )(tokens[i - 1].size) == data->__size_requests);

        for (size_t l = 0; l < data->__size_requests; l++) {
            CU_ASSERT_FATAL(tokens[i++].type == JSMN_OBJECT);
            CU_ASSERT_FATAL(tokens[i - 1].size == 4);
            check_json_string(print_buf, &tokens[i++], "count_bytes");
            check_json_number(print_buf, &tokens[i++],
                    data->__requests[l]->count_bytes, 0);
            check_json_string(print_buf, &tokens[i++], "request_id");
            check_json_number(print_buf, &tokens[i++],
                    data->__requests[l]->request_id, 0);
            check_json_string(print_buf, &tokens[i++], "return_state");
            check_json_enum_read_write_state(print_buf, &tokens[i++],
                    data->__requests[l]->return_state);
            check_json_string(print_buf, &tokens[i++], "return_state_detail");
            CU_ASSERT_FATAL(tokens[i++].type == JSMN_OBJECT);
            CU_ASSERT_FATAL(tokens[i - 1].size == 2);
            check_json_string(print_buf, &tokens[i++], "errno_value");
            check_json_number(print_buf, &tokens[i++],
                    data->__requests[l]->return_state_detail->errno_value, 0);
            check_json_string(print_buf, &tokens[i++], "errno_text");
            check_json_string(print_buf, &tokens[i++],
                    data->__requests[l]->return_state_detail->errno_text);
        }
    }
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

    size_t i = 0;
    CU_ASSERT_FATAL(tokens[i++].type == JSMN_OBJECT);
#ifdef FILENAME_RESOLUTION_ENABLED
    check_json_string(print_buf, &tokens[i++], "traced_filename");
    check_json_string(print_buf, &tokens[i++], data->traced_filename);
#endif
    check_json_string(print_buf, &tokens[i++], "hostname");
    check_json_string(print_buf, &tokens[i++], data->hostname);
    check_json_string(print_buf, &tokens[i++], "pid");
    check_json_number(print_buf, &tokens[i++], data->pid, 0);
    check_json_string(print_buf, &tokens[i++], "tid");
    check_json_number(print_buf, &tokens[i++], data->tid, 0);
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
    if (NULL != data->return_state_detail) {
        check_json_string(print_buf, &tokens[i++], "return_state_detail");
        CU_ASSERT_FATAL(tokens[i++].type == JSMN_OBJECT);
        CU_ASSERT_FATAL(tokens[i - 1].size == 2);
        check_json_string(print_buf, &tokens[i++], "errno_value");
        check_json_number(print_buf, &tokens[i++],
                data->return_state_detail->errno_value, 0);
        check_json_string(print_buf, &tokens[i++], "errno_text");
        check_json_string(print_buf, &tokens[i++],
                data->return_state_detail->errno_text);
    }
    if (NULL != data->__stacktrace_symbols) {
        check_json_string(print_buf, &tokens[i++], "stacktrace_symbols");
        CU_ASSERT_FATAL(tokens[i++].type == JSMN_ARRAY);
        CU_ASSERT_FATAL(
                (size_t )tokens[i - 1].size
                        == data->__size_stacktrace_symbols
                                - data->__start_stacktrace_symbols);
        for (size_t l = data->__start_stacktrace_symbols;
                l < data->__size_stacktrace_symbols; l++) {
            check_json_string(print_buf, &tokens[i++],
                    data->__stacktrace_symbols[l]);
        }
    }
#ifdef LOG_WRAPPER_TIME
    check_json_string(print_buf, &tokens[i++], "wrapper");
    CU_ASSERT_FATAL(tokens[i++].type == JSMN_OBJECT);
    CU_ASSERT_FATAL(tokens[i-1].size == 2);
    check_json_string(print_buf, &tokens[i++], "time_start");
    check_json_number(print_buf, &tokens[i++], data->wrapper.time_start, 1);
    check_json_string(print_buf, &tokens[i++], "time_end");
    check_json_number(print_buf, &tokens[i++], data->wrapper.time_end, 1);
#endif
    if (NULL != data->__file_type) {
        check_json_string(print_buf, &tokens[i++], "file_type");
        CU_ASSERT_FATAL(tokens[i++].type == JSMN_OBJECT);
        if (__void_p_enum_file_type_file_descriptor
                == data->__void_p_enum_file_type) {
            CU_ASSERT_FATAL(tokens[i - 1].size == 1);
            check_json_string(print_buf, &tokens[i++], "descriptor");
            check_json_number(print_buf, &tokens[i++],
                    ((struct file_descriptor*) (data->__file_type))->descriptor,
                    0);
        }
    }
    if (NULL != data->__function_data) {
        check_json_string(print_buf, &tokens[i++], "function_data");
        CU_ASSERT_FATAL(tokens[i++].type == JSMN_OBJECT);
        if (__void_p_enum_function_data_mpi_waitall
                == data->__void_p_enum_function_data) {
            if (NULL
                    == ((struct mpi_waitall*) (data->__function_data))->__requests) {
                CU_ASSERT_FATAL(tokens[i - 1].size == 0);
            } else {
                CU_ASSERT_FATAL(tokens[i - 1].size == 1);
                check_json_string(print_buf, &tokens[i++], "requests");
            }
        }
    }
}

static void check_push_string(const char *string, struct LP_Item *item) {
    CU_ASSERT_FATAL(item->type == LP_STRING);
    CU_ASSERT_FATAL(0 == strcmp(string, item->value.s));
}

static void check_push_integer(const long long integer, struct LP_Item *item) {
    CU_ASSERT_FATAL(item->type == LP_INTEGER);
    CU_ASSERT_FATAL(integer == item->value.i);
}

static void check_push_unsigned_integer(const unsigned long long integer,
        struct LP_Item *item) {
    CU_ASSERT_FATAL(item->type == LP_UINTEGER);
    CU_ASSERT_FATAL(integer == item->value.u);
}

static void check_push_enum_read_write_state(struct LP_Item *item,
        enum read_write_state read_write_state) {
    CU_ASSERT_FATAL(item->type == LP_STRING);

    char buf[libiotrace_struct_push_max_size_enum_read_write_state() + 1];
    size_t len = libiotrace_struct_push_enum_read_write_state(buf, sizeof(buf),
            &read_write_state);

    // remove enclosing '"'
    buf[len - 1] = '\0';
    const char *start = buf + 1;

    CU_ASSERT_FATAL(0 == strcmp(item->value.s, start));
}

static void check_mpi_waitall_push(const struct mpi_waitall *data,
        const char *line_buf) {
    struct LP_Point *point;
    int status = 0;

    point = LP_parse_line(line_buf, &status);
    if (point == NULL) {
        if (8 == status) {
            // struct mpi_waitall has no field element because
            // LIBIOTRACE_STRUCT_STRUCT_ARRAY is not implemented
            // for line protocol
            // TODO: remove after STRUCT_ARRAY is implemented
            return;
        }
        printf("LP_parse_line status: %d\n", status);
        CU_FAIL_FATAL("LP_parse_line returned an error")
    }

    CU_ASSERT_FATAL(0 == strcmp("test", point->measurement));

    struct LP_Item *next_field = point->fields;
    if (NULL == data->__requests) {
        CU_ASSERT_FATAL(NULL == next_field);
    } else {
        CU_ASSERT_FATAL(NULL == next_field);
        // TODO:
//        for (size_t l = 0; l < data->__size_requests; l++) {
//            return_state_detail_errno_text
//            return_state_detail_errno_value
//            return_state
//            request_id
//            count_bytes
//        }
    }
}

static void check_basic_push(const struct basic *data, const char *line_buf) {
    struct LP_Point *point;
    int status = 0;

    point = LP_parse_line(line_buf, &status);
    if (point == NULL) {
        printf("LP_parse_line status: %d\n", status);
        CU_FAIL_FATAL("LP_parse_line returned an error")
    }

    CU_ASSERT_FATAL(0 == strcmp("test", point->measurement));

    struct LP_Item *next_field = point->fields;
    if (NULL != data->__function_data) {
        if (__void_p_enum_function_data_mpi_waitall
                == data->__void_p_enum_function_data) {
            if (NULL
                    != ((struct mpi_waitall*) (data->__function_data))->__requests) {
                CU_ASSERT_FATAL(
                        0 == strcmp("function_data_requests", next_field->key));
                next_field = next_field->next_item;
            }
        }
    }
    if (NULL != data->__file_type) {
        if (__void_p_enum_file_type_file_descriptor
                == data->__void_p_enum_file_type) {
            CU_ASSERT_FATAL(
                    0 == strcmp("file_type_descriptor", next_field->key));
            check_push_integer(
                    ((struct file_descriptor*) (data->__file_type))->descriptor,
                    next_field);
            next_field = next_field->next_item;
        }
    }
#ifdef LOG_WRAPPER_TIME
    CU_ASSERT_FATAL(0 == strcmp("wrapper_time_end", next_field->key));
    check_push_unsigned_integer(data->wrapper.time_start, next_field);
    next_field = next_field->next_item;
    CU_ASSERT_FATAL(0 == strcmp("wrapper_time_start", next_field->key));
    check_push_unsigned_integer(data->wrapper.time_start, next_field);
    next_field = next_field->next_item;
#endif
    if (NULL != data->return_state_detail) {
        CU_ASSERT_FATAL(
                0 == strcmp("return_state_detail_errno_text", next_field->key));
        check_push_string(data->return_state_detail->errno_text, next_field);
        next_field = next_field->next_item;
        CU_ASSERT_FATAL(
                0 == strcmp("return_state_detail_errno_value", next_field->key));
        check_push_integer(data->return_state_detail->errno_value, next_field);
        next_field = next_field->next_item;
    }
    CU_ASSERT_FATAL(0 == strcmp("return_state", next_field->key));
    check_push_enum_read_write_state(next_field, data->return_state);
    next_field = next_field->next_item;
#ifdef IOTRACE_ENABLE_INFLUXDB
    CU_ASSERT_FATAL(0 == strcmp("time_diff", next_field->key));
    check_push_unsigned_integer(data->time_diff, next_field);
    next_field = next_field->next_item;
#endif
    CU_ASSERT_FATAL(0 == strcmp("time_end", next_field->key));
    check_push_unsigned_integer(data->time_end, next_field);
    next_field = next_field->next_item;
    CU_ASSERT_FATAL(0 == strcmp("time_start", next_field->key));
    check_push_unsigned_integer(data->time_end, next_field);
    next_field = next_field->next_item;
    CU_ASSERT_FATAL(0 == strcmp("function_name", next_field->key));
    check_push_string(data->function_name, next_field);
    next_field = next_field->next_item;
    CU_ASSERT_FATAL(0 == strcmp("tid", next_field->key));
    check_push_integer(data->tid, next_field);
    next_field = next_field->next_item;
    CU_ASSERT_FATAL(0 == strcmp("pid", next_field->key));
    check_push_integer(data->pid, next_field);
    next_field = next_field->next_item;
    CU_ASSERT_FATAL(0 == strcmp("hostname", next_field->key));
    check_push_string(data->hostname, next_field);
    next_field = next_field->next_item;
#ifdef FILENAME_RESOLUTION_ENABLED
    CU_ASSERT_FATAL(0 == strcmp("traced_filename", next_field->key));
    check_push_string(data->traced_filename, next_field);
    next_field = next_field->next_item;
#endif

    CU_ASSERT_FATAL(NULL == next_field);

    LP_free_point(point);
}

static void test_struct_fcntl_seal_impl(struct fcntl_seal *fcntl_seal_data) {
    // copy fcntl_seal structure

    char copy_buf[libiotrace_struct_sizeof_fcntl_seal(fcntl_seal_data)];
    memset(copy_buf, 0, sizeof(copy_buf));
    void *pos = (void*) libiotrace_struct_copy_fcntl_seal((void*) copy_buf,
            fcntl_seal_data);
    struct fcntl_seal *copy = (struct fcntl_seal*) (void*) copy_buf;

    // check copy of fcntl_seal structure

    CU_ASSERT_FATAL(copy_buf + sizeof(copy_buf) == pos);
    check_fcntl_seal_copy(fcntl_seal_data, copy);

    // print fcntl_seal structure as json

    char print_buf[libiotrace_struct_max_size_fcntl_seal() + 1];
    memset(print_buf, 0, sizeof(print_buf));
    size_t len = libiotrace_struct_print_fcntl_seal(print_buf,
            sizeof(print_buf), copy);

    // check print of fcntl_seal structure

    check_json_print(print_buf, sizeof(print_buf), len);
    check_fcntl_seal_print(fcntl_seal_data, print_buf, len);
}

static void test_struct_mpi_delete_function_impl(
        struct mpi_delete_function *mpi_delete_function_data) {
    // copy mpi_delete_function structure

    char copy_buf[libiotrace_struct_sizeof_mpi_delete_function(
            mpi_delete_function_data)];
    memset(copy_buf, 0, sizeof(copy_buf));
    void *pos = (void*) libiotrace_struct_copy_mpi_delete_function(
            (void*) copy_buf, mpi_delete_function_data);
    struct mpi_delete_function *copy =
            (struct mpi_delete_function*) (void*) copy_buf;

    // check copy of mpi_delete_function structure

    CU_ASSERT_FATAL(copy_buf + sizeof(copy_buf) == pos);
    check_mpi_delete_function_copy(mpi_delete_function_data, copy);

    // print mpi_delete_function structure as json

    char print_buf[libiotrace_struct_max_size_mpi_delete_function() + 1];
    memset(print_buf, 0, sizeof(print_buf));
    size_t len = libiotrace_struct_print_mpi_delete_function(print_buf,
            sizeof(print_buf), copy);

    // check print of mpi_delete_function structure

    check_json_print(print_buf, sizeof(print_buf), len);
    check_mpi_delete_function_print(mpi_delete_function_data, print_buf, len);
}

/* struct mpi_delete_function has a LIBIOTRACE_STRUCT_KEY_VALUE_ARRAY element:
 * a key value array must be handled */
static void test_struct_mpi_delete_function(void) {
    struct mpi_delete_function mpi_delete_function_data;
    struct file_id file_id_data;
    char file_name[MAXFILENAME];
    dev_t dev_value;
    ino_t ino_t_value;
    char *keys[MAX_MPI_FILE_HINTS];
    char *values[MAX_MPI_FILE_HINTS];
    char key_value_content[MAX_MPI_FILE_HINT_LENGTH];

    // initialize mpi_delete_function structure without key value array

    fill_string(file_name, sizeof(file_name), 's');
    fill_number(&dev_value, sizeof(dev_value));
    fill_number(&ino_t_value, sizeof(ino_t_value));
    file_id_data.device_id = dev_value;
    file_id_data.inode_nr = ino_t_value;
    mpi_delete_function_data.file_name = file_name;
    mpi_delete_function_data.id = file_id_data;

    LIBIOTRACE_STRUCT_SET_KEY_VALUE_ARRAY_NULL(mpi_delete_function_data,
            file_hints)

    test_struct_mpi_delete_function_impl(&mpi_delete_function_data);

    // add one key value entry to mpi_delete_function structure

    fill_string(key_value_content, sizeof(key_value_content), 'k');

    LIBIOTRACE_STRUCT_INIT_KEY_VALUE_ARRAY(mpi_delete_function_data, file_hints,
            keys, values)
    LIBIOTRACE_STRUCT_ADD_KEY_VALUE(mpi_delete_function_data, file_hints,
            key_value_content, key_value_content)

    test_struct_mpi_delete_function_impl(&mpi_delete_function_data);

    // add max. key value pairs to mpi_delete_function structure

    LIBIOTRACE_STRUCT_INIT_KEY_VALUE_ARRAY(mpi_delete_function_data, file_hints,
            keys, values)
    for (int i = 0; i < MAX_MPI_FILE_HINTS; i++) {
        LIBIOTRACE_STRUCT_ADD_KEY_VALUE(mpi_delete_function_data, file_hints,
                key_value_content, key_value_content)
    }

    test_struct_mpi_delete_function_impl(&mpi_delete_function_data);
}

/* struct select_function has a LIBIOTRACE_STRUCT_FD_SET_P element:
 * a fd_set must be handled */
static void test_struct_select_function(void) {
    struct select_function select_function_data;
    long long_value;
    fd_set waiting_for_read;
    fd_set waiting_for_write;
    fd_set waiting_for_except;
    fd_set ready_for_read;
    fd_set ready_for_write;
    fd_set ready_for_except;

    // initialize select_function structure

    fill_number(&long_value, sizeof(long_value));
    select_function_data.timeout.sec = long_value;
    select_function_data.timeout.micro_sec = long_value;
    FD_ZERO(&waiting_for_read); // no file descriptor is set
    FD_ZERO(&waiting_for_write);
    FD_SET(0, &waiting_for_write); // only first file descriptor is set
    FD_ZERO(&waiting_for_except);
    FD_SET(FD_SETSIZE - 1, &waiting_for_except); // only last file descriptor is set
    FD_ZERO(&ready_for_read);
    for (int i = 0; i < FD_SETSIZE; i++) {
        FD_SET(i, &ready_for_read); // all file descriptors are set
    }
    FD_ZERO(&ready_for_write);
    for (int i = 1; i < FD_SETSIZE; i++) {
        FD_SET(i, &ready_for_write); // only first file descriptors is not set
    }
    FD_ZERO(&ready_for_except);
    for (int i = 0; i < (FD_SETSIZE - 1); i++) {
        FD_SET(i, &ready_for_except); // only last file descriptors is not set
    }
    select_function_data.files_waiting_for_read = &waiting_for_read;
    select_function_data.files_waiting_for_write = &waiting_for_write;
    select_function_data.files_waiting_for_except = &waiting_for_except;
    select_function_data.files_ready_for_read = &ready_for_read;
    select_function_data.files_ready_for_write = &ready_for_write;
    select_function_data.files_ready_for_except = &ready_for_except;

    // copy select_function structure

    char copy_buf[libiotrace_struct_sizeof_select_function(
            &select_function_data)];
    memset(copy_buf, 0, sizeof(copy_buf));
    void *pos = (void*) libiotrace_struct_copy_select_function((void*) copy_buf,
            &select_function_data);
    struct select_function *copy = (struct select_function*) (void*) copy_buf;

    // check copy of select_function structure

    CU_ASSERT_FATAL(copy_buf + sizeof(copy_buf) == pos);
    check_select_function_copy(&select_function_data, copy);

    // print select_function structure as json

    char print_buf[libiotrace_struct_max_size_select_function() + 1];
    memset(print_buf, 0, sizeof(print_buf));
    size_t len = libiotrace_struct_print_select_function(print_buf,
            sizeof(print_buf), copy);

    // check print of select_function structure

    check_json_print(print_buf, sizeof(print_buf), len);
    check_select_function_print(&select_function_data, print_buf, len);
}

/* struct fcntl_seal has a LIBIOTRACE_STRUCT_ARRAY_BITFIELD element:
 * a bitfield must be handled */
static void test_struct_fcntl_seal(void) {
    struct fcntl_seal fcntl_seal_data;

    // initialize fcntl_seal structure

    fcntl_seal_data.flags.seal_seal = 0;
    fcntl_seal_data.flags.seal_shrink = 0;
    fcntl_seal_data.flags.seal_grow = 0;
    fcntl_seal_data.flags.seal_write = 0;

    // test fcntl_seal structure

    test_struct_fcntl_seal_impl(&fcntl_seal_data);

    // initialize fcntl_seal structure

    fcntl_seal_data.flags.seal_seal = 1;
    fcntl_seal_data.flags.seal_shrink = 0;
    fcntl_seal_data.flags.seal_grow = 1;
    fcntl_seal_data.flags.seal_write = 1;

    // test fcntl_seal structure

    test_struct_fcntl_seal_impl(&fcntl_seal_data);

    // initialize fcntl_seal structure

    fcntl_seal_data.flags.seal_seal = 0;
    fcntl_seal_data.flags.seal_shrink = 1;
    fcntl_seal_data.flags.seal_grow = 0;
    fcntl_seal_data.flags.seal_write = 0;

    // test fcntl_seal structure

    test_struct_fcntl_seal_impl(&fcntl_seal_data);
}

/* struct fcntl_hint has a LIBIOTRACE_STRUCT_ENUM element:
 * a enum must be handled */
static void test_struct_fcntl_hint(void) {
    struct fcntl_hint fcntl_hint_data;

    // initialize fcntl_hint structure

    fcntl_hint_data.hint = hint_write_life_not_set;

    // copy fcntl_hint structure

    char copy_buf[libiotrace_struct_sizeof_fcntl_hint(&fcntl_hint_data)];
    memset(copy_buf, 0, sizeof(copy_buf));
    void *pos = (void*) libiotrace_struct_copy_fcntl_hint((void*) copy_buf,
            &fcntl_hint_data);
    struct fcntl_hint *copy = (struct fcntl_hint*) (void*) copy_buf;

    // check copy of fcntl_hint structure

    CU_ASSERT_FATAL(copy_buf + sizeof(copy_buf) == pos);
    check_fcntl_hint_copy(&fcntl_hint_data, copy);

    // print fcntl_hint structure as json

    char print_buf[libiotrace_struct_max_size_fcntl_hint() + 1];
    memset(print_buf, 0, sizeof(print_buf));
    size_t len = libiotrace_struct_print_fcntl_hint(print_buf,
            sizeof(print_buf), copy);

    // check print of fcntl_hint structure

    check_json_print(print_buf, sizeof(print_buf), len);
    check_fcntl_hint_print("hint_write_life_not_set", print_buf, len);
}

/* struct msg_function has a LIBIOTRACE_STRUCT_INT_ARRAY element:
 * a array of integers must be handled */
static void test_struct_msg_function(void) {
    struct msg_function msg_function_data;
    struct sockaddr_function sockaddr_function_data;
    char address[MAX_SOCKADDR_LENGTH * 2 + 1];
    int descriptors[MAX_MSG_FILE_DESCRIPTORS];
    int int_value;

    // initialize msg_function structure

    fill_string(address, sizeof(address), 'a');
    fill_number(&int_value, sizeof(int_value));
    sockaddr_function_data.family = AF_UNIX;
    sockaddr_function_data.address = address;
    msg_function_data.sockaddr = &sockaddr_function_data;

    for (int i = 0; i < MAX_MSG_FILE_DESCRIPTORS; i++) {
        descriptors[i] = int_value;
    }
    LIBIOTRACE_STRUCT_SET_INT_ARRAY(msg_function_data, descriptors,
            (int* )descriptors, MAX_MSG_FILE_DESCRIPTORS)

    // copy msg_function structure

    char copy_buf[libiotrace_struct_sizeof_msg_function(&msg_function_data)];
    memset(copy_buf, 0, sizeof(copy_buf));
    void *pos = (void*) libiotrace_struct_copy_msg_function((void*) copy_buf,
            &msg_function_data);
    struct msg_function *copy = (struct msg_function*) (void*) copy_buf;

    // check copy of msg_function structure

    CU_ASSERT_FATAL(copy_buf + sizeof(copy_buf) == pos);
    check_msg_function_copy(&msg_function_data, copy);

    // print msg_function structure as json

    char print_buf[libiotrace_struct_max_size_msg_function() + 1];
    memset(print_buf, 0, sizeof(print_buf));
    size_t len = libiotrace_struct_print_msg_function(print_buf,
            sizeof(print_buf), copy);

    // check print of msg_function structure

    check_json_print(print_buf, sizeof(print_buf), len);
    check_msg_function_print(&msg_function_data, print_buf, len);
}

/* struct mpi_waitall has a LIBIOTRACE_STRUCT_STRUCT_ARRAY element:
 * a array of substructures must be handled */
static void test_struct_mpi_waitall(void) {
    struct mpi_waitall mpi_waitall_data;
    struct mpi_waitall_element mpi_waitall_element_data;
    struct errno_detail errno_detail_data;
    struct mpi_waitall_element *mpi_waitall_element_array[MAX_MPI_IMESSAGES];
    int int_value;
    char errno_text[MAX_ERROR_TEXT];

    // initialize mpi_waitall structure without substructures

    LIBIOTRACE_STRUCT_SET_STRUCT_ARRAY_NULL(mpi_waitall_data, requests)

    // copy mpi_waitall structure without substructures

    char copy_buf[libiotrace_struct_sizeof_mpi_waitall(&mpi_waitall_data)];
    memset(copy_buf, 0, sizeof(copy_buf));
    void *pos = (void*) libiotrace_struct_copy_mpi_waitall((void*) copy_buf,
            &mpi_waitall_data);
    struct mpi_waitall *copy = (struct mpi_waitall*) (void*) copy_buf;

    // check copy of mpi_waitall structure without substructures

    CU_ASSERT_FATAL(copy_buf + sizeof(copy_buf) == pos);
    check_mpi_waitall_copy(&mpi_waitall_data, copy);

    // print mpi_waitall structure without substructures as json

    char print_buf[libiotrace_struct_max_size_mpi_waitall() + 1];
    memset(print_buf, 0, sizeof(print_buf));
    size_t len = libiotrace_struct_print_mpi_waitall(print_buf,
            sizeof(print_buf), copy);

    // check print of mpi_waitall structure without substructures

    check_json_print(print_buf, sizeof(print_buf), len);
    check_mpi_waitall_print(&mpi_waitall_data, print_buf, len);

    // print mpi_waitall structure without substructures as influxdb line protocol (push)

    char line_buf[libiotrace_struct_push_max_size_mpi_waitall(0) + 1 + 5];
    memset(line_buf, 0, sizeof(line_buf));
    len = libiotrace_struct_push_mpi_waitall(line_buf + 5, sizeof(line_buf),
            copy, "");

    // check push of mpi_waitall structure without substructures

    check_line_push(line_buf, sizeof(line_buf), len);
    check_mpi_waitall_push(&mpi_waitall_data, line_buf);

    // add substructures to mpi_waitall structure

    fill_number(&int_value, sizeof(int_value));
    fill_string(errno_text, sizeof(errno_text), 'e');

    errno_detail_data.errno_value = int_value;
    errno_detail_data.errno_text = errno_text;

    mpi_waitall_element_data.count_bytes = int_value;
    mpi_waitall_element_data.request_id = int_value;
    mpi_waitall_element_data.return_state = unknown_read_write_state;
    mpi_waitall_element_data.return_state_detail = &errno_detail_data;

    for (int i = 0; i < MAX_MPI_IMESSAGES; i++) {
        mpi_waitall_element_array[i] = &mpi_waitall_element_data;
    }

    LIBIOTRACE_STRUCT_SET_STRUCT_ARRAY(mpi_waitall_data, requests,
            mpi_waitall_element_array, MAX_MPI_IMESSAGES)

    // copy mpi_waitall structure with substructures

    char copy_buf2[libiotrace_struct_sizeof_mpi_waitall(&mpi_waitall_data)];
    memset(copy_buf2, 0, sizeof(copy_buf2));
    pos = (void*) libiotrace_struct_copy_mpi_waitall((void*) copy_buf2,
            &mpi_waitall_data);
    copy = (struct mpi_waitall*) (void*) copy_buf2;

    // check copy of mpi_waitall structure with substructures

    CU_ASSERT_FATAL(copy_buf2 + sizeof(copy_buf2) == pos);
    check_mpi_waitall_copy(&mpi_waitall_data, copy);

    // print mpi_waitall structure with substructures as json

    memset(print_buf, 0, sizeof(print_buf));
    len = libiotrace_struct_print_mpi_waitall(print_buf, sizeof(print_buf),
            copy);

    // check print of mpi_waitall structure with substructures

    check_json_print(print_buf, sizeof(print_buf), len);
    check_mpi_waitall_print(&mpi_waitall_data, print_buf, len);
}

/* struct basic has LIBIOTRACE_STRUCT_VOID_P_START elements:
 * different substructures must be handled */
static void test_struct_basic(void) {
    struct basic data;
    char hostname[HOST_NAME_MAX];
    pid_t pid_t_value;
    u_int64_t u_int64_t_value;

    // initialize basic structure without substructures

    fill_string(hostname, sizeof(hostname), 'h');
    fill_number(&pid_t_value, sizeof(pid_t_value));
    fill_number(&u_int64_t_value, sizeof(u_int64_t_value));

#ifdef FILENAME_RESOLUTION_ENABLED
    fill_string(data.traced_filename, sizeof(data.traced_filename), 't');
#endif

    data.hostname = hostname;
    data.pid = pid_t_value;
    data.tid = pid_t_value;
    fill_string(data.function_name, sizeof(data.function_name), 'f');
    data.time_start = u_int64_t_value;
    data.time_end = u_int64_t_value;
#ifdef IOTRACE_ENABLE_INFLUXDB
    data.time_diff = u_int64_t_value;
#endif
    data.return_state = unknown_read_write_state;
    data.return_state_detail = NULL;
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

    char print_buf[libiotrace_struct_max_size_basic() + 1];
    memset(print_buf, 0, sizeof(print_buf));
    size_t len = libiotrace_struct_print_basic(print_buf, sizeof(print_buf),
            copy);

    // check print of basic structure without substructures

    check_json_print(print_buf, sizeof(print_buf), len);
    check_basic_print(&data, print_buf, len);

    // print basic structure without substructures as influxdb line protocol (push)

    char line_buf[libiotrace_struct_push_max_size_basic(0) + 1 + 5];
    memset(line_buf, 0, sizeof(line_buf));
    len = libiotrace_struct_push_basic(line_buf + 5, sizeof(line_buf), copy,
            "");

    // check push of basic structure without substructures

    check_line_push(line_buf, sizeof(line_buf), len);
    check_basic_push(&data, line_buf);

    // add substructures to basic structure

    char errno_text[MAX_ERROR_TEXT];
    int int_value;

    fill_string(errno_text, sizeof(errno_text), 'e');
    fill_number(&int_value, sizeof(int_value));

    struct errno_detail errno_detail_data;

    errno_detail_data.errno_value = int_value;
    errno_detail_data.errno_text = errno_text;

    data.return_state_detail = &errno_detail_data;

    int size = 10;
    char **messages = malloc(sizeof(char*) * size);
    CU_ASSERT_FATAL(NULL != messages)
    char tmp_string[size];
    fill_string(tmp_string, sizeof(tmp_string), 's');
    for (int i = 0; i < size; i++) {
        messages[i] = tmp_string;
    }
    LIBIOTRACE_STRUCT_SET_MALLOC_STRING_ARRAY(data, stacktrace_symbols,
            messages, 3, size)

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
    len = libiotrace_struct_print_basic(print_buf, sizeof(print_buf), copy);

    // check print of basic structure with substructures

    check_json_print(print_buf, sizeof(print_buf), len);
    check_basic_print(&data, print_buf, len);

    // print basic structure with substructures as influxdb line protocol (push)

    memset(line_buf, 0, sizeof(line_buf));
    len = libiotrace_struct_push_basic(line_buf + 5, sizeof(line_buf), copy,
            "");

    // check push of basic structure with substructures

    check_line_push(line_buf, sizeof(line_buf), len);
    check_basic_push(&data, line_buf);
}

// TODO: LIBIOTRACE_STRUCT_MALLOC_PTR_ARRAY

CUNIT_CI_RUN("Suite_1", CUNIT_CI_TEST(test_struct_basic),
        CUNIT_CI_TEST(test_struct_mpi_waitall),
        CUNIT_CI_TEST(test_struct_msg_function),
        CUNIT_CI_TEST(test_struct_fcntl_hint),
        CUNIT_CI_TEST(test_struct_fcntl_seal),
        CUNIT_CI_TEST(test_struct_select_function),
        CUNIT_CI_TEST(test_struct_mpi_delete_function))
