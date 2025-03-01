#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <string>

#include "pi.hpp"

// ./a.out 52020

struct SendData {
  int N;  // кол-во отрезков
  int beg;
  int end;
};

int main(int argc, char** argv) {
  if (argc != 2) {
    std::cout << "Usage: ./a.out <port>" << std::endl;

    return 1;
  }

  unsigned short port;

  try {
    port = std::atoi(argv[1]);

  } catch (const std::exception& e) {
    std::cerr << "Invalid port, exception: " << e.what() << std::endl;
    return -1;
  }

  // структуры для хранения адресов сервера и клиента
  sockaddr_in server_addr, client_addr;

  // обнуление структуры адреса сервера
  std::memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;  // установка семейства протоколов в IPv4

  // установка номера порта сервера
  // htons() преобразует порядок байтов из хостового в сетевой
  server_addr.sin_port = htons(port);

  // установка IP-адреса сервера в любой доступный
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

  // создание UDP сокета
  ssize_t desk_sock = socket(AF_INET, SOCK_DGRAM, 0);  // дескриптор сокета
  if (desk_sock < 0) {
    std::cerr << "Can't create socket, " << strerror(errno) << std::endl;
    return 1;
  }

  // привязка сокета к адресу сервера
  ssize_t bytes_bind =
      bind(desk_sock, (struct sockaddr*)&server_addr, sizeof(server_addr));

  if (bytes_bind < 0) {
    std::cerr << "Can't bind socket, " << strerror(errno) << std::endl;
    close(desk_sock);  // закрытие сокета
    return 1;
  }

  // бесконечный цикл обработки входящих запросов
  for (;;) {
    SendData data;

    size_t client_len = sizeof(client_addr);

    // получение данных от клиента
    ssize_t bytes_received =
        recvfrom(desk_sock, &data, sizeof(SendData), 0,
                 (struct sockaddr*)&client_addr, &client_len);

    if (bytes_received < 0) {
      std::cerr << "Receive failed: " << strerror(errno) << std::endl;
      close(desk_sock);
      return 1;
    }

    double sended = PartOfPi(data.N, data.beg, data.end);

    // вывод информации о клиенте и полученных данных
    std::cout << "Data: " << std::endl
              << "\tfrom: " << inet_ntoa(client_addr.sin_addr)
              << std::endl  // inet_ntoa() преобразует IP-адрес из бинарного
                            // формата в текстовый
              << "\tN: " << data.N << std::endl
              << "\t[beg, end]: [" << data.beg << ", " << data.end << "]"
              << std::endl
              << "Part of Pi: " << std::fixed << std::setprecision(6) << sended
              << std::endl
              << std::endl;

    // отправка клиенту частичной суммы
    ssize_t bytes_send = sendto(desk_sock, &sended, sizeof(double), 0,
                                (struct sockaddr*)&client_addr, client_len);
    if (bytes_send < 0) {
      std::cout << "Can't send answer, " << strerror(errno) << std::endl;
      close(desk_sock);
      return 1;
    }
  }

  return 0;
}
