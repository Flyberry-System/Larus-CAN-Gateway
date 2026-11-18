#include "TCPServer.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <cstring>

#ifdef __linux__
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  #include <unistd.h>
#endif

TCPServer::TCPServer(int p)
  : server_socket(-1), client_socket(-1), port(p), running(false) {}

TCPServer::~TCPServer() {
  stop();
}

bool TCPServer::start() {
  server_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (server_socket < 0) {
    std::cerr << "✗ TCP socket creation failed" << std::endl;
    return false;
  }

  int reuse = 1;
  setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(port);

  if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
    std::cerr << "✗ TCP bind failed on port " << port << std::endl;
    return false;
  }

  if (listen(server_socket, 1) < 0) {
    std::cerr << "✗ TCP listen failed" << std::endl;
    return false;
  }

  running = true;
  std::cout << "✓ TCP Server listening on 127.0.0.1:" << port << std::endl;
  std::cout << "  XCSoar: Connect to tcp://127.0.0.1:" << port << std::endl;

  std::thread(&TCPServer::acceptConnections, this).detach();
  return true;
}

void TCPServer::send(const std::string& msg) {
#ifdef __linux__
  if (client_socket >= 0) {
    ssize_t n = write(client_socket, msg.c_str(), msg.length());
    if (n < 0) {
      std::cerr << "✗ TCP write failed, reconnecting..." << std::endl;
      close(client_socket);
      client_socket = -1;
    }
  }
#endif
}

void TCPServer::stop() {
  running = false;
#ifdef __linux__
  if (client_socket >= 0) close(client_socket);
  if (server_socket >= 0) close(server_socket);
#endif
}

void TCPServer::acceptConnections() {
#ifdef __linux__
  while (running) {
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);

    int new_socket = accept(server_socket, (struct sockaddr *)&client_addr, &addr_len);
    if (new_socket >= 0) {
      if (client_socket >= 0) close(client_socket);
      client_socket = new_socket;

      char ip[INET_ADDRSTRLEN];
      inet_ntop(AF_INET, &client_addr.sin_addr, ip, INET_ADDRSTRLEN);
      std::cout << "✓ XCSoar connected from " << ip << ":" << ntohs(client_addr.sin_port) << std::endl;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
#endif
}
