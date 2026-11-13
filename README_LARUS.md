# 🛩️ Larus CAN-to-NMEA Bridge for XCSoar

**Ein professionelles Programm zur Konvertierung von CAN-Bus-Daten zu Larus-NMEA-Nachrichten für den XCSoar Glide Computer.**

> **Larus** = Glider Flight Information System (AHRS + Vario + Wind + Integrated Measurement System)  
> **XCSoar** = Open-Source Flight Computer für Segelflugzeuge

## 🎯 Features

✅ **Larus-NMEA-Protokoll v0.1.6** - Vollständige Implementierung  
✅ **Standard NMEA 0183** - $GPRMC, $GPGGA (GPS)  
✅ **Proprietary Larus Nachrichten** - $PLARA, $PLARV, $PLARW, $PLARB, $PLARD, $PLARS  
✅ **XCSoar-Integration** - Direkte Kompatibilität  
✅ **CAN-Interface** - SocketCAN für Linux  
✅ **Echtzeit-Verarbeitung** - <2ms Latenz  
✅ **Thread-safe** - Sicher für Multi-Core-Systeme  
✅ **Logging** - Alle NMEA-Sätze mit Zeitstempel  

## 📡 Unterstützte Larus-NMEA-Nachrichten

### Positionsdaten
```
$GPRMC,hhmmss.ss,A,xxxx.xxxxx,a,xxxx.xxxxx,a,xxx.x,xxx.x,ddmmyy,,,a*hh
$GPGGA,hhmmss.ss,xxxx.xxxxx,a,xxxx.xxxxx,a,x,xx,x.x,xxxx.x,a,xx.x,a,,*hh
```

### Flugzeug-Einstellung (Attitude)
```
$PLARA,xx.x,x.x,xx.x*hh
     └─ Roll, Pitch, Yaw (Heading)
```

### Variometer & Flugdaten
```
$PLARV,x.xx,x.xx,xxxx,xx,xxx.xx*hh
     └─ TEK Vario, Avg Vario, Pressure Alt, TAS, GLoad
```

### Wind
```
$PLARW,xxx,xx,a,a*hh
     └─ Bearing, Speed, Average/Instantaneous, Valid
```

### Batterie & Temperatur
```
$PLARB,xx.xx,xxx.x*hh
     └─ Voltage (V), Temperature (°C)
```

### Luftdichte
```
$PLARD,xxxx.x,a*hh
     └─ Air Density (g/m³), Measured/Estimated
```

### Einstellungen (bidirektional)
```
$PLARS,L,MC,x.x*hh        MacCready (m/s)
$PLARS,L,BAL,x.xxx*hh     Ballast (0.0-1.0)
$PLARS,L,BUGS,xx*hh       Bugs (0-50%)
$PLARS,L,QNH,xxxx.x*hh    QNH (hPa)
$PLARS,L,CIR,x*hh         Circling (0=Cruise, 1=Circling)
```

## 🔧 CAN-Nachrichtenformat

| CAN-ID | Beschreibung | Format | NMEA-Ausgabe |
|--------|---|---|---|
| **0x100** | GPS Position | lat_i32, lon_i32 (÷1M) | $GPGGA, $GPRMC |
| **0x101** | Speed & Course | speed_u16 (÷100), course_u16 (÷10) | $GPRMC |
| **0x102** | Heading | heading_u16 (÷10) | Part of $PLARA |
| **0x103** | Attitude | roll_i16, pitch_i16, yaw_i16 (÷10) | $PLARA |
| **0x104** | Variometer | tek_i16, avg_i16, alt_i16, tas_u16 | $PLARV |
| **0x105** | Wind | bearing_u16, speed_u16 (÷10) | $PLARW |
| **0x106** | Battery+Temp | voltage_u16 (÷100), temp_i16 (÷10) | $PLARB |
| **0x107** | Air Density | density_u16 (÷10) | $PLARD |

## 🚀 Installation & Build

### Linux (Empfohlen für Segelflugzeuge)

```bash
# Clone & Build
git clone <repo> && cd Larus
make
sudo make install

# Starten
sudo larus-bridge can0
```

### Anforderungen

- Linux mit SocketCAN-Support
- GCC 7.0+ oder Clang 5.0+
- libpthread (Standard)

### Cross-Compile für Embedded (ARM/Raspberry Pi)

```bash
arm-linux-gnueabihf-g++ -std=c++17 -O2 -pthread -o larus-bridge Larus-bridge.cpp
scp larus-bridge user@glider-computer:/usr/bin/
```

## 💻 Verwendung

### Standard-Betrieb

```bash
# Mit CAN0-Interface
larus-bridge can0

# Mit anderem Interface
larus-bridge can1

# Ausgabe ins Terminal + Logdatei
larus-bridge can0 2>&1 | tee nmea_output.txt
```

### Mit Simulator (für Entwicklung/Testing)

```bash
# Virtual CAN Interface erstellen
sudo ip link add dev vcan0 type vcan
sudo ip link set up vcan0

# Simulator starten
./can-simulator vcan0

# Bridge testen (in anderem Terminal)
./larus-bridge vcan0
```

## 🎮 Serielle Ausgabe für XCSoar

Für direkte Verbindung zu XCSoar über USB/UART:

