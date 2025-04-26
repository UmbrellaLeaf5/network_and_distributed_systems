#include <mpi.h>
#include <stdlib.h>
#include <unistd.h>

#include <cmath>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <numeric>
#include <random>
#include <vector>

constexpr int repeat = 3;
int unused_mess;

#define CHECK_SUCCESS(state)                                \
  {                                                         \
    if (state != MPI_SUCCESS) MPI_Abort(MPI_COMM_WORLD, 1); \
  }

void ReminderSection(int rank, int ranks_amount) {
  CHECK_SUCCESS(MPI_Recv(&unused_mess, 1, MPI_INT, MPI_ANY_SOURCE, 0,
                         MPI_COMM_WORLD, MPI_STATUS_IGNORE));

  CHECK_SUCCESS(MPI_Send(&unused_mess, 1, MPI_INT, (rank + 1) % ranks_amount, 0,
                         MPI_COMM_WORLD));
}

void FinalSection(int rank, int ranks_amount) {
  MPI_Status status;

  CHECK_SUCCESS(MPI_Recv(&unused_mess, 1, MPI_INT, MPI_ANY_SOURCE, 1,
                         MPI_COMM_WORLD, &status));

  printf("RANK_%d:\n\tRECEIVE: FIN <- RANK_%d\n\n", rank, status.MPI_SOURCE);

  CHECK_SUCCESS(MPI_Send(&unused_mess, 1, MPI_INT, (rank + 1) % ranks_amount, 1,
                         MPI_COMM_WORLD));

  printf("RANK_%d:\n\tSEND: FIN -> RANK_%d (next)\n\n", rank,
         (rank + 1) % ranks_amount);
}

volatile sig_atomic_t sigflag = 1;
void Handler(int n_signal) { sigflag = 0; }

int main(int argc, char* argv[]) {
  int repeating_count = 0, flag, unused_mess;
  MPI_Status status;

  CHECK_SUCCESS(MPI_Init(&argc, &argv));

  int ranks_amount;
  CHECK_SUCCESS(MPI_Comm_size(MPI_COMM_WORLD, &ranks_amount));

  int rank;
  CHECK_SUCCESS(MPI_Comm_rank(MPI_COMM_WORLD, &rank));

  signal(SIGALRM, Handler);

  srand(rank);

  if (rank != 0) {
    while (repeating_count != repeat) {
      printf("RANK_%d: WAITS marker\n\n", rank);

      CHECK_SUCCESS(MPI_Recv(&unused_mess, 1, MPI_INT, MPI_ANY_SOURCE, 0,
                             MPI_COMM_WORLD, &status));

      printf("RANK_%d:\n\tRECEIVE: marker <- RANK_%d\n\n", rank,
             status.MPI_SOURCE);
      printf("RANK_%d: GO critical\n\n", rank);

      sleep(rand() % 2 + 1);

      printf("RANK_%d: SLEEP\n\n", rank);

      CHECK_SUCCESS(MPI_Send(&unused_mess, 1, MPI_INT,
                             (rank + 1) % ranks_amount, 0, MPI_COMM_WORLD));

      sigflag = 1;
      alarm(rand() % 5 + 1);

      while (sigflag) {
        CHECK_SUCCESS(
            MPI_Iprobe(MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &flag, &status));

        if (flag) ReminderSection(rank, ranks_amount);
      }

      repeating_count++;
    }

    int count = 0;
    sigflag = 1;

    while (sigflag) {
      CHECK_SUCCESS(MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD,
                               &flag, &status));

      if (flag) {
        if (status.MPI_TAG == 0) ReminderSection(rank, ranks_amount);

        if (status.MPI_TAG == 1) {
          FinalSection(rank, ranks_amount);

          count++;
          if (count == 2) break;
        }
      }
    }

  } else {
    CHECK_SUCCESS(MPI_Send(&unused_mess, 1, MPI_INT, (rank + 1) % ranks_amount,
                           0, MPI_COMM_WORLD));

    while (repeating_count != repeat) {
      printf("RANK_%d: WAITS marker\n\n", rank);

      CHECK_SUCCESS(MPI_Recv(&unused_mess, 1, MPI_INT, MPI_ANY_SOURCE, 0,
                             MPI_COMM_WORLD, &status));

      printf("RANK_%d:\n\tRECEIVE: marker <- RANK_%d\n\n", rank,
             status.MPI_SOURCE);
      printf("RANK_%d: GO critical\n\n", rank);

      sleep(rand() % 2 + 1);

      printf("RANK_%d: SLEEP\n\n", rank);

      CHECK_SUCCESS(MPI_Send(&unused_mess, 1, MPI_INT,
                             (rank + 1) % ranks_amount, 0, MPI_COMM_WORLD));

      sigflag = 1;
      alarm(rand() % 5 + 1);

      while (sigflag) {
        CHECK_SUCCESS(
            MPI_Iprobe(MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &flag, &status));

        if (flag) ReminderSection(rank, ranks_amount);
      }

      repeating_count++;
    }

    printf("RANK_%d: ENTER final section\n\n", rank);

    CHECK_SUCCESS(MPI_Send(&unused_mess, 1, MPI_INT, (rank + 1) % ranks_amount,
                           1, MPI_COMM_WORLD));

    sigflag = 1;
    int count = 0;

    while (sigflag) {
      CHECK_SUCCESS(MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD,
                               &flag, &status));

      if (flag) {
        if (status.MPI_TAG == 0) ReminderSection(rank, ranks_amount);

        if (status.MPI_TAG == 1) {
          FinalSection(rank, ranks_amount);

          count++;
          if (count == 2) break;
        }
      }
    }
  }

  printf("RANK_%d: FINISHED\n\n", rank);
  MPI_Finalize();
  return 0;
}