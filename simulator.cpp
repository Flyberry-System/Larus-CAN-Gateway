#include <iostream>
#include <cstring>
#include <cstdlib>
#include <thread>
#include <chrono>
#include <random>
#include <cmath>

#ifdef __linux__
  #include <linux/can.h>
  #include <linux/can/raw.h>
  #include <net/if.h>
  #include <sys/socket.h>
  #include <sys/ioctl.h>
  #include <unistd.h>
#endif

class LarusCANSimulator {
private:
  int socket_fd;
  std::string interface;
  std::mt19937 gen;

  // Sensor simulation state
  double latitude = 48.5;         // Germany (Öschelbronn)
  double longitude = 10.26;
  double altitude = 1200.0;       // meters MSL
  double pressure_alt = 1250.0;   // meters pressure altitude
  double heading = 90.0;          // degrees (East)
  double roll = 0.0;              // degrees
  double pitch = 0.0;             // degrees
  double speed = 50.0;            // knots
  double tas = 60.0;              // km/h
  double tek_vario = 0.5;         // m/s (climb rate)
  double avg_vario = 1.2;         // m/s (avg over circle)
  double gload = 1.0;             // G's
  double wind_bearing = 270.0;    // from West
  double wind_speed = 15.0;       // km/h
  double voltage = 12.5;          // Volts
  double temperature = -15.0;     // Celsius (altitude)
  double air_density = 1.05;      // kg/m³

public:
  LarusCANSimulator(const std::string& iface = "vcan0")
    : interface(iface), socket_fd(-1) {
    gen.seed(std::random_device{}());
  }

  ~LarusCANSimulator() {
    if (socket_fd >= 0) {
#ifdef __linux__
      close(socket_fd);
#endif
    }
  }

