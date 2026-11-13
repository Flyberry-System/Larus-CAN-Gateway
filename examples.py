#!/usr/bin/env python3
"""
Larus CAN Bridge - Python Test Suite & Examples
Demonstrates how to send CAN data and verify NMEA output
"""

import socket
import struct
import time
import subprocess
import threading
import sys

# ===== CAN Message Builders =====

class CANMessage:
    """Build CAN frames for Larus bridge"""

    @staticmethod
    def gps_position(latitude, longitude):
        """Create 0x100 GPS Position message"""
        lat_raw = int(latitude * 1_000_000)
        lon_raw = int(longitude * 1_000_000)

        data = struct.pack('>ii', lat_raw, lon_raw)
        return (0x100, 8, data)

    @staticmethod
    def speed_course(speed_kn, course_deg):
        """Create 0x101 Speed & Course message"""
        speed_raw = int(speed_kn * 100)
        course_raw = int(course_deg * 10)

        data = struct.pack('>HH', speed_raw, course_raw)
        return (0x101, 4, data)

    @staticmethod
    def heading(heading_deg):
        """Create 0x102 Heading message"""
        heading_raw = int(heading_deg * 10)

        data = struct.pack('>H', heading_raw) + b'\x00\x00\x00\x00\x00\x00'
        return (0x102, 2, data)

    @staticmethod
    def depth(depth_m):
        """Create 0x103 Water Depth message"""
        depth_raw = int(depth_m * 100)

        data = struct.pack('>H', depth_raw) + b'\x00\x00\x00\x00\x00\x00'
        return (0x103, 2, data)


class SocketCANSender:
    """Send CAN messages via SocketCAN (Linux)"""

    def __init__(self, interface='can0'):
        self.interface = interface
        self.socket = None

    def connect(self):
        """Connect to CAN interface"""
        try:
            self.socket = socket.socket(socket.AF_CAN, socket.SOCK_RAW, socket.CAN_RAW)
            self.socket.bind((self.interface,))
            print(f"✓ Connected to {self.interface}")
            return True
        except Exception as e:
            print(f"✗ Failed to connect to {self.interface}: {e}")
            return False

    def send_message(self, can_id, dlc, data):
        """Send a CAN message"""
        if not self.socket:
            print("✗ Not connected")
            return False

        try:
            # Pack CAN frame: ID(4) + flags(1) + DLC(1) + pad(2) + data(8)
            frame = struct.pack('=IB3s8s', can_id, dlc, b'\x00\x00\x00', data)
            self.socket.send(frame)
            return True
        except Exception as e:
            print(f"✗ Failed to send: {e}")
            return False

    def close(self):
        """Close connection"""
        if self.socket:
            self.socket.close()


# ===== Test Scenarios =====

def test_gps_tracking(sender, num_points=10):
    """Test GPS position tracking"""
    print("\n=== Test 1: GPS Position Tracking ===")

    # Simulate boat movement from Venice to Trieste
    start_lat, start_lon = 45.4408, 12.3155

    for i in range(num_points):
        lat = start_lat + (i * 0.0001)  # Move north
        lon = start_lon + (i * 0.0001)  # Move east

        can_id, dlc, data = CANMessage.gps_position(lat, lon)
        sender.send_message(can_id, dlc, data)

        print(f"  Sent GPS: {lat:.4f}°N {lon:.4f}°E")
        time.sleep(1)


def test_speed_variation(sender, duration=30):
    """Test speed and course changes"""
    print("\n=== Test 2: Speed & Course Variation ===")

    start_time = time.time()
    course = 0

    while time.time() - start_time < duration:
        # Simulate speed variation
        speed = 5 + 2 * abs(time.time() % 2 - 1)  # Oscillates 5-7 knots
        course = (time.time() * 10) % 360  # Rotating course

        can_id, dlc, data = CANMessage.speed_course(speed, course)
        sender.send_message(can_id, dlc, data)

        print(f"  Speed: {speed:.1f} kn, Course: {course:.1f}°")
        time.sleep(2)


def test_compass_rotation(sender, duration=20):
    """Test compass/heading data"""
    print("\n=== Test 3: Compass Rotation ===")

    start_time = time.time()

    while time.time() - start_time < duration:
        heading = (time.time() * 18) % 360  # 360° in 20 seconds

        can_id, dlc, data = CANMessage.heading(heading)
        sender.send_message(can_id, dlc, data)

        print(f"  Heading: {heading:.1f}°")
        time.sleep(1)


def test_depth_profiling(sender, num_points=20):
    """Test water depth profiling"""
    print("\n=== Test 4: Water Depth Profiling ===")

    for i in range(num_points):
        # Simulate depth variation (e.g., sailing over shallow area)
        depth = 20 - (10 * abs(i - 10) / 10)  # V-shaped profile

        can_id, dlc, data = CANMessage.depth(depth)
        sender.send_message(can_id, dlc, data)

        print(f"  Depth: {depth:.2f} m")
        time.sleep(1)


def test_all_messages(sender):
    """Send all message types at once"""
    print("\n=== Test 5: All Message Types ===")

    # GPS
    can_id, dlc, data = CANMessage.gps_position(45.5, 10.5)
    sender.send_message(can_id, dlc, data)
    print("  Sent GPS")

    time.sleep(0.5)

    # Speed & Course
    can_id, dlc, data = CANMessage.speed_course(5.5, 90.0)
    sender.send_message(can_id, dlc, data)
    print("  Sent Speed/Course")

    time.sleep(0.5)

    # Heading
    can_id, dlc, data = CANMessage.heading(95.3)
    sender.send_message(can_id, dlc, data)
    print("  Sent Heading")

    time.sleep(0.5)

    # Depth
    can_id, dlc, data = CANMessage.depth(15.0)
    sender.send_message(can_id, dlc, data)
    print("  Sent Depth")


# ===== Main =====

def main():
    if len(sys.argv) < 2:
        print("""
Larus CAN Bridge - Test Suite

Usage:
  python3 examples.py <interface> [test_name]

Examples:
  python3 examples.py can0              # Run all tests
  python3 examples.py vcan0 gps         # Test GPS only
  python3 examples.py can0 speed        # Test speed variation
  python3 examples.py can0 depth        # Test depth profiling
  python3 examples.py can0 all          # Send all message types

Interfaces:
  can0, can1, ... (real CAN)
  vcan0, vcan1, ... (virtual CAN - Linux)

Note: Start the bridge before running tests:
  ./larus-bridge <interface>
        """)
        return

    interface = sys.argv[1]
    test_name = sys.argv[2] if len(sys.argv) > 2 else "all"

    sender = SocketCANSender(interface)

    if not sender.connect():
        return

    try:
        if test_name == "gps":
            test_gps_tracking(sender)
        elif test_name == "speed":
            test_speed_variation(sender)
        elif test_name == "compass":
            test_compass_rotation(sender)
        elif test_name == "depth":
            test_depth_profiling(sender)
        elif test_name == "all":
            test_all_messages(sender)
        else:
            print(f"Unknown test: {test_name}")

    except KeyboardInterrupt:
        print("\n✓ Stopped by user")
    finally:
        sender.close()


if __name__ == "__main__":
    main()
