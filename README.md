# GeoLinker Lite
![License](https://img.shields.io/badge/License-MIT-blue.svg)
![Platforms](https://img.shields.io/badge/Platforms-Arduino%20Uno%20R3%20%7C%20Arduino%20Nano%20%7C%20AVR-green)
![Version](https://img.shields.io/badge/Version-1.0.0-brightgreen)

GeoLinkerLite Cloud API Library

A lightweight Arduino library for GPS tracking and cloud data transmission, specifically designed for low-memory devices like Arduino Uno R3 and Nano. The GeoLinkerLite library provides easy integration with GeoLinker cloud platform.

## 🚀 Features

- **Low-Memory Optimized**: Specifically designed for Arduino Uno R3, Nano, and similar AVR-based boards
- **GSM/GPRS Support**: Tested with SIM800L
- **Retry Mechanisms**: Robust error handling and retry logic
- **Timezone Support**: Configurable time offset for local timezone
- **Debug Levels**: Comprehensive debugging with multiple verbosity levels

## 📋 Requirements

### Hardware
- Arduino Uno R3, Arduino Nano, or compatible AVR-based microcontroller
- GPS module with NMEA output capability (tested with common GPS modules)
- GSM module (SIM800L recommended)
- Stable power supply

### Software Dependencies
- **SoftwareSerial** library (included with Arduino IDE)
- **EEPROM** library (included with Arduino IDE)

## 🔧 Installation

### Method 1: Arduino Library Manager (Recommended)
1. **Open Arduino IDE**
2. **Navigate to Library Manager**
   ```
   Tools → Manage Libraries... (or Ctrl+Shift+I)
   ```
3. **Search for GeoLinkerLite**
   - Type "GeoLinkerLite" in the search box
   - Install latest version
4. **Include in Your Sketch**
   ```cpp
   #include <GeoLinkerLite.h>
   ```

### Method 2: Manual Installation
1. **Download Library Files**
   - Download `GeoLinkerLite.h` and `GeoLinkerLite.cpp` from the repository
   - Create a folder named `GeoLinkerLite` in your Arduino libraries directory

2. **Locate Arduino Libraries Folder**
   - **Windows**: `Documents\Arduino\libraries\`
   - **macOS**: `~/Documents/Arduino/libraries/`
   - **Linux**: `~/Arduino/libraries/`

3. **Install Library Files**
   - Copy the library files into `Arduino/libraries/GeoLinkerLite/` following this structure:
     ```
     Arduino/libraries/GeoLinkerLite/
     ├── src/
     │   ├── GeoLinkerLite.h
     │   └── GeoLinkerLite.cpp
     ├── examples/
     │   └── GeoLinkerLite/
     │       └── GeoLinkerLite.ino
     ├── library.properties
     ├── library.json
     ├── keywords.txt
     └── README.md
     ```

4. **Include in Your Sketch**
   ```cpp
   #include <GeoLinkerLite.h>
   ```

## 🏗️ Hardware Setup

### Wiring Diagram
```
Arduino Uno R3/Nano → GPS Module
VCC                 → 3.3V or 5V (check module requirements)
GND                 → GND
Pin 0 (RX)          → GPS TX
NC                  → GPS RX (GPS module RX not connected. Arduino TX pin is used for debugging.)

Arduino Uno R3/Nano → GSM Module (SIM800L)
                    → 3.7V-4.2V (dedicated power supply)
GND                 → GND (common ground)
Pin 8               → GSM RX via voltage divider (see below)
Pin 9               → GSM TX


    Arduino Pin 8 (TX) ──┐
    5V signal            │
                      ┌──┴──┐
                      │ 1kΩ │  R1
                      └──┬──┘
                         │
     GSM RX ─────────────┼─── Safe 3.3V level (~3.3V)
                         │
                         │
                      ┌──┴──┐
                      │ 2kΩ │  R2
                      └──┬──┘
                         │
                        GND

Arduino Uno R3/Nano → Arduino Uno R3/Nano
Pin 2               → Reset control, Must be connected to the RST pin of the Arduino for self reset.
```

### Power Supply Considerations
- Use a stable power supply for both Arduino and GSM module
- GSM modules require significant current during transmission
- Consider using a dedicated power supply or battery pack for GSM module
- Ensure common ground between all components

## 📝 Usage

### Basic Example
```cpp
#include <GeoLinkerLite.h>

// Create the GeoLinkerLite instance
// Using Serial for both debug and GPS
GeoLinkerLite geoLinker(Serial, Serial);

void setup() {
    Serial.begin(9600);
    delay(1000);
    
    // Configure settings (optional - defaults are set in the library)
    geoLinker.setResetPin(2);                    // Reset control pin
    geoLinker.setGSMPins(8, 9);                  // GSM RX, TX pins
    geoLinker.setModemAPN("your.apn.here");      // Your carrier's APN
    geoLinker.setAPIKey("your_api_key");         // Your GeoLinker API key
    geoLinker.setDeviceID("arduino_tracker");    // Unique device ID
    geoLinker.setMaxRetries(3);                  // Max retry attempts
    geoLinker.setDebugLevel(1);                  // Debug level DEBUG_NONE (0), DEBUG_BASIC (1), DEBUG_VERBOSE (2)
    geoLinker.setTimeOffset(5, 30);              // Timezone: +5:30 hours
    
    // Initialize the library
    geoLinker.begin();
    
    // Run the main functionality
    delay(1000);                                // Increase this delay to increase upate interval
    geoLinker.run();
}

void loop() {
    // Should never reach here as both modes end with reset
    delay(1000);
}
```

### Common APN Settings
```cpp
// India
geoLinker.setModemAPN("airtelgprs.com");     // Airtel
geoLinker.setModemAPN("www");                // Jio
geoLinker.setModemAPN("bsnlnet");            // BSNL
geoLinker.setModemAPN("internet");           // VI

// International
geoLinker.setModemAPN("internet");           // Generic
```

## 🔧 API Reference

### Configuration Methods

#### `GeoLinkerLite(Stream &debugSerial, Stream &gpsSerial)`
Constructor that initializes the GeoLinkerLite instance.
- **Parameters:**
  - `debugSerial`: Serial stream for debug output (usually `Serial`)
  - `gpsSerial`: Serial stream for GPS communication (usually `Serial`)

#### `void setResetPin(uint8_t pin)`
Set the reset control pin.
- **Parameters:** `pin` — Arduino pin number (default: 2)

#### `void setGSMPins(uint8_t rxPin, uint8_t txPin)`
Set the GSM module communication pins.
- **Parameters:**
  - `rxPin`: Arduino pin connected to GSM RX (default: 8)
  - `txPin`: Arduino pin connected to GSM TX (default: 9)

#### `void setModemAPN(const char* apn)`
Set the cellular carrier's Access Point Name.
- **Parameters:** `apn` — APN string (default: "internet")

#### `void setAPIKey(const char* key)`
Set your GeoLinker API key.
- **Parameters:** `key` — API key string

#### `void setDeviceID(const char* id)`
Set the unique device identifier.
- **Parameters:** `id` — Device name or ID (default: "GeoLinker_tracker")

#### `void setMaxRetries(uint8_t retries)`
Set maximum retry attempts for data transmission.
- **Parameters:** `retries` — Number of retries (default: 100)

#### `void setDebugLevel(uint8_t level)`
Set debug verbosity level.
- **Parameters:** `level` — `DEBUG_NONE` (0), `DEBUG_BASIC` (1), `DEBUG_VERBOSE` (2)

#### `void setTimeOffset(int8_t hours, int8_t minutes)`
Set timezone offset from UTC.
- **Parameters:**
  - `hours`: Hours offset (-12 to +14)
  - `minutes`: Minutes offset (0, 15, 30, 45)

### Main Functions

#### `void begin()`
Initialize the GeoLinkerLite library. Call this in your `setup()` function.

#### `void run()`
Main library function that handles GPS collection and data transmission. Call this once in your `setup()` function after `begin()`.

## 🔄 Operation Modes

GeoLinkerLite operates in two distinct modes using EEPROM flags:

### GPS Mode
- Waits for GPS fix and valid NMEA data
- Parses coordinates and timestamp
- Saves data to EEPROM with reliability checks
- Triggers reset to switch to GSM mode

### GSM Mode
- Reads GPS data from EEPROM
- Establishes cellular connection
- Sends data to GeoLinker cloud service
- Clears EEPROM data after successful transmission
- Triggers reset to return to GPS mode

## 🌐 Cloud Integration

### API Endpoint
- **URL**: `https://www.circuitdigest.cloud/geolinker`
- **Method**: POST
- **Headers**: 
  - `Authorization: YOUR_API_KEY`
  - `Content-Type: application/json`

### Data Format
The library sends JSON data to the GeoLinker cloud service:

```json
{
  "device_id": "arduino_tracker",
  "lat": [12.9716],
  "long": [77.5946],
  "timestamp": ["2025-06-26 14:30:45"]
}
```

## 🐛 Debugging

### Debug Levels

```cpp
geoLinker.setDebugLevel(0);     // No debug output
geoLinker.setDebugLevel(1);    // Basic status messages
geoLinker.setDebugLevel(2);  // Detailed GPS and network info
```

### Common Debug Messages

**GPS Mode:**
```
[GeoLinker] GPS Mode: Waiting for GPS data...
[GeoLinker] GPS: Lat=12.971600 Lon=77.594600
[GeoLinker] GPS data saved to EEPROM
[GeoLinker] GPS mode complete, resetting...
```

**GSM Mode:**
```
[GeoLinker] GSM Mode: Sending data to server...
[GeoLinker] Network reg status: 1
[GeoLinker] GPRS attached: Yes
[GeoLinker] HTTP status: 200
[GeoLinker] Data sent successfully!
[GeoLinker] EEPROM data cleared
[GeoLinker] GSM mode complete, resetting...
```

## 🔧 Troubleshooting

### Common Issues

**GPS Not Getting Fix**
- Ensure GPS module has clear sky view
- Wait for GPS cold start (may take several minutes)
- Check wiring connections
- Enable verbose debugging to see raw NMEA data

**GSM Connection Issues**
- Verify APN settings for your carrier
- Check SIM card is properly inserted and activated
- Ensure GSM module has adequate power supply
- Check antenna connection

**Memory Issues**
- Avoid adding additional code, it can lead to memmory corruption.

### Power Supply Requirements
- Arduino: 5V via USB or 7-12V via barrel jack
- GPS Module: 3.3V-5V (check module specifications)
- GSM Module: 3.7V-4.2V with high current capability (2A peak)

## 📊 Memory Usage

GeoLinkerLite is optimized for low-memory devices:

- **Flash Memory**: Sketch uses 19766 bytes (61%) of program storage space
- **SRAM**: Global variables use 1217 bytes (59%) of dynamic memory, remaining is used by local variables.
- **EEPROM**: ~60 bytes for GPS data storage

## 🔒 License

This library is licensed under the MIT License. See the license header in source files for full details.

**Copyright (C) 2025 Jobit Joseph, Semicon Media Pvt Ltd (Circuit Digest)**

## 👨‍💻 Author

**Jobit Joseph**  
Semicon Media Pvt Ltd (Circuit Digest)

## 🔄 Version History

- **v1.0.0** - Initial release
- Features: GPS tracking, GSM data transmission
- Optimized for Arduino Uno R3 and Nano

---
