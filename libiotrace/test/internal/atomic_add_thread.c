/*
 * Test to test for atomic_add
 */
#include "CUnit/CUnitCI.h"

#include "libiotrace_config.h"

#ifdef HAVE_STDINT_H
#  include <stdint.h>
#endif
#ifdef HAVE_PTHREAD_H
#  include <pthread.h>
#endif

#include "atomic.h"

int64_t global = 0;

#include "thread_test.h"

static void * test_thread (void * arg ATTRIBUTE_UNUSED) {
    int i;
    for (i = 0; i < NUM_ITERATIONS; i++) {
        atomic_add(&global, 1);
    }
    return NULL;
}

static void test_atomic_add_thread(void) {
    test_thread_wait();
    CU_ASSERT(global == NUM_THREADS * NUM_ITERATIONS);
}

CUNIT_CI_RUN("atomic_add_thread",
             CUNIT_CI_TEST(test_atomic_add_thread))
