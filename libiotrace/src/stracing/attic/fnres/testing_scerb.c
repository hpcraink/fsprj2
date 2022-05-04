/**
 * Testing program for `scerb`
 *   Creates producer & consumer process, which use the data structure
 */
#include <errno.h>      /* NOTE: Already incl'd via `error.h` */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <semaphore.h>
#include <fcntl.h>

#ifdef __linux__
#  include <sys/wait.h>
#endif

#include "iface/libiotrace/scerb_consumer.h"
#include "iface/stracer/scerb_producer.h"

#include "../../../../test/common/error.h"


/* -- Globals / Consts -- */
#define SMO_NAME                            "/SHM_TEST"
#define TEST_STRUCTS_TO_INSERT              33





// --------------- --------------- --------------- ---------------  Producer  --------------- --------------- --------------- ---------------
int run_producer_aka_stracer(
        sem_t *sem_consumer_buf_created, sem_t *sem_consumer_buf_attached, sem_t *sem_consumer_finished,
        pid_t consumer_pid) {
    sm_scerb_t *sm_scerb;

/* --  2. Attach  -- */
    DIE_WHEN_ERRNO( sem_wait(sem_consumer_buf_created) );
    DIE_WHEN_ERR( fnres_scerb_attach_register_producer(&sm_scerb, SMO_NAME) );
    fprintf(stdout, "[PRODUCER] (2.) Attached to buffer\n");
    DIE_WHEN_ERRNO( sem_post(sem_consumer_buf_attached) );


/* --  3. Write data  -- */
    fprintf(stdout, "[PRODUCER] (3.) Inserting `fnres_scevent` structs ...\n");
{
    scevent_t *test_scevent = (scevent_t*)DIE_WHEN_ERRNO_VPTR( malloc(FNRES_SCEVENT_MAX_SIZE) );
    for (int i = 0; i < TEST_STRUCTS_TO_INSERT; i++) {
        test_scevent->fd = i;
        test_scevent->type = i % 2 ? OPEN : CLOSE;
        snprintf(test_scevent->filename, FILENAME_MAX, i % 2 ? "/dev/test__%d": "/var/tmp/this_is_just_a_test_file__%d", i);
        test_scevent->filename_len = strlen(test_scevent->filename) + 1;

        if (-2 == DIE_WHEN_ERR( fnres_scerb_offer(sm_scerb, test_scevent ))) {
            printf("[PRODUCER] Insertion failed  --  Reached (`i`=%d), stopping insertion !\n", i);
            goto cleanup;
        }

        fprintf(stdout, "(%d) Inserted following `fnres_scevent` struct: \n", i +1);
        fnres_scerb_debug_print_scevent(test_scevent, NULL); printf("\n\t-> ");
        //fnres_scerb_debug_print_status(NULL);
    }
cleanup:
    free(test_scevent);
}


    /* --  5. Destroy rb  -- */
    DIE_WHEN_ERRNO( sem_wait(sem_consumer_finished) );
    DIE_WHEN_ERR( fnres_scerb_destory_and_detach(&sm_scerb, SMO_NAME) );
    fprintf(stdout, "[PRODUCER] (5.) Destroyed rb\n");


/* NOTE: We don't wait 4 consumer -> use death of producer as signal to child that buffer was full ... */
    fprintf(stdout, "[PRODUCER] Exiting ...\n");
    return 0;
}


