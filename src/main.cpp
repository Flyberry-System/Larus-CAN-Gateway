#include <atomic>
#include <chrono>
#include <condition_variable>
#include <csignal>
#include <cstdlib>
#include <iostream>
#include <mutex>
#include <queue>
#include <string>
#include <thread>

#include "CANReader.h"
#include "CANtoLarusConverter.h"
#include "TCPServer.h"
#include "SerialPort.h"

static std::atomic<bool> running{true};

static void onSignal(int) {
  running = false;
}

// Thread-sichere Queue zwischen CAN-Reader und Converter
class MessageQueue {
public:
  void push(const CANMessage& m) {
    {
      std::lock_guard<std::mutex> lk(mtx_);
      if (q_.size() >= kMaxSize) q_.pop();   // bei Überlauf älteste verwerfen
      q_.push(m);
    }
    cv_.notify_one();
  }

  bool pop(CANMessage& m, int timeout_ms) {
    std::unique_lock<std::mutex> lk(mtx_);
    if (!cv_.wait_for(lk, std::chrono::milliseconds(timeout_ms),
                      [&] { return !q_.empty(); })) {
      return false;
    }
    m = q_.front();
    q_.pop();
    return true;
  }

private:
  static constexpr size_t kMaxSize = 1024;
  std::queue<CANMessage> q_;
  std::mutex mtx_;
  std::condition_variable cv_;
};

int main(int argc, char* argv[]) {
  std::string can_interface = "can0";
  std::string output_mode = "tcp";
  std::string serial_port = "/dev/ttyUSB0";
  int tcp_port = 2000;

  if (argc > 1) can_interface = argv[1];
  if (argc > 2) output_mode = argv[2];
  if (argc > 3) serial_port = argv[3];
  if (argc > 4) tcp_port = std::atoi(argv[4]);

  std::cout << "\n╔════════════════════════════════════════╗" << std::endl;
  std::cout << "║  Larus CAN-to-NMEA Gateway (v2.2)     ║" << std::endl;
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

  std::signal(SIGINT, onSignal);
  std::signal(SIGTERM, onSignal);

  std::cout << "\nPress Ctrl+C to exit\n" << std::endl;

  MessageQueue queue;

  // Producer: liest nur vom CAN-Socket und legt in die Queue
  std::thread reader_thread([&] {
    CANMessage msg;
    while (running) {
      if (reader.waitForMessage(msg, 100)) queue.push(msg);
    }
  });

  // Consumer: konvertiert und sendet (im Hauptthread)
  CANMessage can_msg;
  while (running) {
    if (queue.pop(can_msg, 100)) converter.convert(can_msg);
  }

  reader_thread.join();
  reader.stop();

  delete tcp_server;
  delete serial_port_obj;

  std::cout << "Gateway beendet." << std::endl;
  return 0;
}