#include <mpi.h>
#include <stdlib.h>

#include <cmath>
#include <cstdio>
#include <iostream>
#include <numeric>
#include <vector>

void CheckSuccess(int state) {
  if (state != MPI_SUCCESS) MPI_Abort(MPI_COMM_WORLD, state);
}

int main(int argc, char* argv[]) {
  int tag = 35817;

  int st;
  st = MPI_Init(&argc, &argv);
  CheckSuccess(st);

  int ranks_amount;
  st = MPI_Comm_size(MPI_COMM_WORLD, &ranks_amount);
  CheckSuccess(st);

  int rank;
  st = MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  CheckSuccess(st);

  srandom(rank);
  int t_0 = random() % 51;
  printf("START_TIME ON RANK_%d: %d\n\n", rank, t_0);

  if (rank) {  // клиент
    int serv_t_0;
    st = MPI_Recv(&serv_t_0, 1, MPI_INT, 0, tag, MPI_COMM_WORLD,
                  MPI_STATUS_IGNORE);
    CheckSuccess(st);

    // printf("CLIENT_%d: first recv:\n\tserv_t_0: %d\n\n", rank, serv_t_0);

    int delta_to_send = t_0 - serv_t_0;
    st = MPI_Send(&delta_to_send, 1, MPI_INT, 0, tag, MPI_COMM_WORLD);
    CheckSuccess(st);

    printf("CLIENT_%d: delta_to_send: %d\n\n", rank, delta_to_send);

    int delta_to_correct;
    st = MPI_Recv(&delta_to_correct, 1, MPI_INT, 0, tag, MPI_COMM_WORLD,
                  MPI_STATUS_IGNORE);
    CheckSuccess(st);

    printf("CLIENT_%d: delta_to_correct: %d\n\n", rank, delta_to_correct);

    t_0 += delta_to_correct;

    printf("CLIENT_%d: FINAL_TIME: %d\n\n", rank, t_0);

  } else {  // сервер
    for (int rank_to_send = 1; rank_to_send < ranks_amount; rank_to_send++) {
      st = MPI_Send(&t_0, 1, MPI_INT, rank_to_send, tag, MPI_COMM_WORLD);
      CheckSuccess(st);

      // printf("SERVER: first send:\n\tt_0: %d\n\tclient: %d\n\n", t_0,
      //        rank_to_send);
    }

    MPI_Status status;
    std::vector<int> delta_times(ranks_amount);
    delta_times[0] = 0;

    for (int rank_to_send = 1; rank_to_send < ranks_amount; rank_to_send++) {
      int received_delta;
      st = MPI_Recv(&received_delta, 1, MPI_INT, MPI_ANY_SOURCE, tag,
                    MPI_COMM_WORLD, &status);
      CheckSuccess(st);

      printf("SERVER: received_delta: %d\n\tclient: %d\n\n", received_delta,
             rank_to_send);

      delta_times[status.MPI_SOURCE] = received_delta;
    }

    int delta_accum =
        std::ceil(std::accumulate(delta_times.begin(), delta_times.end(), 0.0) /
                  delta_times.size());

    printf("SERVER: ACCUM: %d\n\n", delta_accum);

    for (int rank_to_send = 1; rank_to_send < ranks_amount; rank_to_send++) {
      int correct_delta_to_send = delta_accum - delta_times[rank_to_send];

      st = MPI_Send(&correct_delta_to_send, 1, MPI_INT, rank_to_send, tag,
                    MPI_COMM_WORLD);
      CheckSuccess(st);

      printf("SERVER: correct_delta_to_send: %d\n\tclient: %d\n\n",
             correct_delta_to_send, rank_to_send);
    }

    t_0 += delta_accum - delta_times[0];

    printf("SERVER: FINAL_TIME: %d\n\n", t_0);
  }

  MPI_Finalize();
}
