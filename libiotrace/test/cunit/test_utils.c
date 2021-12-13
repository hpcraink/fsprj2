#include <string.h>
#include <unistd.h>

#include "CUnit/CUnitCI.h"
#include "../../src/utils.h"

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

static void test_generate_env(void) {
	const char key[] = "test_key";
	const char value[] = "test_value";
	char env[sizeof(key) + sizeof(value)]; // strlen(key) + strlen(value) + 2 for '=' and '\0'

	generate_env(env, key, sizeof(key) - 1, value);

	CU_ASSERT_FATAL(0 == strncmp(env, key, sizeof(key) - 1));
	CU_ASSERT_FATAL('=' == env[sizeof(key) - 1]);
	CU_ASSERT_FATAL(0 == strncmp(env + sizeof(key), value, sizeof(value) - 1));
	CU_ASSERT_FATAL('\0' == env[sizeof(key) + sizeof(value) - 1]);
}

static void test_read_line(void) {
	char buf[50];
	char *pos;
	char *next;

	// empty string
	buf[0] = '\0';
	pos = buf;
	CU_ASSERT_FATAL(NULL == read_line(buf, sizeof(buf), &pos));

	// string without line break (one line to read)
	char test_string1[] = "test";
	strcpy(buf, test_string1);
	pos = buf;
	next = read_line(buf, sizeof(buf), &pos);
	CU_ASSERT_FATAL(0 == strcmp(next, test_string1));
	CU_ASSERT_FATAL(NULL == read_line(buf, sizeof(buf), &pos));

	// string with \n and \r\n as line break
	char test_string2[] = "another test";
	char test_string3[] = "last test";
	strcpy(buf, test_string1);
	strcpy(buf + sizeof(test_string1) - 1, "\n");
	strcpy(buf + sizeof(test_string1), test_string2);
	strcpy(buf + sizeof(test_string1) + sizeof(test_string2) - 1, "\r\n");
	strcpy(buf + sizeof(test_string1) + sizeof(test_string2) + 1, test_string3);
	pos = buf;
	next = read_line(buf, sizeof(buf), &pos);
	CU_ASSERT_FATAL(0 == strcmp(next, test_string1));
	next = read_line(buf, sizeof(buf), &pos);
	CU_ASSERT_FATAL(0 == strcmp(next, test_string2));
	next = read_line(buf, sizeof(buf), &pos);
	CU_ASSERT_FATAL(0 == strcmp(next, test_string3));
	CU_ASSERT_FATAL(NULL == read_line(buf, sizeof(buf), &pos));

	// string with line break at the end
	strcpy(buf, test_string1);
	strcpy(buf + sizeof(test_string1) - 1, "\n");
	pos = buf;
	next = read_line(buf, sizeof(buf), &pos);
	CU_ASSERT_FATAL(0 == strcmp(next, test_string1));
	CU_ASSERT_FATAL(NULL == read_line(buf, sizeof(buf), &pos));

	// string with line break at the end and buffer size without terminating '\0'
	strcpy(buf, test_string1);
	strcpy(buf + sizeof(test_string1) - 1, "\n");
	pos = buf;
	next = read_line(buf, sizeof(test_string1), &pos);
	CU_ASSERT_FATAL(0 == strcmp(next, test_string1));
	CU_ASSERT_FATAL(NULL == read_line(buf, sizeof(test_string1), &pos));
}

static void test_shorten_log_name(void) {
	char one[] = "";
	char two[] = "0";
	char fourty_nine[] = "012345678901234567890123456789012345678901234567";
	char fifty[] = "0123456789012345678901234567890123456789012345678";
	char fifty_one[] = "01234567890123456789012345678901234567890123456789";

	char shortened[50];

	memset(shortened, -1, sizeof(shortened));
	shorten_log_name(shortened, sizeof(shortened), one, sizeof(one));
	CU_ASSERT_FATAL(0 == strcmp(one, shortened));

	memset(shortened, -1, sizeof(shortened));
	shorten_log_name(shortened, sizeof(shortened), two, sizeof(two));
	CU_ASSERT_FATAL(0 == strcmp(two, shortened));

	memset(shortened, -1, sizeof(shortened));
	shorten_log_name(shortened, sizeof(shortened), fourty_nine,
			sizeof(fourty_nine));
	CU_ASSERT_FATAL(0 == strcmp(fourty_nine, shortened));

	memset(shortened, -1, sizeof(shortened));
	shorten_log_name(shortened, sizeof(shortened), fifty, sizeof(fifty));
	CU_ASSERT_FATAL(0 == strcmp(fifty, shortened));

	memset(shortened, -1, sizeof(shortened));
	shorten_log_name(shortened, sizeof(shortened), fifty_one,
			sizeof(fifty_one));
	CU_ASSERT_FATAL(0 == strcmp(fifty_one + 1, shortened));
}

#define GETTIME_COUNT 10
static void test_gettime(void) {
	volatile u_int64_t time_old = 0;
	volatile u_int64_t time_new;

	for (int i = 0; i < GETTIME_COUNT; i++) {
		time_new = gettime();
		CU_ASSERT_FATAL(time_new >= time_old);
		time_old = time_new;
	}

	for (int i = 0; i < GETTIME_COUNT; i++) {
		sleep(1);
		time_new = gettime();
		CU_ASSERT_FATAL(time_new > time_old);
		time_old = time_new;
	}
}

CUNIT_CI_RUN("Suite_1", CUNIT_CI_TEST(test_generate_env),
		CUNIT_CI_TEST(test_read_line), CUNIT_CI_TEST(test_shorten_log_name),
		CUNIT_CI_TEST(test_gettime))
