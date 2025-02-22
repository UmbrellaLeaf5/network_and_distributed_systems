/* A simple echo UDP server */

#include <arpa/inet.h>   // функции преобразования IP-адресов
#include <errno.h>       // определения номеров ошибок
#include <netinet/in.h>  // определения структур адресов интернет-протоколов
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int main() {
  int sockfd,  // дескриптор сокета
      clilen,  // длина адреса клиента
      n;       // количество полученных байт

  char line[1000];  // буфер для хранения данных
  // структуры для хранения адресов сервера и клиента
  struct sockaddr_in servaddr, cliaddr;

  // обнуление структуры адреса сервера
  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;  // установка семейства протоколов в IPv4
  // установка номера порта сервера
  // htons() преобразует порядок байтов из хостового в сетевой
  servaddr.sin_port = htons(52020);
  // установка IP-адреса сервера в любой доступный
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

  // создание UDP сокета
  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    printf("Can\'t create socket, errno = %d\n", errno);
    exit(1);
  }

  // привязка сокета к адресу сервера
  if (bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
    printf("Can\'t bind socket, errno = %d\n", errno);
    close(sockfd);
    exit(1);
  }

  // бесконечный цикл обработки входящих запросов
  while (1) {
    clilen = sizeof(cliaddr);  // инициализация длины адреса клиента

    // получение данных от клиента
    if ((n = recvfrom(sockfd, line, 1000, 0, (struct sockaddr *)&cliaddr,
                      &clilen)) < 0) {
      printf("Can\'t receive request, errno = %d\n", errno);
      close(sockfd);
      exit(1);
    }

    // вывод информации о клиенте и полученных данных
    printf("from %s %s\n", inet_ntoa(cliaddr.sin_addr), line);
    // inet_ntoa() преобразует IP-адрес из бинарного формата в текстовый

    // отправка полученных данных обратно клиенту (эхо)
    if (sendto(sockfd, line, strlen(line) + 1, 0, (struct sockaddr *)&cliaddr,
               clilen) < 0) {
      printf("Can\'t send answer, errno = %d\n", errno);
      close(sockfd);
      exit(1);
    }
  }

  return 0;
}