#pragma once

#include <cstdint>

class CANMessage {
public:
  uint32_t id;
  uint8_t dlc;
  uint8_t data[8];

  CANMessage() : id(0), dlc(0) {
    for (int i = 0; i < 8; i++) data[i] = 0;
  }
};
