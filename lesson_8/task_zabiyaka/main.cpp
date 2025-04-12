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

#define CHECK_SUCCESS(state)                                \
  {                                                         \
    if (state != MPI_SUCCESS) MPI_Abort(MPI_COMM_WORLD, 1); \
  }

constexpr int no_voting_rank = 1;
constexpr int time_out = 5;
constexpr int tag = 35817;

volatile sig_atomic_t sigflag = 1;
void Handler(int n_signal) { sigflag = 0; }

int main(int argc, char* argv[]) {
  CHECK_SUCCESS(MPI_Init(&argc, &argv));

  int ranks_amount;
  CHECK_SUCCESS(MPI_Comm_size(MPI_COMM_WORLD, &ranks_amount));

  int rank;
  CHECK_SUCCESS(MPI_Comm_rank(MPI_COMM_WORLD, &rank));

  std::signal(SIGALRM, Handler);

  srandom(rank);

  if (random() % 2 == 0 || rank == no_voting_rank) {
    if (rank == no_voting_rank) {
      char mes = 'V';
      for (int i = rank + 1; i < ranks_amount; i++) {
        printf("RANK %d:\n\tVOTE -> RANK %d\n\n", rank, i);

        CHECK_SUCCESS(MPI_Send(&mes, 1, MPI_CHAR, i, tag, MPI_COMM_WORLD));
      }

      char gotten_message = 'N';
      MPI_Status status;

      alarm(time_out);
      while (sigflag) {
        int is_message_recv;

        CHECK_SUCCESS(MPI_Iprobe(MPI_ANY_SOURCE, tag, MPI_COMM_WORLD,
                                 &is_message_recv, &status));

        if (is_message_recv) {
          MPI_Recv(&gotten_message, 1, MPI_CHAR, status.MPI_SOURCE, tag,
                   MPI_COMM_WORLD, MPI_STATUS_IGNORE);

          if (gotten_message == 'O') {
            printf("RANK %d:\n\tRECEIVING: 'OK' <- RANK %d\n\n", rank,
                   status.MPI_SOURCE);

            alarm(0);
          }

          if (gotten_message == 'L') {
            printf("RANK %d:\n\tCURR LEAD: %d\n\n", rank, status.MPI_SOURCE);

            break;
          }
        }
      }

      if (gotten_message == 'N' || gotten_message == 'V') {
        printf("RANK %d: became LEAD!\n\n", rank);

        char mes = 'L';
        for (int i = 0; i < ranks_amount; i++)
          CHECK_SUCCESS(MPI_Send(&mes, 1, MPI_CHAR, i, tag, MPI_COMM_WORLD));
      }

    } else {
      char gotten_message;
      MPI_Status status;
      MPI_Recv(&gotten_message, 1, MPI_CHAR, MPI_ANY_SOURCE, tag,
               MPI_COMM_WORLD, &status);

      if (gotten_message == 'V') {
        printf("RANK %d:\n\tSENDING: 'OK' -> RANK %d\n\n", rank,
               status.MPI_SOURCE);

        char mes = 'O';
        CHECK_SUCCESS(MPI_Send(&mes, 1, MPI_CHAR, status.MPI_SOURCE, tag,
                               MPI_COMM_WORLD));

        mes = 'V';
        for (int i = rank + 1; i < ranks_amount; i++) {
          printf("RANK %d:\n\tVOTE -> RANK %d\n\n", rank, i);

          CHECK_SUCCESS(MPI_Send(&mes, 1, MPI_CHAR, i, tag, MPI_COMM_WORLD));
        }

        sigflag = 1;
        gotten_message = 'N';

        alarm(time_out);

        while (sigflag) {
          int is_message_recv;
          CHECK_SUCCESS(MPI_Iprobe(MPI_ANY_SOURCE, tag, MPI_COMM_WORLD,
                                   &is_message_recv, &status));

          if (is_message_recv) {
            MPI_Recv(&gotten_message, 1, MPI_CHAR, status.MPI_SOURCE, tag,
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            if (gotten_message == 'O') {
              printf("RANK %d:\n\tRECEIVING: 'OK' <- RANK %d\n\n", rank,
                     status.MPI_SOURCE);

              alarm(0);
            }

            if (gotten_message == 'L') {
              printf("RANK %d:\n\tCURR LEAD: %d\n\n", rank, status.MPI_SOURCE);

              break;
            }

            if (gotten_message == 'V') {
              printf("RANK %d:\n\tSENDING: 'OK' -> RANK %d\n\n", rank,
                     status.MPI_SOURCE);

              char mes = 'O';
              CHECK_SUCCESS(MPI_Send(&mes, 1, MPI_CHAR, status.MPI_SOURCE, tag,
                                     MPI_COMM_WORLD));
            }
          }
        }

        if (gotten_message == 'N' || gotten_message == 'V') {
          printf("RANK %d: became LEAD!\n\n", rank);

          char mes = 'L';
          for (int i = 0; i < ranks_amount; i++)
            CHECK_SUCCESS(MPI_Send(&mes, 1, MPI_CHAR, i, tag, MPI_COMM_WORLD));
        }
      }

      if (gotten_message == 'L')
        printf("RANK %d:\n\tCURR LEAD: %d\n\n", rank, status.MPI_SOURCE);
    }

  } else
    printf("RANK %d: DEAD\n\n", rank);

  MPI_Finalize();
  return 0;
}
