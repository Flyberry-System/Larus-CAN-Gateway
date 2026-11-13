#include "NMEAMessage.h"
#include <cstring>

uint8_t NMEAMessage::calculateChecksum(const std::string& msg) {
  uint8_t checksum = 0;
  for (char c : msg) {
    checksum ^= c;
  }
  return checksum;
}

std::string NMEAMessage::formatNMEA(const std::string& msg) {
  uint8_t checksum = calculateChecksum(msg);
  char buffer[256];
  snprintf(buffer, sizeof(buffer), "$%s*%02X\r\n", msg.c_str(), checksum);
  return std::string(buffer);
}

std::string NMEAMessage::createGRMC(double lat, double lon, double speed, double course) {
  char buffer[150];
  time_t now = time(nullptr);
  struct tm *tm_info = gmtime(&now);

  int lat_deg = static_cast<int>(lat);
  double lat_min = (lat - lat_deg) * 60.0;
  int lon_deg = static_cast<int>(lon);
  double lon_min = (lon - lon_deg) * 60.0;

  snprintf(buffer, sizeof(buffer),
           "GPRMC,%02d%02d%02d.00,A,%02d%07.5f,%c,%03d%07.5f,%c,%.1f,%.1f,%02d%02d%02d,,,A",
           tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec,
           lat_deg, lat_min, lat >= 0 ? 'N' : 'S',
           lon_deg, lon_min, lon >= 0 ? 'E' : 'W',
           speed, course,
           tm_info->tm_mday, tm_info->tm_mon + 1, tm_info->tm_year % 100);

  return formatNMEA(buffer);
}

std::string NMEAMessage::createGPGGA(double lat, double lon, int sats, double hdop, double altitude) {
  char buffer[150];
  time_t now = time(nullptr);
  struct tm *tm_info = gmtime(&now);

  int lat_deg = static_cast<int>(lat);
  double lat_min = (lat - lat_deg) * 60.0;
  int lon_deg = static_cast<int>(lon);
  double lon_min = (lon - lon_deg) * 60.0;

  snprintf(buffer, sizeof(buffer),
           "GPGGA,%02d%02d%02d.00,%02d%07.5f,%c,%03d%07.5f,%c,1,%02d,%.1f,%.1f,M,0.0,M,,",
           tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec,
           lat_deg, lat_min, lat >= 0 ? 'N' : 'S',
           lon_deg, lon_min, lon >= 0 ? 'E' : 'W',
           sats, hdop, altitude);

  return formatNMEA(buffer);
}

std::string NMEAMessage::createPLARA(double roll, double pitch, double yaw) {
  char buffer[100];
  snprintf(buffer, sizeof(buffer), "PLARA,%.1f,%.1f,%.1f", roll, pitch, yaw);
  return formatNMEA(buffer);
}

std::string NMEAMessage::createPLARV(double tek_vario, double avg_vario, double pressure_alt,
                                      double tas_kph, double gload) {
  char buffer[120];
  if (gload >= 0) {
    snprintf(buffer, sizeof(buffer), "PLARV,%.2f,%.2f,%.0f,%.0f,%.2f",
             tek_vario, avg_vario, pressure_alt, tas_kph, gload);
  } else {
    snprintf(buffer, sizeof(buffer), "PLARV,%.2f,%.2f,%.0f,%.0f",
             tek_vario, avg_vario, pressure_alt, tas_kph);
  }
  return formatNMEA(buffer);
}

std::string NMEAMessage::createPLARW(double wind_bearing, double wind_speed, bool instantaneous, bool valid) {
  char buffer[100];
  snprintf(buffer, sizeof(buffer), "PLARW,%03.0f,%02.0f,%c,%c",
           wind_bearing, wind_speed,
           instantaneous ? 'I' : 'A',
           valid ? 'A' : 'V');
  return formatNMEA(buffer);
}

std::string NMEAMessage::createPLARB(double voltage, double temperature, double humidity) {
  char buffer[120];
  if (humidity >= 0) {
    snprintf(buffer, sizeof(buffer), "PLARB,%.2f,%.1f,%.1f", voltage, temperature, humidity);
  } else {
    snprintf(buffer, sizeof(buffer), "PLARB,%.2f,%.1f", voltage, temperature);
  }
  return formatNMEA(buffer);
}

std::string NMEAMessage::createPLARD(double air_density, bool measured) {
  char buffer[100];
  snprintf(buffer, sizeof(buffer), "PLARD,%.1f,%c", air_density, measured ? 'M' : 'E');
  return formatNMEA(buffer);
}

std::string NMEAMessage::createPLARS_MC(double maccready) {
  char buffer[80];
  snprintf(buffer, sizeof(buffer), "PLARS,L,MC,%.1f", maccready);
  return formatNMEA(buffer);
}

std::string NMEAMessage::createPLARS_BAL(double ballast_fraction) {
  char buffer[80];
  snprintf(buffer, sizeof(buffer), "PLARS,L,BAL,%.3f", ballast_fraction);
  return formatNMEA(buffer);
}

std::string NMEAMessage::createPLARS_BUGS(int bugs_percent) {
  char buffer[80];
  snprintf(buffer, sizeof(buffer), "PLARS,L,BUGS,%d", bugs_percent);
  return formatNMEA(buffer);
}

std::string NMEAMessage::createPLARS_QNH(double qnh_hpa) {
  char buffer[80];
  snprintf(buffer, sizeof(buffer), "PLARS,L,QNH,%.1f", qnh_hpa);
  return formatNMEA(buffer);
}

std::string NMEAMessage::createPLARS_CIR(int circling) {
  char buffer[80];
  snprintf(buffer, sizeof(buffer), "PLARS,L,CIR,%d", circling);
  return formatNMEA(buffer);
}
