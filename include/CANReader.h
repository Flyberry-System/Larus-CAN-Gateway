#pragma once

#include "CANMessage.h"
#include <cstdint>
#include <string>

class CANReader {
public:
  explicit CANReader(const std::string& iface);
  ~CANReader();

  CANReader(const CANReader&) = delete;
  CANReader& operator=(const CANReader&) = delete;

  bool init(uint32_t bitrate = 1000000);
  bool readMessage(CANMessage& msg);                      // liest einen Frame (blockierend)
  bool waitForMessage(CANMessage& msg, int timeout_ms);   // wartet per poll() auf einen Frame
  void stop();

private:
  std::string interface;
  int socket_fd;
};

