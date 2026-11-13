#pragma once

#include <string>
#include <cstdint>
#include <cstdio>
#include <cmath>
#include <ctime>

class NMEAMessage {
public:
  std::string sentence;

  static uint8_t calculateChecksum(const std::string& msg);
  static std::string formatNMEA(const std::string& msg);

  static std::string createGRMC(double lat, double lon, double speed, double course);
  static std::string createGPGGA(double lat, double lon, int sats, double hdop, double altitude);
  static std::string createPLARA(double roll, double pitch, double yaw);
  static std::string createPLARV(double tek_vario, double avg_vario, double pressure_alt,
                                  double tas_kph, double gload = -1.0);
  static std::string createPLARW(double wind_bearing, double wind_speed, bool instantaneous, bool valid);
  static std::string createPLARB(double voltage, double temperature, double humidity = -1.0);
  static std::string createPLARD(double air_density, bool measured);
  static std::string createPLARS_MC(double maccready);
  static std::string createPLARS_BAL(double ballast_fraction);
  static std::string createPLARS_BUGS(int bugs_percent);
  static std::string createPLARS_QNH(double qnh_hpa);
  static std::string createPLARS_CIR(int circling);
};
