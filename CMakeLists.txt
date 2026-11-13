cmake_minimum_required(VERSION 3.10)
project(Larus-CAN-Gateway)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Main executable
add_executable(larus-bridge Larus-bridge.cpp)

# Platform-specific settings
if(UNIX AND NOT APPLE)
  # Linux - link with real CAN support
  target_link_libraries(larus-bridge PRIVATE pthread)
  message(STATUS "Building for Linux with SocketCAN support")
elseif(WIN32)
  # Windows - Winsock2 for network simulation
  target_link_libraries(larus-bridge PRIVATE ws2_32)
  message(STATUS "Building for Windows (simulation mode)")
endif()

# Optional: build CAN simulator
add_executable(can-simulator simulator.cpp)

# Installation
install(TARGETS larus-bridge DESTINATION bin)
install(FILES nmea_config.json DESTINATION etc/larus)
install(FILES README.md DESTINATION share/doc/larus)
