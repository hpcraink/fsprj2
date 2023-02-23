#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

#include "libiotrace_config.h"

static void* thread_start1(ATTRIBUTE_UNUSED void *arg) {
    printf("in thread1\n");

    return NULL;
}

static void* thread_start2(void *arg) {
    int *tmp = arg;

    printf("in thread2: address of tmp = %p\n", (void *)tmp);
    printf("in thread2: value of tmp =  %d\n", *tmp);

    return NULL;
}

int main(void) {
    pthread_t recv_thread;
    int tmp = 5;

    printf("start: address of tmp = %p\n", (void *)&tmp);

    int ret = pthread_create(&recv_thread, NULL, thread_start1, NULL);
    if (0 != ret) {
        printf("pthread_create() failed. (%d)\n", ret);
        return 1;
    }

    ret = pthread_create(&recv_thread, NULL, thread_start2, &tmp);
    if (0 != ret) {
        printf("pthread_create() failed. (%d)\n", ret);
        return 1;
    }

    sleep(1);

    printf("end\n");

    return 0;
}