  bool init() {
#ifdef __linux__
    socket_fd = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (socket_fd < 0) {
      std::cerr << "✗ Socket creation failed" << std::endl;
      return false;
    }

    struct ifreq ifr;
    strcpy(ifr.ifr_name, interface.c_str());
    if (ioctl(socket_fd, SIOCGIFINDEX, &ifr) < 0) {
      std::cerr << "✗ Interface " << interface << " not found" << std::endl;
      std::cerr << "  Create virtual CAN: sudo ip link add dev vcan0 type vcan" << std::endl;
      std::cerr << "                      sudo ip link set up vcan0" << std::endl;
      return false;
    }

    struct sockaddr_can addr;
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    if (bind(socket_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
      std::cerr << "✗ Bind failed" << std::endl;
      return false;
    }

    std::cout << "✓ Larus CAN Simulator initialized on: " << interface << std::endl;
    return true;
#else
    std::cout << "✓ Larus CAN Simulator (Windows simulation mode)" << std::endl;
    return true;
#endif
  }

  void sendGPSData() {
    // CAN 0x100: GPS Position (lat_i32, lon_i32)
    // Simulate flight path (circle)
    std::normal_distribution<> lat_drift(0, 0.0001);
    std::normal_distribution<> lon_drift(0, 0.0001);

    latitude += lat_drift(gen);
    longitude += lon_drift(gen);

#ifdef __linux__
    struct can_frame frame;
    frame.can_id = 0x100;
    frame.can_dlc = 8;

    int32_t lat_raw = static_cast<int32_t>(latitude * 1000000);
    int32_t lon_raw = static_cast<int32_t>(longitude * 1000000);

    frame.data[0] = (lat_raw >> 24) & 0xFF;
    frame.data[1] = (lat_raw >> 16) & 0xFF;
    frame.data[2] = (lat_raw >> 8) & 0xFF;
    frame.data[3] = lat_raw & 0xFF;
    frame.data[4] = (lon_raw >> 24) & 0xFF;
    frame.data[5] = (lon_raw >> 16) & 0xFF;
    frame.data[6] = (lon_raw >> 8) & 0xFF;
    frame.data[7] = lon_raw & 0xFF;

    if (write(socket_fd, &frame, sizeof(struct can_frame)) > 0) {
      std::cout << "  GPS: " << latitude << "°N " << longitude << "°E" << std::endl;
    }
#endif
  }

  void sendSpeedCourseData() {
    // CAN 0x101: Speed & Course (speed_u16, course_u16)
    std::normal_distribution<> speed_var(0, 2.0);
    speed = std::max(10.0, speed + speed_var(gen));

#ifdef __linux__
    struct can_frame frame;
    frame.can_id = 0x101;
    frame.can_dlc = 4;

    uint16_t speed_raw = static_cast<uint16_t>(speed * 100);
    uint16_t course_raw = static_cast<uint16_t>(heading * 10);

    frame.data[0] = (speed_raw >> 8) & 0xFF;
    frame.data[1] = speed_raw & 0xFF;
    frame.data[2] = (course_raw >> 8) & 0xFF;
    frame.data[3] = course_raw & 0xFF;

    if (write(socket_fd, &frame, sizeof(struct can_frame)) > 0) {
      std::cout << "  Speed: " << speed << " knots, Course: " << heading << "°" << std::endl;
    }
#endif
  }

  void sendAttitudeData() {
    // CAN 0x103: Attitude (roll_i16, pitch_i16, yaw_i16)
    // Simulate gentle banking and climb
    std::uniform_real_distribution<> roll_var(-5, 5);
    std::uniform_real_distribution<> pitch_var(-2, 3);

    roll = roll_var(gen);
    pitch = pitch_var(gen);
    heading = fmod(heading + 0.5, 360.0);  // Slowly turning

#ifdef __linux__
    struct can_frame frame;
    frame.can_id = 0x103;
    frame.can_dlc = 6;

    int16_t roll_raw = static_cast<int16_t>(roll * 10);
    int16_t pitch_raw = static_cast<int16_t>(pitch * 10);
    int16_t yaw_raw = static_cast<int16_t>(heading * 10);

    frame.data[0] = (roll_raw >> 8) & 0xFF;
    frame.data[1] = roll_raw & 0xFF;
    frame.data[2] = (pitch_raw >> 8) & 0xFF;
    frame.data[3] = pitch_raw & 0xFF;
    frame.data[4] = (yaw_raw >> 8) & 0xFF;
    frame.data[5] = yaw_raw & 0xFF;

    if (write(socket_fd, &frame, sizeof(struct can_frame)) > 0) {
      std::cout << "  Attitude: Roll=" << roll << "° Pitch=" << pitch << "° Hdg=" << heading << "°" << std::endl;
    }
#endif
  }

  void sendVariometerData() {
    // CAN 0x104: Variometer (tek_i16, avg_i16, alt_i16, tas_u16)
    // Simulate thermal climbing
    std::normal_distribution<> vario_var(0, 0.2);
    tek_vario = tek_vario + vario_var(gen);
    tek_vario = std::max(-2.0, std::min(4.0, tek_vario));  // Clamp ±2 to +4

    if (pressure_alt < 2000) {
      avg_vario = tek_vario * 0.8 + 0.3;  // Average lags behind
    } else {
      avg_vario = std::max(0.0, tek_vario - 0.5);
    }

    pressure_alt += tek_vario * 0.5;  // Altitude increases with climb

#ifdef __linux__
    struct can_frame frame;
    frame.can_id = 0x104;
    frame.can_dlc = 8;

    int16_t tek_raw = static_cast<int16_t>(tek_vario * 100);
    int16_t avg_raw = static_cast<int16_t>(avg_vario * 100);
    int16_t alt_raw = static_cast<int16_t>(pressure_alt);
    uint16_t tas_raw = static_cast<uint16_t>(tas);

    frame.data[0] = (tek_raw >> 8) & 0xFF;
    frame.data[1] = tek_raw & 0xFF;
    frame.data[2] = (avg_raw >> 8) & 0xFF;
    frame.data[3] = avg_raw & 0xFF;
    frame.data[4] = (alt_raw >> 8) & 0xFF;
    frame.data[5] = alt_raw & 0xFF;
    frame.data[6] = (tas_raw >> 8) & 0xFF;
    frame.data[7] = tas_raw & 0xFF;

    if (write(socket_fd, &frame, sizeof(struct can_frame)) > 0) {
      std::cout << "  Vario: TEK=" << tek_vario << " m/s, Avg=" << avg_vario << " m/s, Alt=" << pressure_alt << " m" << std::endl;
    }
#endif
  }

  void sendWindData() {
    // CAN 0x105: Wind (bearing_u16, speed_u16)
    std::normal_distribution<> wind_var(0, 1.0);
    wind_bearing = fmod(wind_bearing + wind_var(gen), 360.0);
    wind_speed = std::max(5.0, wind_speed + wind_var(gen));

#ifdef __linux__
    struct can_frame frame;
    frame.can_id = 0x105;
    frame.can_dlc = 4;

    uint16_t bearing_raw = static_cast<uint16_t>(wind_bearing * 10);
    uint16_t speed_raw = static_cast<uint16_t>(wind_speed * 10);

    frame.data[0] = (bearing_raw >> 8) & 0xFF;
    frame.data[1] = bearing_raw & 0xFF;
    frame.data[2] = (speed_raw >> 8) & 0xFF;
    frame.data[3] = speed_raw & 0xFF;

    if (write(socket_fd, &frame, sizeof(struct can_frame)) > 0) {
      std::cout << "  Wind: " << wind_bearing << "° from " << wind_speed << " km/h" << std::endl;
    }
#endif
  }

  void sendBatteryData() {
    // CAN 0x106: Battery & Temperature (voltage_u16, temp_i16)
    std::normal_distribution<> volt_noise(0, 0.01);
    voltage = std::max(10.0, std::min(14.0, voltage + volt_noise(gen)));

    std::normal_distribution<> temp_noise(0, 0.1);
    temperature = temperature + temp_noise(gen);

#ifdef __linux__
    struct can_frame frame;
    frame.can_id = 0x106;
    frame.can_dlc = 4;

    uint16_t voltage_raw = static_cast<uint16_t>(voltage * 100);
    int16_t temp_raw = static_cast<int16_t>(temperature * 10);

    frame.data[0] = (voltage_raw >> 8) & 0xFF;
    frame.data[1] = voltage_raw & 0xFF;
    frame.data[2] = (temp_raw >> 8) & 0xFF;
    frame.data[3] = temp_raw & 0xFF;

    if (write(socket_fd, &frame, sizeof(struct can_frame)) > 0) {
      std::cout << "  Battery: " << voltage << "V, Temp: " << temperature << "°C" << std::endl;
    }
#endif
  }

  void sendAirDensityData() {
    // CAN 0x107: Air Density (density_u16)
    // Air density decreases with altitude
    air_density = 1.225 * std::pow(1.0 - 0.0000225 * pressure_alt, 4.255);

#ifdef __linux__
    struct can_frame frame;
    frame.can_id = 0x107;
    frame.can_dlc = 2;

    uint16_t density_raw = static_cast<uint16_t>(air_density * 10);

    frame.data[0] = (density_raw >> 8) & 0xFF;
    frame.data[1] = density_raw & 0xFF;

    if (write(socket_fd, &frame, sizeof(struct can_frame)) > 0) {
      std::cout << "  Air Density: " << air_density << " kg/m³" << std::endl;
    }
#endif
  }

  void run() {
    int cycle = 0;

    std::cout << "\n=== Larus CAN Simulator - Flight Data ===" << std::endl;
    std::cout << "Simulating glider flight near Öschelbronn, Germany" << std::endl;
    std::cout << "Press Ctrl+C to stop\n" << std::endl;

    while (true) {
      // Cycle through messages at different rates
      if (cycle % 2 == 0) sendGPSData();
      if (cycle % 1 == 0) sendSpeedCourseData();
      if (cycle % 1 == 0) sendAttitudeData();
      if (cycle % 1 == 0) sendVariometerData();
      if (cycle % 5 == 0) sendWindData();
      if (cycle % 5 == 0) sendBatteryData();
      if (cycle % 10 == 0) sendAirDensityData();

      cycle++;

      // Update every 500ms for realistic flight data
      std::cout << std::endl;
      std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
  }
};

int main(int argc, char* argv[]) {
  std::string interface = "vcan0";

  if (argc > 1) {
    interface = argv[1];
  }

  std::cout << "\n╔════════════════════════════════════════╗" << std::endl;
  std::cout << "║    Larus CAN Simulator for XCSoar      ║" << std::endl;
  std::cout << "║  Generates realistic flight data       ║" << std::endl;
  std::cout << "╚════════════════════════════════════════╝\n" << std::endl;

  LarusCANSimulator sim(interface);
  if (!sim.init()) {
    return 1;
  }

  sim.run();
  return 0;
}
