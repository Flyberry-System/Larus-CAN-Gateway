#include <iostream>
#include <thread>
#include <chrono>

#include "CANReader.h"
#include "CANtoLarusConverter.h"
#include "TCPServer.h"
#include "SerialPort.h"

int main(int argc, char* argv[]) {
  std::string can_interface = "can0";
  std::string output_mode = "tcp";
  std::string serial_port = "/dev/ttyUSB0";
  int tcp_port = 4353;  // XCSoar standard port

  if (argc > 1) can_interface = argv[1];
  if (argc > 2) output_mode = argv[2];
  if (argc > 3) serial_port = argv[3];
  if (argc > 4) tcp_port = std::atoi(argv[4]);

  std::cout << "\n╔════════════════════════════════════════╗" << std::endl;
  std::cout << "║  Larus CAN-to-NMEA Bridge (v2.2)      ║" << std::endl;
  std::cout << "║  XCSoar Compatible - Socket Ready! ✓  ║" << std::endl;
  std::cout << "╚════════════════════════════════════════╝\n" << std::endl;

  std::cout << "Configuration:" << std::endl;
  std::cout << "  CAN Interface: " << can_interface << std::endl;
  std::cout << "  Output Mode:   " << output_mode << std::endl;
  if (output_mode == "serial" || output_mode == "both") {
    std::cout << "  Serial Port:   " << serial_port << std::endl;
  }

  CANReader reader(can_interface);
  if (!reader.init()) return 1;

  // Initialize output handlers
  TCPServer* tcp_server = nullptr;
  SerialPort* serial_port_obj = nullptr;

  if (output_mode == "tcp" || output_mode == "both") {
    tcp_server = new TCPServer(tcp_port);
    tcp_server->start();
  }

  if (output_mode == "serial" || output_mode == "both") {
    serial_port_obj = new SerialPort(serial_port);
    if (!serial_port_obj->open_port()) {
      std::cerr << "⚠ Serial port failed, continuing with TCP only" << std::endl;
      delete serial_port_obj;
      serial_port_obj = nullptr;
    }
  }

  CANtoLarusConverter converter(tcp_server, serial_port_obj);

  CANMessage can_msg;
  std::cout << "\nPress Ctrl+C to exit\n" << std::endl;

  while (true) {
    if (reader.readMessage(can_msg)) {
      converter.convert(can_msg);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }

  delete tcp_server;
  delete serial_port_obj;
  return 0;
}
