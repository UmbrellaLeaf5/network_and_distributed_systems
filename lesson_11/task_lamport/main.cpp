#include <mpi.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

#include <cmath>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <numeric>
#include <random>
#include <vector>

#include "lamport_queue.hpp"
#include "utils.hpp"

// MARK: RANDOM

#define ALARM_RAND(var)      \
  {                          \
    alarm(rand() % var + 1); \
  }

constexpr int critical_time = 4;
constexpr int reminder_time = 9;

// MARK: TAGS

constexpr int request_tag = 0;
constexpr int reply_tag = 1;
constexpr int release_tag = 2;
constexpr int final_tag = 3;

// MARK: LAMPORT time

constexpr int d = 1;
int local_time = 0;

#define LAMPORT_TICK \
  {                  \
    local_time += d; \
  }

#define LAMPORT_UPDATE(new_time)                     \
  {                                                  \
    local_time = std::max(local_time, new_time) + d; \
  }

// MARK: HANDLER

volatile sig_atomic_t sigflag = 1;
void Handler(int n_signal) { sigflag = 0; }

// MARK: MPI extra stuff

MPI_Status status;
constexpr int repeat = 3;

#define CHECK_SUCCESS(state)                                \
  {                                                         \
    if (state != MPI_SUCCESS) MPI_Abort(MPI_COMM_WORLD, 1); \
  }

// MARK: Special FUNCS

void SendAll(int rank, int ranks_amount, int tag) {
  LAMPORT_TICK;

  for (int other_rank = 0; other_rank < ranks_amount; other_rank++)
    if (other_rank != rank)
      CHECK_SUCCESS(
          MPI_Send(&local_time, 1, MPI_INT, other_rank, tag, MPI_COMM_WORLD));

  printf("RANK_%d: SEND_ALL: %s | time: %d\n\n", rank,
         tag == request_tag   ? "REQUEST"
         : tag == release_tag ? "RELEASE"
         : tag == final_tag   ? "FIN"
                              : "UNKNOWN",
         local_time);
}

// (same part for `Work` and `ProcessMessages` functions)
#define HANDLE_MESSAGE                                                   \
  {                                                                      \
    printf("RANK_%d: RECEIVE: %s: {%d, %d} | time: %d\n\n", rank,        \
           status.MPI_TAG == request_tag   ? "REQUEST"                   \
           : status.MPI_TAG == reply_tag   ? "REPLY"                     \
           : status.MPI_TAG == release_tag ? "RELEASE"                   \
           : status.MPI_TAG == final_tag   ? "FIN"                       \
                                           : "UNKNOWN",                    \
           message_time, status.MPI_SOURCE, local_time);                 \
                                                                         \
    if (status.MPI_TAG == request_tag) {                                 \
      lq.Add({message_time, status.MPI_SOURCE});                         \
      std::cout << "RANK_" << rank << ": QUEUE (after REQUEST): " << lq  \
                << "| time: " << local_time << "\n\n";                   \
                                                                         \
      LAMPORT_TICK;                                                      \
      CHECK_SUCCESS(MPI_Send(&local_time, 1, MPI_INT, status.MPI_SOURCE, \
                             reply_tag, MPI_COMM_WORLD));                \
                                                                         \
      printf("RANK_%d: SEND_REPLY to: RANK_%d | time: %d\n\n", rank,     \
             status.MPI_SOURCE, local_time);                             \
                                                                         \
    } else if (status.MPI_TAG == release_tag) {                          \
      lq.Remove(status.MPI_SOURCE);                                      \
      std::cout << "RANK_" << rank << ": QUEUE (after RELEASE): " << lq  \
                << "| time: " << local_time << "\n\n";                   \
                                                                         \
    } else if (status.MPI_TAG == final_tag)                              \
      fins_amount++;                                                     \
  }

void Work(int rank, int seconds, LamportQueue& lq, int& fins_amount) {
  sigflag = 1;
  ALARM_RAND(seconds);

  while (sigflag) {
    int flag;
    CHECK_SUCCESS(MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag,
                             &status));

    if (flag) {
      int message_time;
      CHECK_SUCCESS(MPI_Recv(&message_time, 1, MPI_INT, status.MPI_SOURCE,
                             status.MPI_TAG, MPI_COMM_WORLD, &status));

      LAMPORT_UPDATE(message_time);

      HANDLE_MESSAGE;
    }
  }
}

void ProcessMessages(int rank, bool collect_permissions, int send_req_time,
                     std::vector<int>& perms, LamportQueue& lq,
                     int& fins_amount) {
  int flag;
  CHECK_SUCCESS(
      MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status));

  while (flag) {
    int message_time;
    CHECK_SUCCESS(MPI_Recv(&message_time, 1, MPI_INT, status.MPI_SOURCE,
                           status.MPI_TAG, MPI_COMM_WORLD, &status));
    LAMPORT_UPDATE(message_time);

    if (send_req_time < message_time && collect_permissions)
      perms[status.MPI_SOURCE] = 1;

    printf("RANK_%d: PERMS_AMOUNT: %d\n\n", rank, Sum(perms));

    HANDLE_MESSAGE;

    CHECK_SUCCESS(MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag,
                             &status));
  }
}

// MARK: MAIN

int main(int argc, char* argv[]) {
  MPI_Status status;
  int fins_amount = 0;

  CHECK_SUCCESS(MPI_Init(&argc, &argv));

  int ranks_amount;
  CHECK_SUCCESS(MPI_Comm_size(MPI_COMM_WORLD, &ranks_amount));

  LamportQueue lq(ranks_amount);
  std::vector<int> perms(ranks_amount, 0);

  int rank;
  CHECK_SUCCESS(MPI_Comm_rank(MPI_COMM_WORLD, &rank));

  signal(SIGALRM, Handler);

  srand(rank);

  for (int repeating_count = 0; repeating_count < repeat; repeating_count++) {
    printf("\n\tCURR_REPEAT: %d/%d (RANK_%d)\n\n\n", repeating_count + 1,
           repeat, rank);
    printf("RANK_%d: ENTER: prologue | time: %d\n\n", rank, local_time);

    LAMPORT_TICK;
    lq.Add({local_time, rank});

    std::cout << "RANK_" << rank << ": QUEUE (in prologue): " << lq
              << "| time: " << local_time << "\n\n";

    SendAll(rank, ranks_amount, request_tag);
    int send_req_time = local_time;

    while (Sum(perms) < ranks_amount - 1 || !lq.IsHead(rank))
      ProcessMessages(rank, true, send_req_time, perms, lq, fins_amount);

    printf("RANK_%d: ENTER: critical | time: %d\n\n", rank, local_time);

    Work(rank, critical_time, lq, fins_amount);

    printf("RANK_%d: ENTER: epilogue | time: %d\n\n", rank, local_time);

    lq.Remove(rank);
    std::cout << "RANK_" << rank << ": QUEUE (in epilogue): " << lq
              << "| time: " << local_time << "\n\n";

    SendAll(rank, ranks_amount, release_tag);

    printf("RANK_%d: ENTER: remainder | time: %d\n\n", rank, local_time);
    Work(rank, reminder_time, lq, fins_amount);
  }

  SendAll(rank, ranks_amount, final_tag);

  while (fins_amount < ranks_amount - 1)
    ProcessMessages(rank, false, -1, perms, lq, fins_amount);

  printf("RANK_%d: FINISHED with time: %d\n\n", rank, local_time);
  MPI_Finalize();

  return 0;
}
