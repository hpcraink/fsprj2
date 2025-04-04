#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <omp.h>
#include "mpi.h"

#define MAX_FILE_NAME_LEN 100

#define ERROR_FATAL(s,val)   do { \
        char _mpi_error_string[MPI_MAX_ERROR_STRING]; \
        int _len = MPI_MAX_ERROR_STRING; \
        MPI_Error_string((val), _mpi_error_string, &_len); \
        fprintf(stderr, "ERROR (%s:%d) in %s errval:%d (in case of POSIX call:'%s', in case of MPI:'%s')\n", \
                __FILE__, __LINE__, (s), val, strerror(val), _mpi_error_string); \
        exit(val); \
    } while(0)

#define MPI_CHECK(x)  do { \
        int _ret = x; \
        if (_ret != MPI_SUCCESS) { \
            ERROR_FATAL("MPI", _ret);\
        } \
    } while(0)

struct open_file {
	int fh;
	int process_id;
	int thread_id;
	int file_number;
};

int open_file(const char *file_prefix, const char *file_postfix, int process_id, int thread_id, int file_number) {
	char file_name[MAX_FILE_NAME_LEN];
        snprintf(file_name, MAX_FILE_NAME_LEN, "%s_%d_%d_%d%s", file_prefix, process_id, thread_id, file_number, file_postfix);

        int fh = open(file_name, O_RDWR | O_APPEND | O_CREAT | O_SYNC, S_IRUSR | S_IWUSR);
        assert(-1 < fh);

        return fh;
}

static __thread size_t open_files_count = 0;
static __thread struct open_file* files;

int get_file_handle(const char *file_prefix, const char *file_postfix, int process_id, int thread_id, int file_number, size_t max_files_per_thread) {
	if (open_files_count == 0) {
		files = malloc(sizeof(struct open_file) * max_files_per_thread);
		assert(NULL != files);
	}

	for(size_t i = 0; i < open_files_count; i++) {
		if (files[i].process_id == process_id &&
			files[i].thread_id == thread_id &&
			files[i].file_number == file_number) {
			if (-1 < files[i].fh) {
				printf("existing open file (%d, %d, %d): pos %lu\n", process_id, thread_id, file_number, i);
				return files[i].fh;
			} else {
				files[i].fh = open_file(file_prefix, file_postfix, process_id, thread_id, file_number);
				printf("existing reopened file (%d, %d, %d): pos %lu\n", process_id, thread_id, file_number, i);
				return files[i].fh;
			}
		}
	}
	
	files[open_files_count].process_id = process_id;
	files[open_files_count].thread_id = thread_id;
	files[open_files_count].file_number = file_number;
	files[open_files_count].fh = open_file(file_prefix, file_postfix, process_id, thread_id, file_number);
	open_files_count++;

	printf("new open file (%d, %d, %d): pos %lu\n", process_id, thread_id, file_number, open_files_count - 1);
	return files[open_files_count - 1].fh;
}

void close_file(int process_id, int thread_id, int file_number) {
	for(size_t i = 0; i < open_files_count; i++) {
                if (files[i].process_id == process_id &&
                        files[i].thread_id == thread_id &&
                        files[i].file_number == file_number) {
                        if (-1 < files[i].fh) {
                                close(files[i].fh);
				files[i].fh = -1;
                        }
                }
        }
}

int get_random_number(int min, int max, unsigned int *seed) {
	return min + rand_r(seed) / (RAND_MAX / (max - min + 1) + 1);
}

int get_random_process_id(size_t files_distribution, int process_id) {
	if (0 == files_distribution) {
		return 0;
	} else {
		return process_id;
	} 
}

int get_random_thread_id(size_t files_distribution, int thread_id) {
        if (0 == files_distribution) {
                return 0;
        } else if (1 == files_distribution) {
		return 0;
	} else {
                return thread_id;
        }
}

int get_random_file_id(int files, unsigned int *seed) {
        return get_random_number(0, files - 1, seed);
}

