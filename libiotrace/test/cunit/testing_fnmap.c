/**
 * Used solely for validating/testing the behaviour of the map implementation (currently `atomic_hash`) via the `fnmap` iface
 * Doesn't test any other functionality of the `fnres` module
 */

#include <string.h>
#include <stdint.h>

#include "CUnit/CUnitCI.h"

#include "../../src/fnres/internal/fnmap.h"
#include "../../src/libiotrace_include_struct.h"


/* -- Constants -- */
#define DEFAULT_FNMAP_MAX_SIZE 100

/* -- Globals -- */
// int64_t next_dummy_id = 3;     /* Avoid colliding ids */


/* -- Hooks -- */
/* run at the start of the suite */
CU_SUITE_SETUP() {
    fnmap_create(DEFAULT_FNMAP_MAX_SIZE);

	return CUE_SUCCESS;
}

/* run at the end of the suite */
CU_SUITE_TEARDOWN() {
    fnmap_destroy();            /* Note: As mentioned in 'fctevent.c', 'fnres_fin' doesn't clean up dyn. allocated filenames (i.e., leaks memory) */

	return CUE_SUCCESS;
}

/* run at the start of each test */
CU_TEST_SETUP() {}

/* run at the end of each test */
CU_TEST_TEARDOWN() {}


/* -- Tests -- */
void test_same_value(void) {
    fnmap_key key = {
        .id = { .fildes = 0 },
        .type = F_DESCRIPTOR,
        .mmap_length = 0
    };

    for (int i = 0; i <= 30; i++) {             // Failure cause: `i <=` --> Too many entries for fnmap
        // printf("Added nr. %d\n", i);
        fnmap_add_or_update(&key, "_TEST_");
        // key.id.fildes++;
    }

    CU_ASSERT_FATAL(false);
}




CUNIT_CI_RUN("Suite_1",
             CUNIT_CI_TEST(test_same_value))
