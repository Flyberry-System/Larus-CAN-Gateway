#include "SerialPort.h"
#include <iostream>

#ifdef __linux__
  #include <fcntl.h>
  #include <termios.h>
  #include <unistd.h>
#endif

SerialPort::SerialPort(const std::string& p)
  : fd(-1), port_name(p) {}

SerialPort::~SerialPort() {
  close_port();
}

bool SerialPort::open_port() {
#ifdef __linux__
  fd = open(port_name.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
  if (fd < 0) {
    std::cerr << "✗ Serial port '" << port_name << "' not found" << std::endl;
    return false;
  }

  fcntl(fd, F_SETFL, 0);

  struct termios tty;
  if (tcgetattr(fd, &tty) != 0) {
    std::cerr << "✗ tcgetattr failed" << std::endl;
    return false;
  }

  cfsetospeed(&tty, B9600);
  cfsetispeed(&tty, B9600);
  tty.c_cflag &= ~PARENB;
  tty.c_cflag &= ~CSTOPB;
  tty.c_cflag &= ~CSIZE;
  tty.c_cflag |= CS8;
  tty.c_iflag &= ~(IXON | IXOFF | IXANY);
  tty.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
  tty.c_oflag &= ~OPOST;

  if (tcsetattr(fd, TCSANOW, &tty) != 0) {
    std::cerr << "✗ tcsetattr failed" << std::endl;
    return false;
  }

  std::cout << "✓ Serial port opened: " << port_name << " @ 9600 baud" << std::endl;
  std::cout << "  XCSoar: Configure as Serial Device" << std::endl;
  return true;
#else
  std::cout << "⚠ Serial port not available on Windows" << std::endl;
  return false;
#endif
}

void SerialPort::send(const std::string& msg) {
#ifdef __linux__
  if (fd >= 0) {
    ssize_t n = write(fd, msg.c_str(), msg.length());
    if (n < 0) {
      std::cerr << "✗ Serial write failed" << std::endl;
    }
  }
#endif
}

void SerialPort::close_port() {
#ifdef __linux__
  if (fd >= 0) {
    close(fd);
    fd = -1;
  }
#endif
}
