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

  // создание TCP сокета
  ssize_t desk_sock = socket(AF_INET, SOCK_STREAM, 0);  // дескриптор сокета
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

  ssize_t listen_id = listen(desk_sock, 5);

  if (listen_id < 0) {
    std::cerr << "Can't change state of socket to listen state, "
              << strerror(errno) << std::endl;
    close(desk_sock);

    return 1;
  }

  for (;;) {
    size_t client_len = sizeof(client_addr);

    ssize_t curr_desk_sock =
        accept(desk_sock, (struct sockaddr*)&client_addr, &client_len);

    if (curr_desk_sock < 0) {
      std::cerr << "Can't accept connection, " << strerror(errno) << std::endl;
      close(desk_sock);

      return 1;
    }

    std::cout << "Connection with " << inet_ntoa(client_addr.sin_addr)
              << std::endl;

    char line[1000];
    std::memset(&line, 0, 1000);

    ssize_t r_w_id;
    while ((r_w_id = read(curr_desk_sock, line, 1000)) > 0) {
      std::cout << line << std::endl;

      std::cout << "String => ";
      std::string new_line;
      std::getline(std::cin, new_line);

      if (new_line == "EXIT") {
        close(curr_desk_sock);

        break;
      }

      r_w_id =
          write(curr_desk_sock, new_line.c_str(), strlen(new_line.c_str()) + 1);
      if (r_w_id < 0) {
        std::cerr << "Can't write, " << strerror(errno) << std::endl;
        close(desk_sock);
        close(curr_desk_sock);

        return 1;
      }

      std::memset(&line, 0, 1000);
    }

    if (r_w_id < 0) {
      std::cerr << "Can't read, " << strerror(errno) << std::endl;
      close(desk_sock);
      close(curr_desk_sock);

      return 1;
    }

    close(curr_desk_sock);
    std::cout << "Connection closed!" << std::endl;
  }

  return 0;
}
