/* A simple echo UDP client */

#include <arpa/inet.h>   // функции преобразования IP-адресов
#include <errno.h>       // определения номеров ошибок
#include <netinet/in.h>  // определения структур адресов интернет-протоколов
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char **argv) {
  int sockfd,  // дескриптор сокета
      n,       // количество полученных байт
      len,     // длина данных
      err;     // для хранения результатов работы некоторых функций

  // буферы для отправки и получения данных
  char sendline[1000], recv_line[1000];

  // структуры для хранения адресов сервера и клиента
  struct sockaddr_in servaddr, cliaddr;
  unsigned short port;  // номер порта

  if (argc < 2 || argc > 3) {
    printf("Usage: a.out <IP address> <port - default 51000>\n");
    exit(1);
  }

  // обнуление структуры адреса сервера
  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;  // установка семейства протоколов в IPv4

  // обработка аргумента порта, если он указан
  if (argc == 3) {
    // чтение номера порта из второго аргумента
    err = sscanf(argv[2], "%d", &port);
    if (err != 1 || port == 0) {  // проверка корректности введенного порта
      printf("Invalid port\n");
      exit(-1);
    }

  } else
    port = 51000;  // порт по умолчанию

  printf("Port set to %d\n", port);

  // установка номера порта в структуре адреса сервера
  // htons() преобразует порядок байтов из хостового в сетевой
  servaddr.sin_port = htons(port);

  // преобразование IP-адреса из текстового формата в бинарный
  if (inet_aton(argv[1], &servaddr.sin_addr) == 0) {
    printf("Invalid IP address\n");
    exit(-1);
  }

  // создание UDP сокета
  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    printf("Can\'t create socket, errno = %d\n", errno);
    exit(1);
  }

  // обнуление структуры адреса клиента
  bzero(&cliaddr, sizeof(cliaddr));

  cliaddr.sin_family = AF_INET;  // установка семейства протоколов в IPv4

  // установка порта клиента в 0 (автоматический выбор)
  cliaddr.sin_port = htons(0);

  // установка IP-адреса клиента в любой доступный
  cliaddr.sin_addr.s_addr = htonl(INADDR_ANY);

  // привязка сокета к адресу клиента
  if (bind(sockfd, (struct sockaddr *)&cliaddr, sizeof(cliaddr)) < 0) {
    printf("Can\'t bind socket, errno = %d\n", errno);
    close(sockfd);  // закрытие сокета
    exit(1);
  }

  printf("String => ");
  fgets(sendline, 1000, stdin);  // чтение строки из стандартного ввода

  // отправка данных на сервер
  if (sendto(sockfd, sendline, strlen(sendline) + 1, 0,
             (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
    printf("Can\'t send request, errno = %d\n", errno);
    close(sockfd);
    exit(1);
  }

  // получение данных от сервера
  if ((n = recvfrom(sockfd, recv_line, 1000, 0, (struct sockaddr *)NULL,
                    NULL)) < 0) {
    printf("Can\'t receive answer, errno = %d\n", errno);
    close(sockfd);
    exit(1);
  }

  printf("%s\n", recv_line);
  close(sockfd);

  return 0;
}