#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>


int runTest_Test(void (*testFunction)(int));
void performIntensiveRamCalculation(int fork_id);
void performIntensiveCPUCalculation(int fork_id);

void wait_for_forks();

#define NUM_FORKS 20
#define LOOP_COUNT_RAM 30000
#define RAM_SIZE_PER_FORK 1024*1024
#define LOOP_COUNT_CPU 80000


int main() {

    for (int i = 0; i < 200; ++i) {
        printf("--- Start %3d in %d CPU TEST ---\n", i, getpid());
        runTest_Test(&performIntensiveCPUCalculation);
        printf("--- End   %3d in %d CPU TEST ---\n", i, getpid());
        sleep(10);
        printf("--- START %3d in %d RAM TEST ---\n", i, getpid());
        runTest_Test(&performIntensiveRamCalculation);
        printf("--- End   %3d in %d RAM TEST ---\n", i, getpid());
        sleep(10);
    }

}

void performIntensiveRamCalculation(int fork_id) {
    int *array = malloc(RAM_SIZE_PER_FORK * sizeof(int));
    int tmp;
    if (array == NULL) {
        printf("MALLOC NOT WORK!");
        return;
    }

    for (long x = 0; x < LOOP_COUNT_RAM; ++x) {
        for (int i = 0; i < RAM_SIZE_PER_FORK; ++i) {
            array[i] = i+tmp;
            tmp = (array[i] * 0) + 1;
        }
    }
    free(array);
}

void performIntensiveCPUCalculation(int fork_id) {
    for (long i = 0; i < LOOP_COUNT_CPU; ++i) {
        for (long x = 0; x < 1024*1024; ++x) {
        }
    }
}

int runTest_Test(void (*testFunction)(int)) {
    for (int i = 0; i < NUM_FORKS; ++i) {
        pid_t child_pid = fork();

        if (child_pid == -1) {
            perror("Fork failed");
            exit(EXIT_FAILURE);
        } else if (child_pid == 0) {

            testFunction(i);

            printf("Kindprozess %d mit PID %d Exit\n", i+1, getpid());
            exit(EXIT_SUCCESS);
        }
    }

    wait_for_forks();
    return 0;
}

void wait_for_forks() {
    int status;
    for (int j = 0; j < NUM_FORKS; ++j) {
        wait(&status);
        //printf("Kindprozess mit PID %d beendet\n", WEXITSTATUS(status));
    }
    printf("Alle Kindprozesse beendet. Neue Forks werden erstellt.\n");
}
