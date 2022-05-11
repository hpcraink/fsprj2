#include <string.h>
#include <unistd.h>

#include "CUnit/CUnitCI.h"
#include "../../src/common/gettime.h"

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

CUNIT_CI_RUN("Suite_2", CUNIT_CI_TEST(test_gettime))
