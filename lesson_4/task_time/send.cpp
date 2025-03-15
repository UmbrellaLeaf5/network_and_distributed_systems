#include <mpi.h>
#include <stdlib.h>

#include <cmath>
#include <cstdio>
#include <iostream>

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
  int t_0 = random() % 21;
  printf("Rank: %d; t_0: %d\n\n", rank, t_0);

  if (rank) {  // клиент
    st = MPI_Send(&t_0, 1, MPI_INT, 0, tag, MPI_COMM_WORLD);
    CheckSuccess(st);

    int send_data[2];
    st = MPI_Recv(&send_data, 2, MPI_INT, MPI_ANY_SOURCE, tag, MPI_COMM_WORLD,
                  MPI_STATUS_IGNORE);
    CheckSuccess(st);

    int t_utc = send_data[0];
    int d = send_data[1];

    int t_1 = t_0 + (random() % 6) + 5;
    int t_1_corrected = t_utc + int(ceil(double(t_1 - t_0 - d) / 2.0));

    printf("On client %d:\n\tt_utc: %d\n\td: %d\n\tt_1: %d\n\tt_1_c: %d\n",
           rank, t_utc, d, t_1, t_1_corrected);

  } else {  // сервер
    for (int i = 1; i < ranks_amount; i++) {
      MPI_Status status;
      int buf;
      st = MPI_Recv(&buf, 1, MPI_INT, MPI_ANY_SOURCE, tag, MPI_COMM_WORLD,
                    &status);
      CheckSuccess(st);

      t_0 += random() % 21;
      int d = random() % 4;
      t_0 += d;
      int t_utc = t_0;

      printf("On server:\n\tt_utc: %d\n\td: %d\n\tclient: %d\n", t_utc, d,
             status.MPI_SOURCE);

      int send_data[2] = {t_utc, d};

      st = MPI_Send(&send_data, 2, MPI_INT, status.MPI_SOURCE, tag,
                    MPI_COMM_WORLD);
      CheckSuccess(st);
    }
  }

  MPI_Finalize();
}
