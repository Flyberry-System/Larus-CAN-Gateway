#pragma once

#include "CANMessage.h"
#include <string>

class CANReader {
private:
  int socket_fd;
  std::string interface;

public:
  CANReader(const std::string& iface = "can0");
  ~CANReader();

  bool init();
  bool readMessage(CANMessage& msg);
  void stop();
};
