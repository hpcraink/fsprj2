#ifndef THREAD_TEST_H
#define THREAD_TEST_H

#define NUM_ITERATIONS 1000
#define NUM_THREADS    4
static pthread_t threads[NUM_THREADS];

/*
CU_SUITE_SETUP() {
    // Figure out the number of threads to start
    // Allocate the threads-array
    return CUE_SUCCESS;
}
*/

/* Each test has to define this function */
static void * test_thread (void * arg);

CU_TEST_SETUP() {
    int i;
    for (i = 0; i < NUM_THREADS; i++) {
        int ret = pthread_create(&threads[i], NULL, test_thread, NULL);
        CU_ASSERT_FATAL (0 == ret);
    }
}

/*
 * This function must be called before checking any any values
 */
static void test_thread_wait(void) {
    int i;
    for (i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL); // Don't care for return value;
    }    
}

CU_TEST_TEARDOWN() {
    // At Teardown we don't have to do anything
}

#endif /* THREAD_TEST_H */
