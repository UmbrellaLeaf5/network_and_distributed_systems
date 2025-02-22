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

// Владимир Ефимович:
// ./a.out 10.55.162.165 51000

// Я:
// ./a.out 10.55.165.200 52020

struct SendData {
  int N;  // кол-во отрезков
  int beg;
  int end;
};

int main(int argc, char** argv) {
  if (argc < 2 || argc > 3) {
    std::cout << "Usage: ./a.out <IP address> <port - default 51000>"
              << std::endl;

    return 1;
  }

  unsigned short port = 51000;

  if (argc == 3) try {
      port = std::atoi(argv[2]);

    } catch (const std::exception& e) {
      std::cerr << "Invalid port, exception: " << e.what() << std::endl;
      return -1;
    }

  // структуры для хранения адресов сервера и клиента
  sockaddr_in server_addr, client_addr;

  // обнуление структуры адреса сервера
  std::memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;  // установка семейства протоколов в IPv4

  // установка номера порта в структуре адреса сервера
  // htons() преобразует порядок байтов из хостового в сетевой
  server_addr.sin_port = htons(port);

  // преобразование IP-адреса из текстового формата в бинарный
  if (inet_aton(argv[1], &server_addr.sin_addr) == 0) {
    std::cerr << "Invalid IP address!" << std::endl;
    return -1;
  }

  // создание UDP сокета
  int desk_sock = socket(AF_INET, SOCK_DGRAM, 0);  // дескриптор сокета
  if (desk_sock < 0) {
    std::cerr << "Can't create socket, " << strerror(errno) << std::endl;
    return 1;
  }

  // обнуление структуры адреса клиента
  std::memset(&client_addr, 0, sizeof(client_addr));

  client_addr.sin_family = AF_INET;  // установка семейства протоколов в IPv4

  // установка порта клиента в 0 (автоматический выбор)
  client_addr.sin_port = htons(0);

  // установка IP-адреса клиента в любой доступный
  client_addr.sin_addr.s_addr = htonl(INADDR_ANY);

  // привязка сокета к адресу клиента
  ssize_t bytes_bind =
      bind(desk_sock, (struct sockaddr*)&client_addr, sizeof(client_addr));

  if (bytes_bind < 0) {
    std::cerr << "Can't bind socket, " << strerror(errno) << std::endl;
    close(desk_sock);  // закрытие сокета
    return 1;
  }

  SendData data;

  int k;  // кол-во запросов
  std::cout << "Enter N and k: " << std::endl;
  std::cin >> data.N >> k;

  double Pi = 0;

  for (int i = 0; i < k; i++) {
    data.beg = data.N * i / k;
    data.end = data.N * (i + 1) / k - 1;

    // отправка серверу структуры {N, beg, end}
    ssize_t bytes_send =
        sendto(desk_sock, &data, sizeof(data), 0,
               (const struct sockaddr*)&server_addr, sizeof(server_addr));

    if (bytes_send < 0) {
      std::cerr << "Error sending, " << strerror(errno) << std::endl;
      close(desk_sock);
      return 1;
    }

    // получение данных от сервера
    double received;
    ssize_t bytes_received = recvfrom(desk_sock, &received, sizeof(double), 0,
                                      (struct sockaddr*)NULL, NULL);

    if (bytes_received < 0) {
      std::cerr << "Receive failed: " << strerror(errno) << std::endl;
      close(desk_sock);
      return 1;
    }

    std::cout << "K: " << i << ", part of Pi: " << std::fixed
              << std::setprecision(6) << received << std::endl;

    Pi += received;
  }

  std::cout << "Pi: " << std::fixed << std::setprecision(6) << Pi << std::endl;

  close(desk_sock);
  return 0;
}
