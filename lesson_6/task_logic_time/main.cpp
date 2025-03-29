#include <mpi.h>
#include <stdlib.h>

#include <cstdio>

#include "utils.hpp"

void CheckSuccess(int state) {
  if (state != MPI_SUCCESS) MPI_Abort(MPI_COMM_WORLD, state);
}

int main(int argc, char* argv[]) {
  int tag = 35817;
  int d = 1;

  int st;
  st = MPI_Init(&argc, &argv);
  CheckSuccess(st);

  int ranks_amount;
  st = MPI_Comm_size(MPI_COMM_WORLD, &ranks_amount);
  CheckSuccess(st);

  int rank;
  st = MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  CheckSuccess(st);

  int t = 0;
  printf("RANK_%d: START TIME: %d\n\n", rank, t);

  std::string file_name = "0" + ToString(rank + 1) + ".dat";

  if (IsFileExists(file_name)) {
    std::ifstream file(file_name);

    if (file.is_open()) {
      std::string line;

      while (std::getline(file, line)) {
        int value = std::atoi(line.c_str());

        switch (Sign(value)) {
          case 0: {  // внутр
            t += d;

            printf("RANK_%d: internal:\n\tnew time: %d\n\n", rank, t);
            break;
          }

          case 1: {  // отправка
            t += d;

            printf("RANK_%d: sending:\n\tto: RANK_%d\n\tnew time: %d\n\n", rank,
                   value - 1, t);

            st = MPI_Send(&t, 1, MPI_INT, value - 1, tag, MPI_COMM_WORLD);
            CheckSuccess(st);
            break;
          }

          default: {  // приём
            value = -value;

            int t_mes;
            st = MPI_Recv(&t_mes, 1, MPI_INT, value - 1, tag, MPI_COMM_WORLD,
                          MPI_STATUS_IGNORE);
            CheckSuccess(st);

            int t_new = std::max(t, t_mes) + d;

            printf(
                "RANK_%d: receiving:\n\tfrom: RANK_%d\n\told time: "
                "%d\n\tnew time: %d\n\n",
                value - 1, rank, t, t_new);

            t = t_new;
            break;
          }
        }
      }

      file.close();

      printf("RANK_%d: FINAL TIME: %d\n\n", rank, t);
    } else {
      std::cerr << "Error opening file: " << file_name << std::endl;
    }

  } else
    printf("RANK_%d: NO COMMANDS \n\n", rank);

  MPI_Finalize();
}
