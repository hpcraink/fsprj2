/**
 * Testing program for `scerb`
 *   Creates producer & consumer process, which use the data structure
 */
#include <errno.h>      /* NOTE: Already incl'd via `error.h` */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <time.h>

#include <semaphore.h>
#include <fcntl.h>

#ifdef __linux__
#  include <sys/wait.h>
#endif

#include "iface/scerb.h"
#include "../../../../test/common/error.h"


/* -- Globals / Consts -- */
#define SMO_NAME                            "/SHM_TEST"
#define BUFF_DATA_MIN_SIZE_BYTES            3300            // Will be round up to (multiple of) pagesize
#define TEST_STRUCTS_TO_INSERT              33




// TODO: offset pointer calculation for `lwrb`



// --------------- --------------- --------------- ---------------  Producer  --------------- --------------- --------------- ---------------
int run_producer(
        sem_t *sem_producer_buf_created, sem_t *sem_consumer_buf_attached, sem_t *sem_producer_finished,
        pid_t consumer_pid) {
/* --  1. Init rb  -- */
    DIE_WHEN_ERR( fnres_scerb_create_and_attach(SMO_NAME, BUFF_DATA_MIN_SIZE_BYTES) );
    puts("[PRODUCER] (1.) Inited rb");
    DIE_WHEN_ERRNO( sem_post(sem_producer_buf_created) );

/* --  2. Write data  -- */
    DIE_WHEN_ERRNO( sem_wait(sem_consumer_buf_attached) );
    puts("[PRODUCER] (2.) Inserting `fnres_scevent` structs ...");
{
    fnres_scevent *test_scevent = (fnres_scevent*)DIE_WHEN_ERRNO_VPTR( malloc(FNRES_SCEVENT_MAX_SIZE) );
    for (int i = 0; i < TEST_STRUCTS_TO_INSERT; i++) {
        test_scevent->fd = i;
        test_scevent->type = i % 2 ? OPEN : CLOSE;
        snprintf(test_scevent->filename, FILENAME_MAX, i % 2 ? "/dev/test__%d": "/var/tmp/this_is_just_a_test_file__%d", i);
        test_scevent->filename_len = strlen(test_scevent->filename) + 1;

        if (-2 == DIE_WHEN_ERR( fnres_scerb_offer(test_scevent ))) {
            printf("[PRODUCER] Insertion failed  --  Reached (`i`=%d) `BUFF_DATA_MIN_SIZE_BYTES` (=%d), stopping insertion !\n", i, BUFF_DATA_MIN_SIZE_BYTES);
            goto cleanup;
        }

        printf("(%d) Inserted following `fnres_scevent` struct: \n\t", i +1);
        fnres_scerb_debug_print_scevent(test_scevent, NULL); printf("\n\t-> ");
        fnres_scerb_debug_print_status(NULL);
    }
cleanup:
    free(test_scevent);
}

/* --  4. Unmap  -- */
    DIE_WHEN_ERR( fnres_scerb_detach(SMO_NAME) );
    puts("[PRODUCER] (3.) Detached rb");
    DIE_WHEN_ERRNO( sem_post(sem_producer_finished) );


/* NOTE: We don't wait 4 consumer -> use death of producer as signal to child that buffer was full ... */
    puts("[PRODUCER] Exiting ...");
    return 0;
}


// --------------- --------------- --------------- ---------------  Consumer  --------------- --------------- --------------- ---------------
int run_consumer(
        sem_t *sem_producer_buf_created, sem_t *sem_consumer_buf_attached, sem_t *sem_producer_finished) {
    DIE_WHEN_ERRNO( sem_wait(sem_producer_buf_created) );
    DIE_WHEN_ERR( fnres_scerb_attach(SMO_NAME) );
    fprintf(stderr, "[CONSUMER] (1.) Attached to buffer\n");
    DIE_WHEN_ERRNO( sem_post(sem_consumer_buf_attached) );

    /* --  3. Retrieve data  -- */
    fprintf(stderr, "[CONSUMER] (2.) Retrieving `fnres_scevent` structs ...\n");
{
    fnres_scevent *test_scevent = (fnres_scevent*)alloca(FNRES_SCEVENT_MAX_SIZE);
    for (int i = 0; i < TEST_STRUCTS_TO_INSERT; i++) {
        while(-2 == fnres_scerb_poll(test_scevent)) {
            fprintf(stderr, "Nothing 2 retrieve -> spinning ...\n");
            // nanosleep((const struct timespec[]){{0, 10000000L}}, NULL);     // Reduce spinning
            if (1 == getppid()) {
                LOG_WARN("[CONSUMER] Producer exited, hence nothing 2 consume will be produced");
                _exit(1);       // Otherwise, semaphore will prevent exit
            }
        }
        fprintf(stderr, "(%d) Retrieved `fnres_scevent` struct: \n\t", i +1);
        fnres_scerb_debug_print_scevent(test_scevent, stderr); fprintf(stderr, "\n\t-> ");
        fnres_scerb_debug_print_status(stderr);
    }
}

    /* --  4. Destroy rb  -- */
    DIE_WHEN_ERRNO( sem_wait(sem_producer_finished) );
    DIE_WHEN_ERR( fnres_scerb_destory_and_detach(SMO_NAME) );
    fprintf(stderr, "[CONSUMER] (3.) Destroyed rb\n");

    fprintf(stderr, "[CONSUMER] Exiting ...\n");
    return 0;
}



int main(void) {
/* SETUP: Semaphores */
#define SEM_PARENT_BUF_CREATED "/sem_producer_buf_created"
#define SEM_CHILD_BUF_ATTACHED "/sem_consumer_buf_attached"
#define SEM_PARENT_FINISHED    "/sem_producer_finished"
// Cleanup old semaphores w/ same name (from programs which, e.g. have crashed prior)  ==> Don't check 4 error (since named semaphore might not exist)
    sem_unlink(SEM_PARENT_BUF_CREATED);
    sem_unlink(SEM_CHILD_BUF_ATTACHED);
    sem_unlink(SEM_PARENT_FINISHED);

    sem_t *sem_producer_buf_created,        // Make sure consumer doesn't start buffer which isn't inited yet
          *sem_consumer_buf_attached,       // Make sure consumer is attached to buffer before producer writes  (to make sure ringbuffer is used concurrently)
          *sem_producer_finished;           // Make sure consumer doesn't destroy buffer before producer has finished
    if (SEM_FAILED == (sem_producer_buf_created = sem_open(SEM_PARENT_BUF_CREATED, O_CREAT, 0660, 0)) ||
        SEM_FAILED == (sem_consumer_buf_attached = sem_open(SEM_CHILD_BUF_ATTACHED, O_CREAT, 0660, 0)) ||
        SEM_FAILED == (sem_producer_finished = sem_open(SEM_PARENT_FINISHED, O_CREAT, 0660, 0))) {
        LOG_ERROR_AND_EXIT("`sem_open` failed: %s", strerror(errno));
    }


/* Run producer & consumer */
    pid_t consumer_pid;
    return ((consumer_pid = DIE_WHEN_ERRNO( fork() ))) ?
        run_producer(sem_producer_buf_created, sem_consumer_buf_attached, sem_producer_finished, consumer_pid) :        // Parent
        run_consumer(sem_producer_buf_created, sem_consumer_buf_attached, sem_producer_finished);                       // Child
}
