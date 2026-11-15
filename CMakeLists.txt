cmake_minimum_required(VERSION 3.10)
project(Larus-CAN-Gateway)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O2 -pthread")

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

# Platform-specific settings
if(UNIX AND NOT APPLE)
  # Linux - link with real CAN support
  target_link_libraries(larus-gateway PRIVATE pthread)
  message(STATUS "Building for Linux with SocketCAN support")
elseif(WIN32)
  # Windows - Winsock2 for network simulation
  target_link_libraries(larus-gateway PRIVATE ws2_32)
  message(STATUS "Building for Windows (simulation mode)")
endif()

# Optional: build CAN simulator (development)
add_executable(can-simulator simulator.cpp)

# Installation
install(TARGETS larus-gateway DESTINATION bin)
install(FILES nmea_config.json DESTINATION etc/larus)
install(FILES larus-gateway.service DESTINATION etc/systemd/system)
install(FILES README.md DESTINATION share/doc/larus)

