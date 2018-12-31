/*
 * Test to test for atomic_add
 */
#include "CUnit/CUnitCI.h"

#include "libiotrace_config.h"

#ifdef HAVE_STDINT_H
#  include <stdint.h>
#endif

#include "atomic.h"

int32_t globalx = 42;
int64_t globaly = 4711;
int64_t globalz = -7;

static void test_atomic_add(void) {
    int32_t x = 42;
    int64_t y = 4711;
    int64_t z = -7;
    // Atomic_add returns the value BEFORE the operation
    CU_ASSERT_FATAL(atomic_add32(&x, 1) == 42);
    CU_ASSERT_FATAL(atomic_add32(&x, -1) == 43);
    CU_ASSERT_FATAL(atomic_add64(&y, 1) == 4711ll);
    CU_ASSERT_FATAL(atomic_add64(&y, -1) == 4712ll);
    CU_ASSERT_FATAL(atomic_add(&z, 1) == -7ll);
    CU_ASSERT_FATAL(atomic_add(&z, -1) == -6ll);
    CU_ASSERT_FATAL(atomic_add32(&globalx, 1) == 42);
    CU_ASSERT_FATAL(atomic_add32(&globalx, -1) == 43);
    CU_ASSERT_FATAL(atomic_add64(&globaly, 1) == 4711ll);
    CU_ASSERT_FATAL(atomic_add64(&globaly, -1) == 4712ll);
    CU_ASSERT_FATAL(atomic_add(&globalz, 1) == -7ll);
    CU_ASSERT_FATAL(atomic_add(&globalz, -1) == -6ll);
}

CUNIT_CI_RUN("atomic_add",
             CUNIT_CI_TEST(test_atomic_add));
