#pragma once

#include <string>

class SerialPort {
private:
  int fd;
  std::string port_name;

public:
  SerialPort(const std::string& p);
  ~SerialPort();

  bool open_port();
  void send(const std::string& msg);
  void close_port();
};
