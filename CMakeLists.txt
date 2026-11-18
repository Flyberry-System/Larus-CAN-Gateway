cmake_minimum_required(VERSION 3.10)
project(Larus-CAN-Gateway)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O2")

# Abhängigkeiten
find_package(Threads REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(SOCKETCAN REQUIRED libsocketcan)

# Include directories
include_directories(include)

# Source files for main executable
set(GATEWAY_SOURCES
  src/main.cpp
  src/NMEAMessage.cpp
  src/CANReader.cpp
  src/TCPServer.cpp
  src/SerialPort.cpp
  src/CANtoLarusConverter.cpp
)

# Main executable (production)
add_executable(larus-gateway ${GATEWAY_SOURCES})

target_include_directories(larus-gateway PRIVATE ${SOCKETCAN_INCLUDE_DIRS})
target_link_libraries(larus-gateway PRIVATE Threads::Threads ${SOCKETCAN_LIBRARIES})

# Optional: build CAN simulator (development)
add_executable(can-simulator simulator.cpp)

# Installation
install(TARGETS larus-gateway DESTINATION bin)
install(FILES nmea_config.json DESTINATION etc/larus)
install(FILES larus-gateway.service DESTINATION etc/systemd/system)
install(FILES README.md DESTINATION share/doc/larus)