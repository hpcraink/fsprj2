#include <omp.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <assert.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>

/* rm -f mptf_*.txt && IOTRACE_LOG_NAME=multi_test1 IOTRACE_DATABASE_IP=127.0.0.1 IOTRACE_DATABASE_PORT=8086 IOTRACE_INFLUX_ORGANIZATION=hse IOTRACE_INFLUX_BUCKET=hsebucket IOTRACE_INFLUX_TOKEN=OXBWllU1poZotgyBlLlo2XQ_u4AYGYKQmdxvJJeotKRyvdn5mwjEhCXyOjyldpMmNt_9YY4k3CK-f5Eh1bN0Ng== LD_PRELOAD=../src/libiotrace_shared.so OMP_NUM_THREADS=4 ./multi_shared 20 4 */

void usage(void) {
    printf(
            "USAGE: OMP_NUM_THREADS=n multi_processes_threads_files [ITERATIONS] [FILES_COUNT] [SEED]\n"
                    "\n"
                    "Runs File I/O tests on multiple files (named mptf_*.txt).\n"
                    "If the optional ITERATIONS is not specified, this will indefinitely\n"
                    "write into file mptf_*.txt filling up Your file-system.\n"
                    "Default for FILES_COUNT is 1. Default for SEED is current time.\n"
                    "If FILES_COUNT is specified, ITERATIONS must be specified too.\n"
                    "If SEED is specified, ITERATIONS and FILES_COUNT must be specified\n"
                    "too.\n");
    exit(EXIT_FAILURE);
}

int32_t random_number(struct random_data buf, int max /* exclusive */) {
    int32_t result = 0;
    int ret = random_r(&buf, &result);
    assert(0 == ret);
    return result % max;
}

int open_file(int current_file, int thread_num, int count_files, FILE *file[]) {
    int tmp_file = (current_file + 1) % count_files;
    printf("thread %d: open file %d\n", thread_num, tmp_file);
    fflush(stdout);
    if (NULL == file[tmp_file]) {
        char filename[50];
        snprintf(filename, 50, "mptf_%d.txt", tmp_file);
        filename[50 - 1] = '\0';

        FILE *tmp = fopen(filename, "a+");
        if (NULL != tmp) {
            file[tmp_file] = tmp;
        }
    }
    return tmp_file;
}

int main(int argc, char *argv[]) {
    const size_t max_bytes = 4000;
    const unsigned int max_sleep = 5;
    int max_iterations = INT_MAX;
    int count_files = 1;
//    const int count_threads = omp_get_max_threads();
    int base_seed = -1;

    if (argc > 1) {
        char *endptr;
        max_iterations = strtol(argv[1], &endptr, 10);
        if (*endptr != '\0' || max_iterations < 0)
            usage();
    }
    if (argc > 2) {
        char *endptr;
        count_files = strtol(argv[2], &endptr, 10);
        if (*endptr != '\0' || count_files < 0)
            usage();
    }
    if (argc > 3) {
        char *endptr;
        base_seed = strtol(argv[3], &endptr, 10);
        if (*endptr != '\0' || base_seed < 0)
            usage();
    }

    if (0 > base_seed) {
        struct timespec t;
        clock_gettime(CLOCK_REALTIME, &t);
        base_seed = t.tv_nsec;
    }

#pragma omp parallel
    {
        int thread_num = omp_get_thread_num();
        printf("Start thread %d\n", thread_num);
        fflush(stdout);
        FILE *file[count_files];
        int current_file = -1;
        int32_t result = 0;
        char statebuf[256];
        struct random_data buf;
        buf.state = NULL;

        for (int i = 0; i < count_files; i++) {
            file[i] = NULL;
        }

        unsigned int seed = (int) base_seed + thread_num;
        int ret = initstate_r(seed, statebuf, sizeof(statebuf), &buf);
        assert(0 == ret);

////#pragma omp single
////        {
//            printf("Start reading and writing\n");
//            fflush(stdout);

        for (int i = 0; i < max_iterations; i++) {
//#pragma omp task
//                {
            result = random_number(buf, 4);

            switch (result) {
            case 0:
                /* close current file */
                if (0 < current_file && NULL != file[current_file]) {
                    printf("thread %d: close file %d\n", thread_num,
                            current_file);
                    fflush(stdout);
                    fclose(file[current_file]);
                    file[current_file] = NULL;
                } else {
                    current_file = open_file(current_file, thread_num,
                            count_files, file);
                }
                break;
            case 1:
                /* write to current file */
                if (0 > current_file || NULL == file[current_file]) {
                    current_file = open_file(current_file, thread_num,
                            count_files, file);
                } else {
                    printf("thread %d: write file %d\n", thread_num,
                            current_file);
                    fflush(stdout);
                    result = random_number(buf, max_bytes);
                    result += 1;
                    void *tmp_addr = malloc(result);
                    assert(NULL != tmp_addr);
                    memset(tmp_addr, (int) result, result);
                    fwrite(tmp_addr, 1, result, file[current_file]);
                }
                break;
            case 2:
                /* read current file */
                if (0 > current_file || NULL == file[current_file]) {
                    current_file = open_file(current_file, thread_num,
                            count_files, file);
                } else {
                    printf("thread %d: read file %d\n", thread_num,
                            current_file);
                    fflush(stdout);
                    result = random_number(buf, max_bytes);
                    result += 1;
                    void *tmp_addr = malloc(result);
                    assert(NULL != tmp_addr);
                    fread(tmp_addr, 1, result, file[current_file]);
                }
                break;
            case 3: {
                /* change current file */
                printf("thread %d: change file %d\n", thread_num, current_file);
                fflush(stdout);
                int tmp_file = (current_file + 1) % count_files;
                current_file = tmp_file;
                break;
            }
            default:
                /* nothing */
                printf("thread %d: nothing\n", thread_num);
                fflush(stdout);
                break;
            }

            sleep(random_number(buf, max_sleep));
//                }
        }

//#pragma omp taskwait
//        printf("End reading and writing\n");
//        fflush(stdout);
//    }
    }
}