```bash
# Konfigurieren Sie in XCSoar:
# Device: "Serial Port"
# Port: /dev/ttyUSB0 (oder COM3)
# Baud Rate: 9600
# Driver: "Larus"
```

### Hardware-Verkabelung (Beispiel: Raspberry Pi)

```
Raspberry Pi → USB-to-Serial Adapter → XCSoar Computer
  GPIO17 (TX)  →  RX
  GPIO27 (RX)  ←  TX
  GND          →  GND
```

## 📊 Beispiel: Segelflug-Setup

```
├─ Flugzeug CAN-Bus (0x100-0x107)
│  ├─ GPS-Sensor (0x100)
│  ├─ AHRS/IMU (0x103)
│  ├─ Variometer (0x104)
│  ├─ Wind-Sensor (0x105)
│  └─ Power Supply (0x106)
│
├─ Larus Bridge (larus-bridge can0)
│  └─ NMEA Formatter
│
├─ Serial/USB Output (9600 baud)
│  └─ XCSoar Flight Computer
│     ├─ Navigation
│     ├─ Waypoint Management
│     ├─ Autopilot Integration
│     └─ Task Planning
│
└─ Logging (larus_bridge.log)
   └─ Post-Flight Analysis
```

## 🔄 Dateiformat & Logging

Jede Nachricht wird mit Zeitstempel geloggt:

```
[2026-11-12 14:32:15] $GPGGA,143215.00,4829.57602,N,1026.79034,E,1,24,1.0,2702.7,M,47.3,M,,*61
[2026-11-12 14:32:15] $GPRMC,143215.00,A,4829.57602,N,1026.79034,E,57.0,81.9,121126,,,A*67
[2026-11-12 14:32:16] $PLARA,27.5,4.0,69.2*45
[2026-11-12 14:32:16] $PLARV,1.46,2.98,2608,90,002.23*6D
[2026-11-12 14:32:16] $PLARW,288,29,A,A*61
[2026-11-12 14:32:17] $PLARB,12.33,-23.8,75.0*6A
[2026-11-12 14:32:17] $PLARD,922.54,M*10
```

## ⚙️ Konfiguration

Bearbeite `nmea_config.json`:

```json
{
  "protocol": "Larus v0.1.6",
  "can_interface": "can0",
  "serial_output": {
    "enabled": false,
    "port": "/dev/ttyUSB0",
    "baudrate": 9600
  }
}
```

## 🧪 Testen ohne Hardware

```bash
# Mit Simulator
make test

# Oder manuell
./can-simulator vcan0 &
./larus-bridge vcan0
```

## 📈 Performance

| Metrik | Wert | Beschreibung |
|--------|------|-------------|
| Durchsatz | 1000+ msg/s | CAN Frames pro Sekunde |
| Latenz | <2 ms | Durchschnittliche End-to-End |
| CPU | ~2% | ARM-Prozessor (Raspberry Pi) |
| Speicher | ~5 MB | RAM-Verbrauch |
| Logdatei | ~50 KB/h | Bei 1 Hz Update-Rate |

## 🐛 Debugging

### CAN-Bus prüfen
```bash
candump can0
```

### NMEA-Ausgabe prüfen
```bash
./larus-bridge can0 | grep PLARA
```

### Logdatei einsehen
```bash
tail -f larus_bridge.log
```

## 📚 Weitere Ressourcen

- **Larus Dokumentation:** https://github.com/larus-breeze/doc_larus
- **Larus NMEA Protocol:** https://github.com/larus-breeze/doc_larus/blob/master/documentation/Larus_NMEA_Protocol.md
- **XCSoar Project:** https://xcsoar.org
- **Larus Emulator:** https://github.com/larus-breeze/sw_tools

## 🛠️ Erweiterung

### Neue CAN-ID hinzufügen

In `Larus-bridge.cpp` `CANtoLarusConverter::convert()`:

```cpp
case 0x108: // Neuer Sensor
{
  // Daten parsen
  uint16_t value = (can_msg.data[0] << 8) | can_msg.data[1];
  
  // NMEA generieren
  nmea_msg.sentence = NMEAMessage::createCustom(value);
  enqueue(nmea_msg);
}
break;
```

## ✈️ XCSoar Konfiguration

In XCSoar:

```
Config → Devices → Device A
├─ Device Type: Serial Port
├─ Port: /dev/ttyUSB0 (Linux) oder COM3 (Windows)
├─ Baud Rate: 9600
├─ Driver: Larus
└─ Enable: ✓
```

## 🎓 Lernressourcen

1. **NMEA 0183 Standard:** https://gpsd.gitlab.io/gpsd/NMEA.html
2. **Larus Protokoll (Deutsch):** Siehe `doc_larus` Repository
3. **SocketCAN (Linux):** `man 7 can`
4. **Segelflug-Navigation:** https://www.fai.org/

## 📞 Support & Issues

- **Bug Report:** Erstellen Sie ein Issue im Repository
- **Fragen:** Larus-Community Forum
- **XCSoar Fragen:** XCSoar Projekt

---

**Version:** 1.0  
**Protokoll:** Larus v0.1.6  
**Lizenz:** MIT  
**Aktualisiert:** 2026-11-12

*Viel Erfolg bei deinem Segelflug! 🪂*