// --------------- --------------- --------------- ---------------  Consumer  --------------- --------------- --------------- ---------------
int run_consumer_aka_libiotrace(
        sem_t *sem_consumer_buf_created, sem_t *sem_consumer_buf_attached, sem_t *sem_consumer_finished) {
    sm_scerb_t *sm_scerb;

/* --  1. Init rb  -- */
    DIE_WHEN_ERR( fnres_scerb_create(&sm_scerb, SMO_NAME) );
    fprintf(stderr, "[CONSUMER] (1.) Inited rb\n");
    DIE_WHEN_ERRNO( sem_post(sem_consumer_buf_created) );


/* --  3. Read data  -- */
    DIE_WHEN_ERRNO( sem_wait(sem_consumer_buf_attached) );
    fprintf(stderr, "[CONSUMER] (3.) Retrieving `fnres_scevent` structs ...\n");
{
    scevent_t *test_scevent = (scevent_t*)alloca(FNRES_SCEVENT_MAX_SIZE);
    for (int i = 0; i < TEST_STRUCTS_TO_INSERT; i++) {
        memset(test_scevent, 0, FNRES_SCEVENT_MAX_SIZE);        // MAKE SURE WE NEVER PRINT OLD DATA
        while (-2 == fnres_scerb_poll(sm_scerb, test_scevent)) {
            fprintf(stderr, "Nothing 2 retrieve -> spinning ...\n");
//            nanosleep((const struct timespec[]){{0, 10000000L}}, NULL);                                   // TESTING --------------------- Reduce spinning
            if (1 == getppid()) {
                LOG_WARN("[CONSUMER] Producer exited, hence nothing 2 consume will be produced");
                _exit(1);       // Otherwise, semaphore will prevent exit
            }
        }
        fprintf(stderr, "(%d) Retrieved `fnres_scevent` struct: \n\t", i +1);
        fnres_scerb_debug_print_scevent(test_scevent, stderr); fprintf(stderr, "\n\t-> ");
        //fnres_scerb_debug_print_status(stderr);
    }
}

/* --  4. Unmap  -- */
    DIE_WHEN_ERR( fnres_scerb_detach(&sm_scerb, SMO_NAME) );
    puts("[PRODUCER] (4.) Detached rb");
    DIE_WHEN_ERRNO( sem_post(sem_consumer_finished) );

    fprintf(stderr, "[CONSUMER] Exiting ...\n");
    return 0;
}



int main(void) {
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);

/* SETUP: Semaphores */
#define SEM_CONSUMER_BUF_CREATED  "/test_sem0"
#define SEM_PRODUCER_BUF_ATTACHED "/test_sem1"
#define SEM_CONSUMER_FINISHED     "/test_sem2"
// Cleanup old semaphores w/ same name (from programs which, e.g. have crashed prior)  ==> Don't check 4 error (since named semaphore might not exist)
    sem_unlink(SEM_CONSUMER_BUF_CREATED);
    sem_unlink(SEM_PRODUCER_BUF_ATTACHED);
    sem_unlink(SEM_CONSUMER_FINISHED);

    sem_t *sem_consumer_buf_created,        // Make sure producer doesn't start buffer which isn't inited yet
          *sem_producer_buf_attached,       // Make sure consumer is attached to buffer before producer writes  (to make sure ringbuffer is used concurrently)
          *sem_consumer_finished;           // Make sure consumer doesn't destroy buffer before producer has finished
    if (SEM_FAILED == (sem_consumer_buf_created = sem_open(SEM_CONSUMER_BUF_CREATED, O_CREAT, 0660, 0)) ||
        SEM_FAILED == (sem_producer_buf_attached = sem_open(SEM_PRODUCER_BUF_ATTACHED, O_CREAT, 0660, 0)) ||
        SEM_FAILED == (sem_consumer_finished = sem_open(SEM_CONSUMER_FINISHED, O_CREAT, 0660, 0))) {
        LOG_ERROR_AND_EXIT("`sem_open` failed: %s", strerror(errno));
    }


/* Run producer & consumer */
    pid_t consumer_pid;
    return ((consumer_pid = DIE_WHEN_ERRNO( fork() ))) ?
           run_producer_aka_stracer(sem_consumer_buf_created, sem_producer_buf_attached, sem_consumer_finished,
                                    consumer_pid) :        // Parent
           run_consumer_aka_libiotrace(sem_consumer_buf_created, sem_producer_buf_attached, sem_consumer_finished);                       // Child
}
