#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int main() {
  int sockfd, newsockfd, clilen, n;
  char line[1000];
  struct sockaddr_in servaddr, cliaddr;

  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(51000);
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    printf("Can\'t create socket, errno = %d\n", errno);
    exit(1);
  }

  if (bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
    printf("Can\'t bind socket, errno = %d\n", errno);
    close(sockfd);
    exit(1);
  }

  if (listen(sockfd, 5) < 0) {
    printf("Can\'t change state of socket to listen state, errno = %d\n",
           errno);
    close(sockfd);
    exit(1);
  }

  while (1) {
    clilen = sizeof(cliaddr);

    if ((newsockfd = accept(sockfd, (struct sockaddr *)&cliaddr, &clilen)) <
        0) {
      printf("Can\'t accept connection, errno = %d\n", errno);
      close(sockfd);
      exit(1);
    }

    printf("Connection with %s\n", inet_ntoa(cliaddr.sin_addr));

    bzero(line, 1000);
    while ((n = read(newsockfd, line, 1000)) > 0) {
      printf("%s\n", line);

      if ((n = write(newsockfd, line, strlen(line) + 1)) < 0) {
        printf("Can\'t write, errno = %d\n", errno);
        close(sockfd);
        close(newsockfd);
        exit(1);
      }

      bzero(line, 1000);
    }

    if (n < 0) {
      printf("Can\'t read, errno = %d\n", errno);
      close(sockfd);
      close(newsockfd);
      exit(1);
    }

    close(newsockfd);
    printf("Connection closed\n");
  }

  return 0;
}
