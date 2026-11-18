#include "CANtoLarusConverter.h"
#include <chrono>
#include <cmath>
#include <cstring>
#include <ctime>
#include <iostream>

namespace {

double toDeg360(double rad) {
  double d = std::fmod(rad * 180.0 / M_PI, 360.0);
  return d < 0 ? d + 360.0 : d;
}

uint32_t be32(const uint8_t* d) {
  return (uint32_t(d[0]) << 24) | (uint32_t(d[1]) << 16) |
         (uint32_t(d[2]) << 8)  |  uint32_t(d[3]);
}

}  // namespace

CANtoLarusConverter::CANtoLarusConverter(TCPServer* tcp, SerialPort* serial)
  : tcp_server(tcp), serial_port(serial) {
  log_file.open("larus_gateway.log", std::ios::app);
}

CANtoLarusConverter::~CANtoLarusConverter() {
  if (log_file.is_open()) log_file.close();
}

float CANtoLarusConverter::bytesToFloat32(const uint8_t* data) {
  uint32_t bits = be32(data);
  float result;
  memcpy(&result, &bits, sizeof(float));
  return result;
}

void CANtoLarusConverter::enqueue(const NMEAMessage& msg) {
  if (tcp_server)  tcp_server->send(msg.sentence);
  if (serial_port) serial_port->send(msg.sentence);

  std::cout << msg.sentence;

  if (log_file.is_open()) {
    time_t now = time(nullptr);
    struct tm tm_info;
    localtime_r(&now, &tm_info);
    char timestamp[30];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", &tm_info);
    log_file << "[" << timestamp << "] " << msg.sentence;
    log_file.flush();
  }
}

// Larus Sensor Box, CAN-IDs 0x120 - 0x12f
void CANtoLarusConverter::convert(const CANMessage& can_msg) {
  using clock = std::chrono::steady_clock;
  static clock::time_point last_plard, last_plarb;
  static int last_circling = -1;

  const auto now = clock::now();
  auto due = [&](clock::time_point& last, int interval_ms) {
    if (now - last < std::chrono::milliseconds(interval_ms)) return false;
    last = now;
    return true;
  };

  NMEAMessage nmea_msg;

  switch (can_msg.id) {

    case 0x120:   // Roll / Pitch
    {
      sensors.roll  = bytesToFloat32(&can_msg.data[0]) * 180.0 / M_PI;
      sensors.pitch = bytesToFloat32(&can_msg.data[4]) * 180.0 / M_PI;
    }
    break;

    case 0x121:   // Yaw / Turn rate -> $PLARA
    {
      sensors.heading = toDeg360(bytesToFloat32(&can_msg.data[0]));
      nmea_msg.sentence = NMEAMessage::createPLARA(sensors.roll, sensors.pitch, sensors.heading);
      enqueue(nmea_msg);
    }
    break;

    case 0x122:   // TAS / IAS
    {
      sensors.tas_kph = bytesToFloat32(&can_msg.data[0]) * 3.6;
      // IAS wird in keinem NMEA-Satz gebraucht.
    }
    break;

    case 0x123:   // Vario -> $PLARV
    {
      sensors.tek_vario = bytesToFloat32(&can_msg.data[0]);
      sensors.avg_vario = bytesToFloat32(&can_msg.data[4]);
      nmea_msg.sentence = NMEAMessage::createPLARV(sensors.tek_vario, sensors.avg_vario,
                                                    sensors.pressure_alt, sensors.tas_kph,
                                                    sensors.gload);
      enqueue(nmea_msg);
    }
    break;

    case 0x124:   // Wind momentan -> $PLARW,...,I,A
    {
      sensors.wind_bearing = toDeg360(bytesToFloat32(&can_msg.data[0]));
      sensors.wind_speed   = bytesToFloat32(&can_msg.data[4]) * 3.6;
      nmea_msg.sentence = NMEAMessage::createPLARW(sensors.wind_bearing, sensors.wind_speed,
                                                    false, true);
      enqueue(nmea_msg);
    }
    break;

    case 0x125:   // Wind Mittelwert -> $PLARW,...,A,A
    {
      double bearing = toDeg360(bytesToFloat32(&can_msg.data[0]));
      double speed   = bytesToFloat32(&can_msg.data[4]) * 3.6;
      nmea_msg.sentence = NMEAMessage::createPLARW(bearing, speed, true, true);
      enqueue(nmea_msg);
    }
    break;

    case 0x126:   // Druck / Dichte -> $PLARD (1 Hz)
    {
      float pressure_pa  = bytesToFloat32(&can_msg.data[0]);
      float density_kgm3 = bytesToFloat32(&can_msg.data[4]);

      // Druckhöhe nach ISA (1013.25 hPa)
      sensors.pressure_alt = 44330.77 * (1.0 - std::pow(pressure_pa / 101325.0, 0.1902632));
      sensors.air_density  = density_kgm3 * 1000.0;   // kg/m³ -> g/m³

      if (due(last_plard, 1000)) {
        nmea_msg.sentence = NMEAMessage::createPLARD(sensors.air_density, true);
        enqueue(nmea_msg);
      }
    }
    break;

    case 0x127:   // G-Kraft
    {
      // data[0] = g_force, data[4] = vertical_g_force (ggf. anpassen)
      sensors.gload = bytesToFloat32(&can_msg.data[0]) / 9.81;
    }
    break;

    case 0x129:   // Versorgungsspannung / Kreisflug
    {
      sensors.voltage = bytesToFloat32(&can_msg.data[0]);
      const int circling = (can_msg.data[4] == 2) ? 1 : 0;   // 2 = CIRCLING

      if (due(last_plarb, 1000)) {
        // Sensorbox liefert keine Temperatur/Feuchte -> NAN = Felder weglassen
        nmea_msg.sentence = NMEAMessage::createPLARB(sensors.voltage, NAN, NAN);
        enqueue(nmea_msg);
      }

      if (circling != last_circling) {   // $PLARS nur bei Änderung senden
        last_circling = circling;
        nmea_msg.sentence = NMEAMessage::createPLARS_CIR(circling);
        enqueue(nmea_msg);
      }
    }
    break;

    // In der Spezifikation definiert, aber für NMEA nicht benötigt
    case 0x128:   // Apparent roll/pitch
    case 0x12a:   // System state / git tag
    case 0x12b:   // Sensor health
    case 0x12f:   // Config value
      break;

    default:
      // Andere IDs (z.B. GPS 0x140-0x149, Heartbeat) werden ignoriert
      break;
  }
}