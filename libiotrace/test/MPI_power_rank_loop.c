#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>
#include <unistd.h>

#define INTERVALS 1000000
#define TAG 0

double calculate_pi(int intervals) {
    double sum = 0.0;
    for (int i = 0; i < intervals; i++) {
        double x = (i + 0.5) / intervals;
        sum += 4.0 / (1.0 + x * x);
    }
    return sum / intervals;
}

void send_ready_signal(int rank, int size) {
    for (int i = 0; i < size; i++) {
        if (i != rank) {
            MPI_Send(NULL, 0, MPI_BYTE, i, TAG, MPI_COMM_WORLD);
        }
    }
}

void wait_for_signals_from_rank(int rank) {
    MPI_Status status;
    int flag;

    while (1) {
        MPI_Iprobe(rank, TAG, MPI_COMM_WORLD, &flag, &status);

        if (flag) {
            MPI_Recv(NULL, 0, MPI_BYTE, rank, TAG, MPI_COMM_WORLD, &status);
            break;
        } else {
            sleep(10);
        }
    }
}

int main(int argc, char* argv[]) {
    int rank, size, iterations = INT_MAX;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) {
            printf("Invalid number of iterations. Using default (infinite).\n");
            iterations = INT_MAX;
        }
    }

    time_t startup_time = time(NULL);

    for (int count = 0; count < iterations; count++) {
        for (int current_rank = 0; current_rank < size; current_rank++) {
            if (rank == current_rank) {
                time_t start_time = time(NULL);

                printf("[%05ld] Rank %d starting as %d heavy computation in iteration %d\n", (time(NULL)-startup_time), rank,current_rank, count + 1);

                while (difftime(time(NULL), start_time) < 10.0) {
                    calculate_pi(INTERVALS);
                }

                send_ready_signal(rank, size);
            }else {
                printf("[%05ld] Rank %d wait for all in iteration %d\n", (time(NULL)-startup_time), rank, count + 1);
                wait_for_signals_from_rank(current_rank);
            }
            //Sync all
            MPI_Barrier(MPI_COMM_WORLD);
        }
    }

    MPI_Finalize();
    return 0;
}