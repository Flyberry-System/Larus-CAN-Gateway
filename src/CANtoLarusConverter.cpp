#include "CANtoLarusConverter.h"
#include <iostream>
#include <cstring>
#include <ctime>
#include <cmath>

CANtoLarusConverter::CANtoLarusConverter(TCPServer* tcp, SerialPort* serial)
  : tcp_server(tcp), serial_port(serial) {
  log_file.open("larus_gateway.log", std::ios::app);
}

CANtoLarusConverter::~CANtoLarusConverter() {
  if (log_file.is_open()) log_file.close();
}

float CANtoLarusConverter::bytesToFloat32(const uint8_t* data) {
  uint32_t bits = (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3];
  float result;
  memcpy(&result, &bits, sizeof(float));
  return result;
}

void CANtoLarusConverter::enqueue(const NMEAMessage& msg) {
  std::lock_guard<std::mutex> lock(queue_mutex);
  nmea_queue.push(msg);

  // Output to all handlers
  if (tcp_server) tcp_server->send(msg.sentence);
  if (serial_port) serial_port->send(msg.sentence);

  std::cout << msg.sentence;

  if (log_file.is_open()) {
    time_t now = time(nullptr);
    struct tm *tm_info = localtime(&now);
    char timestamp[30];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_info);
    log_file << "[" << timestamp << "] " << msg.sentence;
    log_file.flush();
  }
}

bool CANtoLarusConverter::dequeue(NMEAMessage& msg) {
  std::lock_guard<std::mutex> lock(queue_mutex);
  if (nmea_queue.empty()) return false;
  msg = nmea_queue.front();
  nmea_queue.pop();
  return true;
}

void CANtoLarusConverter::convertLarusOfficial(const CANMessage& can_msg) {
  NMEAMessage nmea_msg;

  switch (can_msg.id) {
    case 0x120:
    {
      float roll_rad = bytesToFloat32(&can_msg.data[0]);
      float pitch_rad = bytesToFloat32(&can_msg.data[4]);
      sensors.roll = roll_rad * 180.0 / M_PI;
      sensors.pitch = pitch_rad * 180.0 / M_PI;
    }
    break;

    case 0x121:
    {
      float yaw_rad = bytesToFloat32(&can_msg.data[0]);
      sensors.heading = yaw_rad * 180.0 / M_PI;
      nmea_msg.sentence = NMEAMessage::createPLARA(sensors.roll, sensors.pitch, sensors.heading);
      enqueue(nmea_msg);
    }
    break;

    case 0x122:
    {
      float tas_ms = bytesToFloat32(&can_msg.data[0]);
      float ias_ms = bytesToFloat32(&can_msg.data[4]);
      sensors.tas_kph = tas_ms * 3.6;
      sensors.speed = ias_ms * 1.9438;
    }
    break;

    case 0x123:
    {
      float vario_ms = bytesToFloat32(&can_msg.data[0]);
      float avg_vario_ms = bytesToFloat32(&can_msg.data[4]);
      sensors.tek_vario = vario_ms;
      sensors.avg_vario = avg_vario_ms;
      nmea_msg.sentence = NMEAMessage::createPLARV(sensors.tek_vario, sensors.avg_vario,
                                                    sensors.pressure_alt, sensors.tas_kph,
                                                    sensors.gload);
      enqueue(nmea_msg);
    }
    break;

    case 0x124:
    {
      float wind_dir_rad = bytesToFloat32(&can_msg.data[0]);
      float wind_speed_ms = bytesToFloat32(&can_msg.data[4]);
      sensors.wind_bearing = wind_dir_rad * 180.0 / M_PI;
      sensors.wind_speed = wind_speed_ms * 3.6;
      nmea_msg.sentence = NMEAMessage::createPLARW(sensors.wind_bearing, sensors.wind_speed, false, true);
      enqueue(nmea_msg);
    }
    break;

    case 0x125:
    {
      float avg_wind_dir_rad = bytesToFloat32(&can_msg.data[0]);
      float avg_wind_speed_ms = bytesToFloat32(&can_msg.data[4]);
      double avg_bearing = avg_wind_dir_rad * 180.0 / M_PI;
      double avg_speed = avg_wind_speed_ms * 3.6;
      nmea_msg.sentence = NMEAMessage::createPLARW(avg_bearing, avg_speed, true, true);
      enqueue(nmea_msg);
    }
    break;

    case 0x126:
    {
      float pressure_pa = bytesToFloat32(&can_msg.data[0]);
      float density_kgm3 = bytesToFloat32(&can_msg.data[4]);
      sensors.air_density = density_kgm3;
      nmea_msg.sentence = NMEAMessage::createPLARD(sensors.air_density, true);
      enqueue(nmea_msg);
    }
    break;

    case 0x127:
    {
      float g_force_ms2 = bytesToFloat32(&can_msg.data[0]);
      sensors.gload = g_force_ms2 / 9.81;
    }
    break;

    case 0x129:
    {
      float supply_voltage = bytesToFloat32(&can_msg.data[0]);
      uint8_t circle_mode = can_msg.data[4];
      sensors.voltage = supply_voltage;

      int circling = (circle_mode == 2) ? 1 : 0;
      nmea_msg.sentence = NMEAMessage::createPLARB(sensors.voltage, sensors.temperature, sensors.humidity);
      enqueue(nmea_msg);

      nmea_msg.sentence = NMEAMessage::createPLARS_CIR(circling);
      enqueue(nmea_msg);
    }
    break;

    default:
      break;
  }
}

