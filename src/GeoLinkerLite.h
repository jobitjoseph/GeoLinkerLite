/* 
 * GeoLinker Lite Library
 * Copyright (C) 2025 Jobit Joseph, Semicon Media Pvt Ltd (Circuit Digest)
 * Author: Jobit Joseph
 * Project: GeoLinkerLite Cloud API Library
 *
 * Licensed under the MIT License
 * You may not use this file except in compliance with the License.
 * 
 * You may obtain a copy of the License at:
 * https://opensource.org/license/mit/
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software (the "Software") and associated documentation files, to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, subject to the following additional conditions:

 * 1. All copies or substantial portions must retain:  
 *    - The original copyright notice  
 *    - A prominent statement crediting the original author/creator  

 * 2. Modified versions must:  
 *    - Clearly mark the changes as their own  
 *    - Preserve all original credit notices
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef GeoLinkerLite_h
#define GeoLinkerLite_h

#include <Arduino.h>
#include <Stream.h>
#include <EEPROM.h>
#include <avr/pgmspace.h>

class GeoLinkerLite {
  public:
    // Constructor
    GeoLinkerLite(Stream &debugSerial, Stream &gpsSerial);
    
    // Configuration methods
    void setResetPin(uint8_t pin);
    void setGSMPins(uint8_t rxPin, uint8_t txPin);
    void setModemAPN(const char* apn);
    void setAPIKey(const char* key);
    void setDeviceID(const char* id);
    void setMaxRetries(uint8_t retries);
    void setDebugLevel(uint8_t level);
    void setTimeOffset(int8_t hours, int8_t minutes);
    
    // Main functions
    void begin();
    void run();
    
  private:
    // Configuration
    uint8_t _resetPin = 2;
    uint8_t _gsmRxPin = 8;
    uint8_t _gsmTxPin = 9;
    const char* _modemAPN = "internet";
    const char* _apiKey = "";
    const char* _deviceID = "GeoLinker_tracker";
    uint8_t _maxRetries = 100;
    uint8_t _debugLevel = 2; // DEBUG_VERBOSE by default
    int8_t _offsetHour = 5;
    int8_t _offsetMin = 30;
    
    // Constants
    static const uint8_t EEPROM_FLAG_ADDR = 10;
    static const uint8_t EEPROM_LAT_ADDR = 20;
    static const uint8_t EEPROM_LON_ADDR = 33;
    static const uint8_t EEPROM_TIME_ADDR = 46;
    static const uint8_t GPS_READY_FLAG = 0x22;
    static const uint8_t LAT_STR_LENGTH = 12;
    static const uint8_t LON_STR_LENGTH = 12;
    static const uint8_t TIME_STR_LENGTH = 20;
    static const uint16_t GPS_BUFFER_SIZE = 100;
    static const uint16_t modem_cmdTimeout = 5000;
    static const uint16_t modem_httpTimeout = 15000;
    
    // Serial interfaces
    Stream* _debugSerial;
    Stream* _gpsSerial;
    Stream* _modemSerial;
    
    // Debug levels
    static const uint8_t DEBUG_NONE = 0;
    static const uint8_t DEBUG_BASIC = 1;
    static const uint8_t DEBUG_VERBOSE = 2;
    
    // Debug functions
    void debugPrint(const String& msg, uint8_t level);
    void debugPrint_P(PGM_P msg, uint8_t level);
    
    // EEPROM functions
    void writeStringWithLengthToEEPROM(int address, const char* str, int maxLength);
    void readStringWithLengthFromEEPROM(int address, char* buffer, int maxLength);
    void saveGPSDataToEEPROM(float lat, float lon, const char* timestamp);
    void readGPSDataFromEEPROM(char* latStr, char* lonStr, char* timestamp);
    void clearEEPROMData();
    
    // GPS functions
    bool parseNMEA(const char* line, float& lat, float& lon, char* timestamp);
    void handleGPSMode();
    
    // GSM functions
    String modemSendAT(const String& cmd, uint32_t timeout, const char* expect);
    bool checkNetworkRegistration();
    bool checkGprsContext();
    bool modemHttpPost(const String& json, int& httpStatus, String& httpResponse);
    void handleGSMMode();
};

#endif
