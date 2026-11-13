# Quick Start - Schnelleinstieg

## 30 Sekunden Setup

### Auf Linux (mit CAN-Hardware)

```bash
git clone <repo>
cd Larus
make
sudo ./larus-bridge can0
```

### Auf Linux (Simulator - keine Hardware nötig)

```bash
make
make test
# Simulator läuft automatisch, Ctrl+C zum Beenden
```

### Auf Windows (Simulation)

```bash
mkdir build
cd build
cmake ..
cmake --build .
cd ..
.\build\Release\larus-bridge.exe
```

---

## Echtdaten von CAN senden

### Mit `cansend` (Linux SocketCAN)

```bash
# GPS-Daten senden (0x100): Breitengrad 45.5°, Längengrad 10.5°
cansend can0 100#0201D57C0201D57C

# Geschwindigkeit 5.0 Knoten, Kurs 90°
cansend can0 101#01F401F4

# Kompass 95.3°
cansend can0 102#025B

# Tiefe 15.00m
cansend can0 103#0E74
```

### Mit Python (cross-platform)

```python
import socket
import struct
import time

# SocketCAN auf Linux
if __name__ == "__main__":
    sock = socket.socket(socket.AF_CAN, socket.SOCK_RAW, socket.CAN_RAW)
    sock.bind(('can0',))
    
    # GPS: Latitude 45.5, Longitude 10.5
    lat = int(45.5 * 1_000_000)
    lon = int(10.5 * 1_000_000)
    
    frame = struct.pack('>I2B4B', 0x100, 8, 0,
                        *lat.to_bytes(4, 'big'),
                        *lon.to_bytes(4, 'big'))
    
    sock.send(frame)
    sock.close()
```

---

## Ausgabe prüfen

Das Programm gibt NMEA-Sätze aus:

```
$GPGGA,143215,4530.0000,N,1030.0000,E,1,12,0.9,0.0,M,0.0,M,,*00
$GPRMC,143216,A,4530.0000,N,1030.0000,E,5.23,90.00,120260,,*00
$GPHDG,95.3,,,*00
$SDDBT,49.2,f,15.00,M,49.2,F*00
```

Diese können direkt in maritime Software importiert werden:
- SeaNav
- OpenCPN
- Navily
- Jotun
- Vesper

---

## Dateien überblick

| Datei | Zweck |
|-------|-------|
| `Larus-bridge.cpp` | Hauptprogramm |
| `simulator.cpp` | Testdaten-Generator |
| `CMakeLists.txt` | CMake Build-Konfiguration |
| `Makefile` | Traditionelles Make Build |
| `nmea_config.json` | Konfiguration der CAN-zu-NMEA Mapping |
| `README.md` | Vollständige Dokumentation |

---

## Problemlösung

### ❌ "Permission denied"
```bash
sudo ./larus-bridge can0
```

### ❌ "can0: No such device"
```bash
# CAN-Interface prüfen
ip link show

# Virtuelles Interface erstellen (für Simulator)
sudo ip link add dev vcan0 type vcan
sudo ip link set up vcan0
```

### ❌ "Bind failed"
```bash
# Benutzer zur CAN-Gruppe hinzufügen
sudo usermod -a -G can $USER
# Neu anmelden erforderlich!
```

---

## Nächste Schritte

1. **Weitere CAN-IDs hinzufügen** → `Larus-bridge.cpp` Zeile 200+
2. **Konfiguration anpassen** → `nmea_config.json` editieren
3. **Serielle Ausgabe** → UART/USB-Gerät in der Hauptschleife verbinden
4. **Docker-Deployment** → Dockerfile erstellen für Embedded-Systeme

Happy sailing! 🚢