int main(int argc, char **argv) {
	const char *usage =
		"Usage: mpiexec -n p -x OMP_NUM_THREADS=t ./mpi_random_file_io f files_distribution sleep duration minb maxb seed\n"
		"    p                  number of processes\n"
		"    t                  number of threads\n"
		"    f                  number of files\n"
		"    files_distribution 0: files per program\n"
		"                       1: files per process\n"
		"                       2: files per thread\n"
		"    sleep              seconds to sleep between file operations\n"
		"    duration           seconds until program terminates\n"
		"    minb               minimum bytes to read or write during one operation\n"
		"    maxb               maximum bytes to read or write during one operation\n"
		"    seed               seed for randomization\n"
		"Example: rm -f mpi_random_file_io_*.txt && mpiexec -n 4 -x OMP_NUM_THREADS=2 ./mpi_random_file_io 20 0 2 120 10 200 1234567\n";

	//int comm_size;
	int comm_rank;
	const char *file_prefix = "mpi_random_file_io";
	const char *file_postfix = ".txt";

        if (argc < 8) {
		printf("ERROR: missing argument\n%s", usage);
                return 1;
        }
	long tmp_f = strtol(argv[1], NULL, 0);
        if (0 > tmp_f) {
                printf("ERROR: number of files must be positive\n%s", usage);
                return 2;
        }
	size_t f = tmp_f;
	long tmp_files_distribution = strtol(argv[2], NULL, 0);
        if (0 > tmp_files_distribution || 2 < tmp_files_distribution) {
                printf("ERROR: files distribution must be 0, 1 or 2\n%s", usage);
                return 3;
        }
	size_t files_distribution = tmp_files_distribution;
	long tmp_seconds_to_sleep = strtol(argv[3], NULL, 0);
        if (0 > tmp_seconds_to_sleep) {
                printf("ERROR: seconds to sleep must be positive\n%s", usage);
                return 4;
        }
	size_t seconds_to_sleep = tmp_seconds_to_sleep;
	long tmp_duration = strtol(argv[4], NULL, 0);
        if (0 > tmp_duration) {
                printf("ERROR: seconds until program terminates must be positive\n%s", usage);
                return 5;
        }
	size_t duration = tmp_duration;
	long tmp_minb = strtol(argv[5], NULL, 0);
        if (0 > tmp_minb) {
                printf("ERROR: minimum bytes to read or write must be positive\n%s", usage);
                return 6;
        }
	size_t minb = tmp_minb;
	long tmp_maxb = strtol(argv[6], NULL, 0);
        if (0 > tmp_maxb || tmp_maxb < tmp_minb) {
                printf("ERROR: maximum bytes to read or write must be positive and greater then minb\n%s", usage);
                return 7;
        }
	size_t maxb = tmp_maxb;
	long tmp_s = strtol(argv[7], NULL, 0);
	if (0 > tmp_s) {
                printf("ERROR: seed must be positive\n%s", usage);
                return 8;
        }
	unsigned int s = tmp_s;

	int mpi_thread_level = MPI_THREAD_MULTIPLE;
	int mpi_thread_level_supported;
	MPI_CHECK(MPI_Init_thread(&argc, &argv, mpi_thread_level, &mpi_thread_level_supported));
	if (mpi_thread_level != mpi_thread_level_supported)
        	ERROR_FATAL("MPI Thread-level not supported", EINVAL);

	//MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
	MPI_Comm_rank(MPI_COMM_WORLD, &comm_rank);

	char * to_write = malloc(maxb);
	assert(NULL != to_write);
	for (size_t i = 0; i < maxb; i++)
        	to_write[i] = ('a' + i) % 24;

	time_t start = time(NULL);

#pragma omp parallel
	{
		//threads = omp_get_num_threads();
                int thread_id = omp_get_thread_num();
                unsigned int seed = s + comm_rank + thread_id;

		// check duration
		while ((size_t)(time(NULL) - start) < duration) {

			// sleep
			sleep(seconds_to_sleep);

			// scale max open files over duration
			size_t max_open_files = (size_t)ceil(((float)f / duration) * (time(NULL) - start));
			if (f < max_open_files) {
				max_open_files = f;
			}

			// random choose a file
			int file_process_id = get_random_process_id(files_distribution, comm_rank);
			int file_thread_id = get_random_thread_id(files_distribution, thread_id);
			int file_id = get_random_file_id(max_open_files, &seed);

			// random choose a action
			int action = get_random_number(0, 2, &seed);

			// execute action
			switch (action) {
				case 0:
					{
						// write
						int bytes = get_random_number(minb, maxb, &seed);
						int fh = get_file_handle(file_prefix, file_postfix, file_process_id, file_thread_id, file_id, f);
						int ret = write(fh, to_write, bytes);
						assert(-1 < ret);
						break;
					}
				case 1:
					{
						// read
						int bytes = get_random_number(minb, maxb, &seed);
                                        	int fh = get_file_handle(file_prefix, file_postfix, file_process_id, file_thread_id, file_id, f);
                                        	int ret = read(fh, to_write, bytes);
                                        	assert(-1 < ret);
                                        	break;
					}
				case 2:
					// close
					close_file(file_process_id, file_thread_id, file_id);
					break;
				default:
					__builtin_unreachable();
			}
		}
	}

	MPI_CHECK(MPI_Finalize());
	return EXIT_SUCCESS;
}
