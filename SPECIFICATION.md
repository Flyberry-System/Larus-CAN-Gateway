# 🛩️ Larus CAN Spezifikation - Vollständige Referenz

## 📊 Systemarchitektur

```
┌─────────────────────────────────────────────────────┐
│            Larus Flight Information System            │
└─────────────────────────────────────────────────────┘

SENSOREN & INPUT (senden Daten)
│
├─ Larus Sensor Box (Object ID 2)        Bits 0x120-0x12f
│  ├─ 0x120: Roll & Pitch              (2× F32 rad)
│  ├─ 0x121: Yaw & Turn Rate           (2× F32 rad, rad/s)
│  ├─ 0x122: TAS & IAS                 (2× F32 m/s)
│  ├─ 0x123: Vario & Avg Vario         (2× F32 m/s)
│  ├─ 0x124: Wind Dir & Speed          (2× F32 rad, m/s)
│  ├─ 0x125: Avg Wind                  (2× F32 rad, m/s)
│  ├─ 0x126: Pressure & Density        (2× F32 Pa, kg/m³)
│  ├─ 0x127: G-Force                   (2× F32 m/s²)
│  ├─ 0x128: Apparent Attitude         (2× F32 rad)
│  ├─ 0x129: Supply Voltage & Mode     (F32 V, u8 mode)
│  ├─ 0x12a: System State              (u32, u32)
│  └─ 0x12b: Sensor Health             (2× F32)
│
├─ GPS Receiver (Object ID 3)            Bits 0x140-0x149
│  ├─ 0x140: Date & Time               (u16, u8×5)
│  ├─ 0x141: Latitude                  (F64 rad)
│  ├─ 0x142: Longitude                 (F64 rad)
│  ├─ 0x143: Altitude & Geoid Sep      (2× F32 m)
│  ├─ 0x144: Ground Track & Speed      (2× F32 rad, m/s)
│  ├─ 0x145: Satellites & Fix Type     (u8, u8)
│  ├─ 0x146: D-GNSS Accuracy           (2× F32 m, rad)
│  ├─ 0x147: D-GNSS Heading            (F32 rad)
│  ├─ 0x148: D-GNSS Rel Pos N/E        (2× F32 m)
│  └─ 0x149: D-GNSS Rel Pos D/Length   (2× F32 m)
│
├─ Magnetic Sensor (Object ID 6)         Bits 0x070
│  └─ 0x070: Magnetic X/Y/Z            (3× i16, 75LSB/μT)
│
└─ Ambient Sensor (Object ID 5)          Bits 0x2d0
   └─ 0x2d0: Temperature & Humidity    (2× F32 °C, %)

DISPLAY & OUTPUT (empfangen Daten)
│
└─ Vario Display (Object ID 4)           Bits 0x280-0x28f
   ├─ 0x280: Sound Output              (u16, u16, u8, bool)
   ├─ 0x281: Voltage & Temp            (2× F32 V, °C)
   └─ 0x282: Avg & Thermal Climb       (2× F32 m/s)
```

---

## 🔄 Larus Bridge - Unterstützte Formate

### Tier 1: VEREINFACHT (0x100-0x107) ✅
```
Eingabe:    8 CAN-Frames (Integer mit Scaling)
Ausgabe:    NMEA 0183 + Larus $PLARx
Komplexität: Niedrig
Use Case:   DIY, Prototyping
```

### Tier 2: OFFIZIELL LARUS SPEC (0x120-0x12f) ✅
```
Eingabe:    12 CAN-Frames (IEEE 754 F32)
Ausgabe:    NMEA 0183 + Larus $PLARx
Komplexität: Mittel
Use Case:   Larus Sensorbox, Production
```

### Tier 3: VOLL-SPEZIFIKATION (0x140-0x149, 0x070, 0x2d0) ⏳
```
Eingabe:    GPS + Magnetic + Ambient (16+ Frames)
Ausgabe:    Erweiterte NMEA + Status
Komplexität: Hoch
Use Case:   Vollständige Avioniksysteme
```

---

## 📋 Detaillierte Object Directory

### Larus Sensor Box (0x120-0x12f)

| ID | Name | Format | Update | Zweck |
|----|----|--------|--------|-------|
| 0x120 | roll_pitch | 2× F32 | 100ms | Attitude (rad) |
| 0x121 | yaw_turn_rate | 2× F32 | 100ms | Heading & rotation |
| 0x122 | airspeed | 2× F32 | 100ms | TAS/IAS (m/s) |
| 0x123 | vario | 2× F32 | 100ms | Climb rate (m/s) |
| 0x124 | wind | 2× F32 | 100ms | Wind (rad, m/s) |
| 0x125 | average_wind | 2× F32 | 100ms | Avg wind (rad, m/s) |
| 0x126 | atmosphere | 2× F32 | 100ms | Pressure, density |
| 0x127 | g_force | 2× F32 | 100ms | G-force (m/s²) |
| 0x128 | apparent_att | 2× F32 | 100ms | Filtered attitude |
| 0x129 | fly_state | F32+u8 | 100ms | Voltage, mode |
| 0x12a | system_state | 2× u32 | 100ms | Health bits |
| 0x12b | sensor_health | 2× F32 | 100ms | Quality metrics |

**Einheiten:** SI Standard (rad, m/s, Pa, kg/m³, m/s²)

---

### GPS Receiver (0x140-0x149)

