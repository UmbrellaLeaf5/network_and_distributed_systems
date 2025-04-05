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

  std::vector<int> t(ranks_amount, 0);
  printf("RANK_%d: START TIME: ", rank);
  PrintVector(t);
  printf("\n\n");

  std::string file_name = "v0" + ToString(rank + 1) + ".dat";

  if (IsFileExists(file_name)) {
    std::ifstream file(file_name);

    if (file.is_open()) {
      std::string line;

      while (std::getline(file, line)) {
        int value = std::atoi(line.c_str());

        switch (Sign(value)) {
          case 0: {  // внутр
            t[rank] += d;

            printf("RANK_%d: internal:\n\tnew time: ", rank);
            PrintVector(t);
            printf("\n\n");
            break;
          }

          case 1: {  // отправка
            t[rank] += d;

            printf("RANK_%d: sending:\n\tto: RANK_%d\n\tnew time: ", rank,
                   value - 1);
            PrintVector(t);
            printf("\n\n");

            st = MPI_Send(t.data(), ranks_amount, MPI_INT, value - 1, tag,
                          MPI_COMM_WORLD);
            CheckSuccess(st);
            break;
          }

          default: {  // приём
            value = -value;

            t[rank] += d;

            int t_mes[ranks_amount];
            st = MPI_Recv(t_mes, ranks_amount, MPI_INT, value - 1, tag,
                          MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            CheckSuccess(st);

            for (int i = 0; i < ranks_amount; i++)
              if (i != rank) t[i] = std::max(t[i], t_mes[i]);

            printf("RANK_%d: receiving:\n\tfrom: RANK_%d\n\t time: ", value - 1,
                   rank);
            PrintVector(t);
            printf("\n\n");

            break;
          }
        }
      }

      file.close();

      printf("RANK_%d: FINAL TIME: ", rank);
      PrintVector(t);
      printf("\n\n");
    } else
      std::cerr << "Error opening file: " << file_name << std::endl;

  } else
    printf("RANK_%d: NO COMMANDS \n\n", rank);

  MPI_Finalize();
}
