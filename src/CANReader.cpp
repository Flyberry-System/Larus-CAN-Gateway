#include "CANReader.h"
#include <iostream>
#include <cstring>

#include <linux/can.h>
#include <linux/can/raw.h>
#include <net/if.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <poll.h>
#include <unistd.h>
#include <libsocketcan.h>

CANReader::CANReader(const std::string& iface) : interface(iface), socket_fd(-1) {}

CANReader::~CANReader() {
  stop();
}

bool CANReader::init(uint32_t bitrate) {
  // Bitrate setzen (Interface muss dafür down sein)
  can_do_stop(interface.c_str());
  if (can_set_bitrate(interface.c_str(), bitrate) < 0) {
    // z.B. bei vcan0 nicht unterstützt, oder fehlende Rechte
    std::cerr << "⚠ Could not set bitrate on " << interface
              << " (vcan or missing CAP_NET_ADMIN?), continuing" << std::endl;
  }
  if (can_do_start(interface.c_str()) < 0) {
    std::cerr << "⚠ Could not bring up " << interface << std::endl;
  }

  socket_fd = socket(PF_CAN, SOCK_RAW, CAN_RAW);
  if (socket_fd < 0) {
    std::cerr << "✗ CAN socket creation failed" << std::endl;
    return false;
  }

  // ... ab hier unverändert (ifr, ioctl, bind, rcvbuf)

  std::cout << "✓ CAN interface initialized: " << interface
            << " @ " << bitrate << " bit/s" << std::endl;
  return true;
}

bool CANReader::readMessage(CANMessage& msg) {
  if (socket_fd < 0) return false;

  struct can_frame frame;
  ssize_t bytes = read(socket_fd, &frame, sizeof(frame));
  if (bytes != (ssize_t)sizeof(frame)) return false;

  msg.id  = frame.can_id & CAN_EFF_MASK;   // EFF/RTR/ERR-Flags entfernen
  msg.dlc = frame.can_dlc;
  memset(msg.data, 0, sizeof(msg.data));
  memcpy(msg.data, frame.data, frame.can_dlc > 8 ? 8 : frame.can_dlc);
  return true;
}

bool CANReader::waitForMessage(CANMessage& msg, int timeout_ms) {
  if (socket_fd < 0) return false;

  struct pollfd pfd;
  pfd.fd = socket_fd;
  pfd.events = POLLIN;
  pfd.revents = 0;

  int r = poll(&pfd, 1, timeout_ms);
  if (r <= 0) return false;                 // Timeout oder Signal (EINTR)
  if (!(pfd.revents & POLLIN)) return false;

  return readMessage(msg);
}

void CANReader::stop() {
  if (socket_fd >= 0) {
    close(socket_fd);
    socket_fd = -1;
  }
}