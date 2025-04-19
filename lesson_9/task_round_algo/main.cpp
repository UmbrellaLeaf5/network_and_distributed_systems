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
constexpr int time_out = 2;

volatile sig_atomic_t sigflag = 1;
void Handler(int n_signal) { sigflag = 0; }

int main(int argc, char* argv[]) {
  int leader;
  MPI_Status status;

  CHECK_SUCCESS(MPI_Init(&argc, &argv));

  int ranks_amount;
  CHECK_SUCCESS(MPI_Comm_size(MPI_COMM_WORLD, &ranks_amount));

  int rank;
  CHECK_SUCCESS(MPI_Comm_rank(MPI_COMM_WORLD, &rank));

  std::signal(SIGALRM, Handler);

  // MEANS: список с кандидатами
  std::vector<int> cands(ranks_amount, 0);

  srandom(rank);

  if (rank == no_voting_rank) {
    int next_rank = 0;
    int is_iprobed = 0;

    cands[rank] = 1;

    for (next_rank = (rank + 1) % ranks_amount; next_rank != rank;
         next_rank = (next_rank + 1) % ranks_amount) {
      printf("RANK_%d:\n\tSEND: cands -> RANK_%d\n\n", rank, next_rank);

      CHECK_SUCCESS(MPI_Send(cands.data(), ranks_amount, MPI_INT, next_rank, 0,
                             MPI_COMM_WORLD));

      sigflag = 1;
      alarm(time_out);

      while (sigflag) {
        CHECK_SUCCESS(
            MPI_Iprobe(next_rank, 1, MPI_COMM_WORLD, &is_iprobed, &status));

        if (is_iprobed) {
          CHECK_SUCCESS(MPI_Recv(cands.data(), ranks_amount, MPI_INT, next_rank,
                                 1, MPI_COMM_WORLD, &status));

          printf("RANK_%d: NEXT: RANK_%d\n\n", rank, next_rank);
          alarm(0);
          break;
        }
      }

      if (is_iprobed) break;

      printf("RANK_%d:\n\tRECEIVE: nothing <- RANK_%d\n\n", rank, next_rank);
    }

    if (next_rank == rank) {
      printf("RANK_%d:\n\tONLY ONE process left\n\n", rank);

      goto final;
    }

    printf("RANK_%d: WAITING\n\n", rank);
    CHECK_SUCCESS(MPI_Recv(cands.data(), ranks_amount, MPI_INT, MPI_ANY_SOURCE,
                           0, MPI_COMM_WORLD, &status));

    printf("RANK_%d: CIRCLE CLOSE\n\n", rank);
    CHECK_SUCCESS(MPI_Send(cands.data(), ranks_amount, MPI_INT,
                           status.MPI_SOURCE, 1, MPI_COMM_WORLD));

    if (cands[rank] == 1) {
      for (int i = ranks_amount - 1; i >= 0; i--)
        if (cands[i] == 1) {
          leader = i;
          break;
        }

      printf("RANK_%d:\n\tCURR lead: RANK_%d\n\n", rank, leader);
      CHECK_SUCCESS(
          MPI_Send(&leader, 1, MPI_INT, next_rank, 2, MPI_COMM_WORLD));
    }

    CHECK_SUCCESS(MPI_Recv(&leader, 1, MPI_INT, MPI_ANY_SOURCE, 2,
                           MPI_COMM_WORLD, &status));

    printf("RANK_%d:\n\tCURR lead: RANK_%d\n\n", rank, leader);

  } else if (random() % 2 != 0) {
    int next_rank = 0;
    int is_iprobed = 0;

    CHECK_SUCCESS(MPI_Recv(cands.data(), ranks_amount, MPI_INT, MPI_ANY_SOURCE,
                           0, MPI_COMM_WORLD, &status));

    printf("RANK_%d:\n\tRECEIVE: cands <- RANK_%d\n\n", rank,
           status.MPI_SOURCE);
    printf("RANK_%d:\n\tSEND: OK -> RANK_%d\n\n", rank, status.MPI_SOURCE);

    cands[rank] += 1;
    CHECK_SUCCESS(MPI_Send(cands.data(), ranks_amount, MPI_INT,
                           status.MPI_SOURCE, 1, MPI_COMM_WORLD));

    for (next_rank = (rank + 1) % ranks_amount; next_rank != rank;
         next_rank = (next_rank + 1) % ranks_amount) {
      printf("RANK_%d:\n\tSEND: Vote -> RANK_%d\n\n", rank, next_rank);

      CHECK_SUCCESS(MPI_Send(cands.data(), ranks_amount, MPI_INT, next_rank, 0,
                             MPI_COMM_WORLD));

      sigflag = 1;
      alarm(time_out);

      while (sigflag) {
        CHECK_SUCCESS(
            MPI_Iprobe(next_rank, 1, MPI_COMM_WORLD, &is_iprobed, &status));

        if (is_iprobed) {
          CHECK_SUCCESS(MPI_Recv(cands.data(), ranks_amount, MPI_INT, next_rank,
                                 1, MPI_COMM_WORLD, &status));

          printf("RANK_%d: NEXT: RANK_%d\n\n", rank, next_rank);
          alarm(0);
          break;
        }
      }

      if (is_iprobed) break;
    }

    CHECK_SUCCESS(MPI_Recv(&leader, 1, MPI_INT, MPI_ANY_SOURCE, 2,
                           MPI_COMM_WORLD, &status));

    CHECK_SUCCESS(MPI_Send(&leader, 1, MPI_INT, next_rank, 2, MPI_COMM_WORLD));

    printf("RANK_%d:\n\tCURR lead: RANK_%d\n\n", rank, leader);

  } else
    printf("RANK_%d: DEAD\n\n", rank);

final:
  MPI_Finalize();
  return 0;
}
