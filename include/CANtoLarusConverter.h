#pragma once

#include "CANMessage.h"
#include "NMEAMessage.h"
#include "TCPServer.h"
#include "SerialPort.h"
#include <cstdint>
#include <fstream>

class CANtoLarusConverter {
public:
  CANtoLarusConverter(TCPServer* tcp = nullptr, SerialPort* serial = nullptr);
  ~CANtoLarusConverter();

  CANtoLarusConverter(const CANtoLarusConverter&) = delete;
  CANtoLarusConverter& operator=(const CANtoLarusConverter&) = delete;

  void convert(const CANMessage& can_msg);

private:
  static float bytesToFloat32(const uint8_t* data);
  void enqueue(const NMEAMessage& msg);   // sendet an TCP/Serial und loggt

  std::ofstream log_file;
  TCPServer* tcp_server;      // nicht besitzend
  SerialPort* serial_port;    // nicht besitzend

  struct SensorData {
    double pressure_alt = 0.0;   // m, aus 0x126
    double heading      = 0.0;   // Grad, 0-360
    double roll         = 0.0;   // Grad
    double pitch        = 0.0;   // Grad
    double tas_kph      = 0.0;
    double tek_vario    = 0.0;   // m/s
    double avg_vario    = 0.0;   // m/s
    double gload        = 1.0;
    double wind_bearing = 0.0;   // Grad
    double wind_speed   = 0.0;   // km/h
    double air_density  = 0.0;   // g/m³
    double voltage      = 0.0;   // V
  } sensors;
};