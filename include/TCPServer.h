#pragma once

#include <string>

class TCPServer {
private:
  int server_socket;
  int client_socket;
  int port;
  bool running;

  void acceptConnections();

public:
  TCPServer(int p);
  ~TCPServer();

  bool start();
  void send(const std::string& msg);
  void stop();
};
