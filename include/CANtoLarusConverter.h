#pragma once

#include "CANMessage.h"
#include "NMEAMessage.h"
#include "TCPServer.h"
#include "SerialPort.h"
#include <queue>
#include <mutex>
#include <fstream>

class CANtoLarusConverter {
private:
  std::queue<NMEAMessage> nmea_queue;
  std::mutex queue_mutex;
  std::ofstream log_file;
  TCPServer* tcp_server;
  SerialPort* serial_port;

  struct SensorData {
    double latitude = 0.0;
    double longitude = 0.0;
    double altitude = 0.0;
    double pressure_alt = 0.0;
    double heading = 0.0;
    double roll = 0.0;
    double pitch = 0.0;
    double speed = 0.0;
    double course = 0.0;
    double tas_kph = 0.0;
    double tek_vario = 0.0;
    double avg_vario = 0.0;
    double gload = 1.0;
    double wind_bearing = 0.0;
    double wind_speed = 0.0;
    double air_density = 1.225;
    double voltage = 12.0;
    double temperature = 15.0;
    double humidity = 50.0;
  } sensors;

  static float bytesToFloat32(const uint8_t* data);
  void convertLarusOfficial(const CANMessage& can_msg);

public:
  CANtoLarusConverter(TCPServer* tcp = nullptr, SerialPort* serial = nullptr);
  ~CANtoLarusConverter();

  void convert(const CANMessage& can_msg);
  void enqueue(const NMEAMessage& msg);
  bool dequeue(NMEAMessage& msg);
};