void CANtoLarusConverter::convert(const CANMessage& can_msg) {
  NMEAMessage nmea_msg;

  // Support BOTH simplified (0x100-0x107) AND official spec (0x120-0x12f)
  if (can_msg.id >= 0x120 && can_msg.id <= 0x12f) {
    convertLarusOfficial(can_msg);
    return;
  }

  // === SIMPLIFIED MODE (0x100-0x107) ===
  switch (can_msg.id) {
    case 0x100:
    {
      int32_t lat_raw = (can_msg.data[0] << 24) | (can_msg.data[1] << 16) |
                        (can_msg.data[2] << 8) | can_msg.data[3];
      int32_t lon_raw = (can_msg.data[4] << 24) | (can_msg.data[5] << 16) |
                        (can_msg.data[6] << 8) | can_msg.data[7];
      sensors.latitude = lat_raw / 1000000.0;
      sensors.longitude = lon_raw / 1000000.0;
      nmea_msg.sentence = NMEAMessage::createGPGGA(sensors.latitude, sensors.longitude,
                                                    12, 0.9, sensors.altitude);
      enqueue(nmea_msg);
    }
    break;

    case 0x101:
    {
      uint16_t speed_raw = (can_msg.data[0] << 8) | can_msg.data[1];
      uint16_t course_raw = (can_msg.data[2] << 8) | can_msg.data[3];
      sensors.speed = speed_raw / 100.0;
      sensors.course = course_raw / 10.0;
      nmea_msg.sentence = NMEAMessage::createGRMC(sensors.latitude, sensors.longitude,
                                                   sensors.speed, sensors.course);
      enqueue(nmea_msg);
    }
    break;

    case 0x103:
    {
      int16_t roll_raw = (can_msg.data[0] << 8) | can_msg.data[1];
      int16_t pitch_raw = (can_msg.data[2] << 8) | can_msg.data[3];
      int16_t yaw_raw = (can_msg.data[4] << 8) | can_msg.data[5];
      sensors.roll = roll_raw / 10.0;
      sensors.pitch = pitch_raw / 10.0;
      sensors.heading = yaw_raw / 10.0;
      nmea_msg.sentence = NMEAMessage::createPLARA(sensors.roll, sensors.pitch, sensors.heading);
      enqueue(nmea_msg);
    }
    break;

    case 0x104:
    {
      int16_t tek_raw = (can_msg.data[0] << 8) | can_msg.data[1];
      int16_t avg_raw = (can_msg.data[2] << 8) | can_msg.data[3];
      int16_t alt_raw = (can_msg.data[4] << 8) | can_msg.data[5];
      uint16_t tas_raw = (can_msg.data[6] << 8) | can_msg.data[7];
      sensors.tek_vario = tek_raw / 100.0;
      sensors.avg_vario = avg_raw / 100.0;
      sensors.pressure_alt = alt_raw;
      sensors.tas_kph = tas_raw;
      nmea_msg.sentence = NMEAMessage::createPLARV(sensors.tek_vario, sensors.avg_vario,
                                                    sensors.pressure_alt, sensors.tas_kph,
                                                    sensors.gload);
      enqueue(nmea_msg);
    }
    break;

    case 0x105:
    {
      uint16_t bearing_raw = (can_msg.data[0] << 8) | can_msg.data[1];
      uint16_t speed_raw = (can_msg.data[2] << 8) | can_msg.data[3];
      sensors.wind_bearing = bearing_raw / 10.0;
      sensors.wind_speed = speed_raw / 10.0;
      nmea_msg.sentence = NMEAMessage::createPLARW(sensors.wind_bearing, sensors.wind_speed,
                                                    false, true);
      enqueue(nmea_msg);
    }
    break;

    case 0x106:
    {
      uint16_t voltage_raw = (can_msg.data[0] << 8) | can_msg.data[1];
      int16_t temp_raw = (can_msg.data[2] << 8) | can_msg.data[3];
      sensors.voltage = voltage_raw / 100.0;
      sensors.temperature = temp_raw / 10.0;
      nmea_msg.sentence = NMEAMessage::createPLARB(sensors.voltage, sensors.temperature, sensors.humidity);
      enqueue(nmea_msg);
    }
    break;

    case 0x107:
    {
      uint16_t density_raw = (can_msg.data[0] << 8) | can_msg.data[1];
      sensors.air_density = density_raw / 10.0;
      nmea_msg.sentence = NMEAMessage::createPLARD(sensors.air_density, true);
      enqueue(nmea_msg);
    }
    break;

    default:
      std::cout << "⚠ Unmapped CAN ID: 0x" << std::hex << can_msg.id << std::dec << std::endl;
  }
}