| ID | Name | Format | Update | Zweck |
|----|----|--------|--------|-------|
| 0x140 | date_time | u16,u8×5 | 100ms | UTC time |
| 0x141 | latitude | F64 | 100ms | Position (rad) |
| 0x142 | longitude | F64 | 100ms | Position (rad) |
| 0x143 | altitude | 2× F32 | 100ms | MSL alt, geoid |
| 0x144 | track_speed | 2× F32 | 100ms | Ground track (m/s) |
| 0x145 | satellites | 2× u8 | 100ms | Sat count, fix type |
| 0x146 | dgnss_accuracy | 2× F32 | 100ms | Accuracy (m, rad) |
| 0x147 | dgnss_heading | F32 | 100ms | D-GNSS heading |
| 0x148 | dgnss_relpos_ne | 2× F32 | 100ms | Rel pos N/E (m) |
| 0x149 | dgnss_relpos_d | 2× F32 | 100ms | Rel pos D/len (m) |

**Hinweis:** Latitude/Longitude in **Radians** (nicht Grad!)

---

### Magnetic Sensor (0x070)

| ID | Name | Format | Update | Zweck |
|----|----|--------|--------|-------|
| 0x070 | magnetic_data | 3× i16 | 10ms | 3-axis mag field |

**Einheit:** 75 LSB/μT (Rohdaten für Kompass-Kalibrierung)

---

### Ambient Sensor (0x2d0)

| ID | Name | Format | Update | Zweck |
|----|----|--------|--------|-------|
| 0x2d0 | temp_humidity | 2× F32 | 10s | OAT, humidity |

**Einheiten:** °C, % RH

---

### Vario Display (0x280-0x28f)

| ID | Name | Format | Update | Zweck |
|----|----|--------|--------|-------|
| 0x280 | sound | u16,u16,u8,bool | 100ms | Audio output |
| 0x281 | volt_temp | 2× F32 | 1s | Display voltage |
| 0x282 | avg_climb_rates | 2× F32 | 100ms | Thermal assist |

**Hinweis:** Display EMPFÄNGT, sendet nicht (für Bedienung)

---

## 🔄 Konvertierungen

### Vereinfacht → Offiziell
```c
// 0x101 (Vereinfacht): speed_u16÷100 (knots)
→ 0x122 (Offiziell): ias_f32 (m/s)
  ias_ms = (speed_knots / 100.0) * 0.51444  // knots to m/s
```

### Offiziell GPS → NMEA
```c
// 0x141 (Latitude F64 rad)
→ $GPGGA latitude field (degrees)
  lat_deg = latitude_rad * 180.0 / M_PI
```

### Offiziell → Vereinfacht (optional)
```c
// 0x123 (Vario F32 m/s)
→ 0x104 (TEK i16÷100)
  tek_i16 = (vario_ms * 100)
```

---

## 📡 NMEA Ausgabe (für alle Formate)

```
Standard NMEA 0183
├─ $GPRMC  Minimum recommended navigation
├─ $GPGGA  Global positioning system fix data

Larus Proprietary ($PLARx)
├─ $PLARA  Attitude (roll, pitch, yaw)
├─ $PLARV  Variometer (vario, altitude, TAS, GLoad)
├─ $PLARW  Wind (direction, speed)
├─ $PLARB  Battery voltage & temperature
├─ $PLARD  Air density
└─ $PLARS  Settings (bidirectional)
```

---

## 🎯 Implementierungs-Roadmap

### Phase 1: ✅ DONE
- [x] Vereinfacht Mode (0x100-0x107)
- [x] Offiziell Mode (0x120-0x12f)
- [x] NMEA Output

### Phase 2: ⏳ READY
- [ ] GPS Support (0x140-0x149)
- [ ] Magnetic Sensor (0x070)
- [ ] Ambient Sensor (0x2d0)

### Phase 3: 🔮 OPTIONAL
- [ ] Vario Display Input (0x280-0x28f)
- [ ] D-GNSS Features
- [ ] Generic Data Points (0x5xx, 0x6xx)

---

## 💡 Welches Format sollte ich verwenden?

| Anforderung | Empfehlung | Grund |
|---|---|---|
| Schnell testen | Vereinfacht (0x100-0x107) | Einfach zu debuggen |
| Prototyp | Vereinfacht | Flexible Skalierung |
| Larus Sensorbox | Offiziell (0x120-0x12f) | Native Format |
| GPS-Integration | GPS Module (0x140-0x149) | Offiziell support |
| Full System | Alle | Maximum Features |

---

## 🔗 Referenzen

- **Larus Spec Master:** https://github.com/larus-breeze/doc_larus
- **CAN Specification:** https://github.com/larus-breeze/doc_larus/blob/master/documentation/can_spec.md
- **Object Directory:** https://github.com/larus-breeze/doc_larus/tree/master/documentation/can_details/object_directory

---

## 🧮 Datentyp-Referenz

```
u8   = unsigned 8-bit    (0-255)
u16  = unsigned 16-bit   (0-65535)
u32  = unsigned 32-bit   (0-4.3B)
i16  = signed 16-bit     (-32768-32767)
i32  = signed 32-bit     (-2.1B-2.1B)
f32  = IEEE 754 float    (±3.4e±38)
f64  = IEEE 754 double   (±1.7e±308)
bool = 0=false, 1=true

Byte Order: Big-Endian (Motorola)
```

---

**Bridge Support Status:** Tier 1 & 2 ✅ | Tier 3 ⏳

*Stand: 2026-11-12 | Larus v0.1.6*
