# 🛩️ Larus CAN Bridge - Unterstützte Standards

## ⭐ NEUERUNG: Offizielle Larus Spezifikation jetzt unterstützt!

Die Bridge unterstützt jetzt **BEIDE** CAN-Formate:

### 1️⃣ **Vereinfachter Mode** (0x100-0x107)
✅ Schneller & einfacher Einstieg  
✅ Integer mit Scaling (u16, i16)  
✅ Für Prototyping & DIY  

### 2️⃣ **Offizieller Larus Spec** (0x120-0x12f) ⭐ **NEU**
✅ Professionelle IEEE 754 F32 (Floating Point)  
✅ Offizielle Larus Sensor Box Spezifikation  
✅ Für Production-Systeme  

---

## 📊 CAN-ID Mapping (Beide Modes)

### VEREINFACHT (0x100-0x107)
```
0x100  GPS Position         (lat_i32÷1M, lon_i32÷1M)
0x101  Speed & Course       (speed_u16÷100, course_u16÷10)
0x102  Heading              (heading_u16÷10)
0x103  Attitude             (roll_i16÷10, pitch_i16÷10, yaw_i16÷10)
0x104  Variometer           (tek_i16÷100, avg_i16÷100, alt_i16, tas_u16)
0x105  Wind                 (bearing_u16÷10, speed_u16÷10)
0x106  Battery+Temp         (voltage_u16÷100, temp_i16÷10)
0x107  Air Density          (density_u16÷10)
```

### OFFIZIELL (0x120-0x12f) ⭐
```
0x120  Roll & Pitch         (roll_f32 rad, pitch_f32 rad)
0x121  Yaw & Turn Rate      (yaw_f32 rad, turn_rate_f32 rad/s)
0x122  TAS & IAS            (tas_f32 m/s, ias_f32 m/s)
0x123  Vario & Avg          (vario_f32 m/s, avg_vario_f32 m/s)
0x124  Wind Dir & Speed     (wind_dir_f32 rad, wind_speed_f32 m/s)
0x125  Avg Wind             (avg_wind_dir_f32 rad, avg_wind_speed_f32 m/s)
0x126  Pressure & Density   (pressure_f32 Pa, density_f32 kg/m³)
0x127  G-Force              (g_force_f32 m/s², vertical_g_f32 m/s²)
0x128  Apparent Attitude    (roll_f32 rad, pitch_f32 rad)
0x129  Supply Voltage       (voltage_f32 V, circle_mode_u8)
0x12a  System State         (state_u32, git_tag_u32)
0x12b  Sensor Health        (mag_dist_f32, speed_acc_f32)
```

---

## 🔄 Automatische Konvertierung

Die Bridge erkennt automatisch, welcher Mode verwendet wird:

```
CAN-Message kommt an
  ├─ Wenn ID 0x100-0x107  → Vereinfachter Mode
  ├─ Wenn ID 0x120-0x12f  → Offizieller Spec Mode
  └─ Konvertierung zu NMEA → Ausgabe
```

**Keine Konfiguration nötig!** Beide Formate funktionieren parallel.

---

## 💾 Datentypen

### Vereinfacht
```
u16 = unsigned 16-bit integer
i16 = signed 16-bit integer
i32 = signed 32-bit integer
  → Skalierung in Software (÷10, ÷100, etc.)
```

### Offiziell
```
f32 = IEEE 754 Single Precision (32-bit float)
u8  = unsigned 8-bit
u32 = unsigned 32-bit
  → Native Floating-Point Werte
```

---

## 🎯 Welchen Mode verwenden?

| Szenario | Mode | Grund |
|----------|------|-------|
| DIY/Learning | Vereinfacht | Einfacher zu debuggen |
| Prototyping | Vereinfacht | Schnell implementiert |
| Production | Offiziell | Professional Standard |
| Larus Sensor Box | Offiziell | Native Format |
| Embedded Custom | Vereinfacht | Flexibler |

---

## 🔧 Code-Beispiele

### Vereinfacht senden (0x100 - GPS)
```c
// Latitude 48.5°N, Longitude 10.26°E
int32_t lat = 48500000;    // 48.5 * 1,000,000
int32_t lon = 10260000;    // 10.26 * 1,000,000

frame.can_id = 0x100;
frame.data[0] = (lat >> 24) & 0xFF;
frame.data[1] = (lat >> 16) & 0xFF;
frame.data[2] = (lat >>  8) & 0xFF;
frame.data[3] = (lat >>  0) & 0xFF;
frame.data[4] = (lon >> 24) & 0xFF;
frame.data[5] = (lon >> 16) & 0xFF;
frame.data[6] = (lon >>  8) & 0xFF;
frame.data[7] = (lon >>  0) & 0xFF;
```

### Offiziell senden (0x120 - Roll/Pitch in Radians)
```c
#include <string.h>

// Roll 27.5°, Pitch 4.0° → in Radians
float roll_rad = 27.5 * M_PI / 180.0;     // ≈ 0.480 rad
float pitch_rad = 4.0 * M_PI / 180.0;     // ≈ 0.070 rad

frame.can_id = 0x120;
memcpy(&frame.data[0], &roll_rad, 4);     // Bytes 0-3: Roll (F32)
memcpy(&frame.data[4], &pitch_rad, 4);    // Bytes 4-7: Pitch (F32)
```

---

## 📡 NMEA Ausgabe (gleich für beide!)

```
$GPGGA,...      GPS Position/Fix
$GPRMC,...      Recommended Minimum
$PLARA,...      Attitude (Roll/Pitch/Yaw)
$PLARV,...      Variometer Data
$PLARW,...      Wind Information
$PLARB,...      Battery + Temperature
$PLARD,...      Air Density
$PLARS,...      Settings/Circling Mode
```

---

## ⚙️ Interne Konvertierung

```
Vereinfacht Mode (Integer)
  ├─ Parse: Bytes → Integer
  ├─ Scale: ÷10, ÷100, etc.
  └─ NMEA: Format String

Offiziell Mode (Float)
  ├─ Parse: Bytes → IEEE 754 F32
  ├─ Convert: rad→deg, m/s→km/h, etc.
  └─ NMEA: Format String
```

---

## 🧪 Testen

### Simulator (Vereinfacht)
```bash
make test        # Generiert 0x100-0x107
```

### Echte Larus Sensorbox (Offiziell)
```bash
larus-bridge can0    # Liest 0x120-0x12f
```

### Beide gleichzeitig
```bash
larus-bridge can0    # Unterstützt beide paralell!
```

---

## 📚 Quellen

- **Vereinfacht:** Eigenes Mapping für schnelle Integration
- **Offiziell:** https://github.com/larus-breeze/doc_larus/blob/master/documentation/can_spec.md

---

## ✨ Zusammenfassung

| Aspekt | Vereinfacht | Offiziell |
|--------|---|---|
| IDs | 0x100-0x107 | 0x120-0x12f |
| Datentyp | Integer + Scaling | IEEE 754 F32 |
| Einheiten | Beliebig | SI Standard |
| Komplexität | Niedrig | Mittel |
| Performance | Sehr schnell | Schnell |
| Standard | Proprietär | Larus Spec |
| Treiber | Alle | Larus Sensorbox |

**Die Bridge unterstützt BEIDE! Wähle den besten für Deine Anwendung.**

---

*Version: 2.0 | 2026-11-12 | Dual-Mode Support*
