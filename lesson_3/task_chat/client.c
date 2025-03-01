#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char **argv) {
  int desk_sock, r_w_id, i, err;
  char send_line[1000], recv_line[1000];
  struct sockaddr_in serv_addr;

  unsigned short port;

  if (argc < 2 || argc > 3) {
    printf("Usage: ./a.out <IP address> <port - default 51000>\n");
    exit(1);
  }

  bzero(&serv_addr, sizeof(serv_addr));

  serv_addr.sin_family = AF_INET;

  if (argc == 3) {
    err = sscanf(argv[2], "%d", &port);

    if (err != 1 || port == 0) {
      printf("Invalid port\n");
      exit(-1);
    }
  } else
    port = 51000;

  printf("Port set to %d\n", port);

  bzero(send_line, 1000);
  bzero(recv_line, 1000);

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(port);

  if (inet_aton(argv[1], &serv_addr.sin_addr) == 0) {
    printf("Invalid IP address\n");
    exit(-1);
  }

  if ((desk_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    printf("Can\'t create socket, errno = %d\n", errno);
    exit(1);
  }

  if (connect(desk_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) <
      0) {
    printf("Can\'t connect, errno = %d\n", errno);
    close(desk_sock);
    exit(1);
  }

  for (i = 0; i < 5; i++) {
    printf("String => ");
    fflush(stdin);

    fgets(send_line, 1000, stdin);
    printf("\n");

    if (!strcmp(send_line, "EXIT\n")) {
      printf("Connection closed\n");
      close(desk_sock);
      break;
    }

    if ((r_w_id = write(desk_sock, send_line, strlen(send_line) + 1)) < 0) {
      printf("Can\'t write, errno = %d\n", errno);
      close(desk_sock);
      exit(1);
    }

    if ((r_w_id = read(desk_sock, recv_line, 1000)) <= 0) {
      if (r_w_id < 0) {
        printf("Can\'t read, errno = %d\n", errno);
        close(desk_sock);
        exit(1);

      } else {
        printf("Connection closed\n");
        close(desk_sock);

        break;
      }
    }

    printf("%s\n", recv_line);
  }

  close(desk_sock);
  return 0;
}
