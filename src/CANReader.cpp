#include "CANReader.h"
#include <iostream>
#include <cstring>

#ifdef __linux__
  #include <linux/can.h>
  #include <linux/can/raw.h>
  #include <net/if.h>
  #include <sys/socket.h>
  #include <sys/ioctl.h>
  #include <unistd.h>
#endif

CANReader::CANReader(const std::string& iface) : interface(iface), socket_fd(-1) {}

CANReader::~CANReader() {
  stop();
}

bool CANReader::init() {
#ifdef __linux__
  socket_fd = socket(PF_CAN, SOCK_RAW, CAN_RAW);
  if (socket_fd < 0) {
    std::cerr << "✗ CAN socket creation failed" << std::endl;
    return false;
  }

  struct ifreq ifr;
  strcpy(ifr.ifr_name, interface.c_str());
  if (ioctl(socket_fd, SIOCGIFINDEX, &ifr) < 0) {
    std::cerr << "✗ Interface " << interface << " not found" << std::endl;
    return false;
  }

  struct sockaddr_can addr;
  addr.can_family = AF_CAN;
  addr.can_ifindex = ifr.ifr_ifindex;

  if (bind(socket_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    std::cerr << "✗ CAN bind failed" << std::endl;
    return false;
  }

  std::cout << "✓ CAN interface initialized: " << interface << std::endl;
  return true;
#else
  std::cout << "✓ CAN interface (simulated): " << interface << std::endl;
  return true;
#endif
}

bool CANReader::readMessage(CANMessage& msg) {
#ifdef __linux__
  if (socket_fd < 0) return false;

  struct can_frame frame;
  int bytes = read(socket_fd, &frame, sizeof(struct can_frame));

  if (bytes > 0) {
    msg.id = frame.can_id;
    msg.dlc = frame.can_dlc;
    for (int i = 0; i < 8; i++) {
      msg.data[i] = frame.data[i];
    }
    return true;
  }
  return false;
#else
  return false;
#endif
}

void CANReader::stop() {
#ifdef __linux__
  if (socket_fd >= 0) {
    close(socket_fd);
    socket_fd = -1;
  }
#endif
}
