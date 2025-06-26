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
#include "GeoLinkerLite.h"
#include <SoftwareSerial.h>

GeoLinkerLite::GeoLinkerLite(Stream &debugSerial, Stream &gpsSerial) {
    _debugSerial = &debugSerial;
    _gpsSerial = &gpsSerial;
}

void GeoLinkerLite::setResetPin(uint8_t pin) { _resetPin = pin; }
void GeoLinkerLite::setGSMPins(uint8_t rxPin, uint8_t txPin) { 
    _gsmRxPin = rxPin; 
    _gsmTxPin = txPin; 
}
void GeoLinkerLite::setModemAPN(const char* apn) { _modemAPN = apn; }
void GeoLinkerLite::setAPIKey(const char* key) { _apiKey = key; }
void GeoLinkerLite::setDeviceID(const char* id) { _deviceID = id; }
void GeoLinkerLite::setMaxRetries(uint8_t retries) { _maxRetries = retries; }
void GeoLinkerLite::setDebugLevel(uint8_t level) { _debugLevel = level; }
void GeoLinkerLite::setTimeOffset(int8_t hours, int8_t minutes) {
    _offsetHour = hours;
    _offsetMin = minutes;
}

void GeoLinkerLite::begin() {
    pinMode(_resetPin, INPUT);
    debugPrint_P(PSTR("GeoLinker Lite Starting..."), DEBUG_BASIC);
}

void GeoLinkerLite::run() {
    uint8_t eepromFlag = EEPROM.read(EEPROM_FLAG_ADDR);
    eepromFlag == GPS_READY_FLAG ? handleGSMMode() : handleGPSMode();
}

// ========================================
// DEBUG FUNCTIONS
// ========================================
void GeoLinkerLite::debugPrint(const String& msg, uint8_t level) {
    if (_debugLevel >= level) {
        _debugSerial->print(F("[GeoLinker] "));
        _debugSerial->println(msg);
    }
}

void GeoLinkerLite::debugPrint_P(PGM_P msg, uint8_t level) {
    if (_debugLevel >= level) {
        _debugSerial->print(F("[GeoLinker] "));
        _debugSerial->println((__FlashStringHelper*)msg);
    }
}

// ========================================
// EEPROM FUNCTIONS
// ========================================
void GeoLinkerLite::writeStringWithLengthToEEPROM(int address, const char* str, int maxLength) {
    int len = strlen(str);
    if (len >= maxLength) len = maxLength - 1; // Reserve space for null terminator
    
    // Write length byte first
    EEPROM.write(address, (uint8_t)len);
    
    // Write string data
    for (int i = 0; i < len; i++) {
        EEPROM.write(address + 1 + i, str[i]);
    }
    
    // Write null terminator
    EEPROM.write(address + 1 + len, '\0');
    
    // Fill remaining bytes with 0x00 for safety
    for (int i = len + 2; i <= maxLength; i++) {
        EEPROM.write(address + i, 0x00);
    }
    
    debugPrint("Stored string '" + String(str) + "' at addr " + String(address) + " with length " + String(len), DEBUG_VERBOSE);
}

void GeoLinkerLite::readStringWithLengthFromEEPROM(int address, char* buffer, int maxLength) {
    // Read length byte
    uint8_t storedLength = EEPROM.read(address);
    
    // Validate stored length
    if (storedLength >= maxLength) {
        storedLength = maxLength - 1; // Ensure we don't overflow buffer
    }
    
    // Read string data
    for (int i = 0; i < storedLength; i++) {
        buffer[i] = EEPROM.read(address + 1 + i);
    }
    
    // Ensure null termination
    buffer[storedLength] = '\0';
    
    debugPrint("Read string '" + String(buffer) + "' from addr " + String(address) + " with length " + String(storedLength), DEBUG_VERBOSE);
}

void GeoLinkerLite::saveGPSDataToEEPROM(float lat, float lon, const char* timestamp) {
    // Convert coordinates to strings with 6 decimal precision
    char latStr[LAT_STR_LENGTH];
    char lonStr[LON_STR_LENGTH];
    
    dtostrf(lat, 0, 6, latStr);
    dtostrf(lon, 0, 6, lonStr);
    
    // Save all data as strings with length prefixes
    writeStringWithLengthToEEPROM(EEPROM_LAT_ADDR, latStr, LAT_STR_LENGTH);
    writeStringWithLengthToEEPROM(EEPROM_LON_ADDR, lonStr, LON_STR_LENGTH);
    writeStringWithLengthToEEPROM(EEPROM_TIME_ADDR, timestamp, TIME_STR_LENGTH);
    EEPROM.write(EEPROM_FLAG_ADDR, GPS_READY_FLAG);
    
    debugPrint("Saved to EEPROM - Lat: " + String(latStr) + " Lon: " + String(lonStr) + " Time: " + String(timestamp), DEBUG_BASIC);
}

void GeoLinkerLite::readGPSDataFromEEPROM(char* latStr, char* lonStr, char* timestamp) {
    readStringWithLengthFromEEPROM(EEPROM_LAT_ADDR, latStr, LAT_STR_LENGTH);
    readStringWithLengthFromEEPROM(EEPROM_LON_ADDR, lonStr, LON_STR_LENGTH);
    readStringWithLengthFromEEPROM(EEPROM_TIME_ADDR, timestamp, TIME_STR_LENGTH);
}

void GeoLinkerLite::clearEEPROMData() {
    EEPROM.write(EEPROM_FLAG_ADDR, 0x00);
    // Clear GPS data including length bytes
    for (int i = EEPROM_LAT_ADDR; i < EEPROM_TIME_ADDR + TIME_STR_LENGTH + 1; i++) {
        EEPROM.write(i, 0x00);
    }
    debugPrint_P(PSTR("EEPROM data cleared including length bytes"), DEBUG_VERBOSE);
}

// ========================================
// GPS FUNCTIONS
// ========================================
bool GeoLinkerLite::parseNMEA(const char* line, float& lat, float& lon, char* timestamp) {
    if (!line || strlen(line) < 20) {
        debugPrint_P(PSTR("GPRMC line too short"), DEBUG_BASIC);
        return false;
    }
    
    // Make a working copy
    char workingCopy[GPS_BUFFER_SIZE];
    strncpy(workingCopy, line, sizeof(workingCopy) - 1);
    workingCopy[sizeof(workingCopy) - 1] = '\0';
    
    char* fields[12];
    int fieldCount = 0;
    
    // Split into fields
    char* token = strtok(workingCopy, ",");
    while (token != NULL && fieldCount < 12) {
        fields[fieldCount] = token;
        fieldCount++;
        token = strtok(NULL, ",");
    }
    
    if (fieldCount < 10) {
        debugPrint_P(PSTR("Not enough GPRMC fields"), DEBUG_BASIC);
        return false;
    }
    
    char* timeStr = fields[1];    // Time
    char* statusStr = fields[2];  // Status (A/V)
    char* latStr = fields[3];     // Latitude
    char* latDirStr = fields[4];  // N/S
    char* lonStr = fields[5];     // Longitude
    char* lonDirStr = fields[6];  // E/W
    char* dateStr = fields[9];    // Date
    
    // Validate fields
    if (!timeStr || !statusStr || !latStr || !latDirStr || 
        !lonStr || !lonDirStr || !dateStr ||
        strlen(timeStr) < 6 || strlen(dateStr) != 6) {
        debugPrint_P(PSTR("GPS NMEA format invalid"), DEBUG_BASIC);
        return false;
    }
    
    // Check GPS fix status
    if (statusStr[0] != 'A') {
        debugPrint_P(PSTR("GPS data invalid (no fix)"), DEBUG_BASIC);
        return false;
    }
    
    // Parse coordinates
    float rawLat = atof(latStr);
    float rawLon = atof(lonStr);
    
    int latDeg = (int)(rawLat / 100);
    float latMin = rawLat - latDeg * 100;
    lat = latDeg + (latMin / 60.0);
    if (latDirStr[0] == 'S') lat = -lat;
    
    int lonDeg = (int)(rawLon / 100);
    float lonMin = rawLon - lonDeg * 100;
    lon = lonDeg + (lonMin / 60.0);
    if (lonDirStr[0] == 'W') lon = -lon;
    
    // Parse date and time
    uint8_t day = (dateStr[0] - '0') * 10 + (dateStr[1] - '0');
    uint8_t month = (dateStr[2] - '0') * 10 + (dateStr[3] - '0');
    uint8_t year = (dateStr[4] - '0') * 10 + (dateStr[5] - '0');
    
    uint8_t hh = (timeStr[0] - '0') * 10 + (timeStr[1] - '0');
    uint8_t mm = (timeStr[2] - '0') * 10 + (timeStr[3] - '0');
    uint8_t ss = (timeStr[4] - '0') * 10 + (timeStr[5] - '0');
    
    // Apply time offset
    int totalMin = hh * 60 + mm + _offsetHour * 60 + _offsetMin;
    if (totalMin < 0) totalMin += 24 * 60;
    if (totalMin >= 24 * 60) totalMin -= 24 * 60;
    
    hh = totalMin / 60;
    mm = totalMin % 60;
    
    // Format timestamp
    snprintf(timestamp, 20, "20%02d-%02d-%02d %02d:%02d:%02d", year, month, day, hh, mm, ss);
    
    debugPrint("GPS: Lat=" + String(lat, 6) + " Lon=" + String(lon, 6), DEBUG_BASIC);
    return true;
}

void GeoLinkerLite::handleGPSMode() {
    debugPrint_P(PSTR("GPS Mode: Waiting for GPS data..."), DEBUG_BASIC);
    
    // Allocate GPS buffers
    char* gpsBuffer = (char*)malloc(GPS_BUFFER_SIZE);
    char* lastValidGPRMC = (char*)malloc(GPS_BUFFER_SIZE);
    
    if (!gpsBuffer || !lastValidGPRMC) {
        debugPrint_P(PSTR("Memory allocation failed!"), DEBUG_BASIC);
        return;
    }
    
    memset(gpsBuffer, 0, GPS_BUFFER_SIZE);
    memset(lastValidGPRMC, 0, GPS_BUFFER_SIZE);
    
    uint8_t gpsBufferIndex = 0;
    bool gpsDataValid = false;
    unsigned long startTime = millis();
    const unsigned long gpsTimeout = 300000; // 5 minutes timeout
    
    while (!gpsDataValid && (millis() - startTime < gpsTimeout)) {
        while (_gpsSerial->available()) {
            char c = _gpsSerial->read();
            
            if (c == '\n') {
                gpsBuffer[gpsBufferIndex] = '\0';
                
                if (strncmp(gpsBuffer, "$GPRMC", 6) == 0 && gpsBufferIndex > 20) {
                    strncpy(lastValidGPRMC, gpsBuffer, GPS_BUFFER_SIZE - 1);
                    lastValidGPRMC[GPS_BUFFER_SIZE - 1] = '\0';
                    
                    float lat, lon;
                    char timestamp[20];
                    
                    if (parseNMEA(lastValidGPRMC, lat, lon, timestamp)) {
                        // Save to EEPROM as strings
                        saveGPSDataToEEPROM(lat, lon, timestamp);
                        debugPrint_P(PSTR("GPS data saved to EEPROM"), DEBUG_BASIC);
                        gpsDataValid = true;
                        break;
                    }
                }
                gpsBufferIndex = 0;
            } else if (c != '\r') {
                if (gpsBufferIndex < GPS_BUFFER_SIZE - 1) {
                    gpsBuffer[gpsBufferIndex++] = c;
                }
            }
            
            if (gpsBufferIndex >= GPS_BUFFER_SIZE - 1) {
                gpsBufferIndex = 0;
            }
        }
        delay(10);
    }
    
    // Free GPS buffers
    free(gpsBuffer);
    free(lastValidGPRMC);
    
    if (!gpsDataValid) {
        debugPrint_P(PSTR("GPS timeout - no valid data received"), DEBUG_BASIC);
    }
    
    // Wait and trigger reset
    debugPrint_P(PSTR("GPS mode complete, resetting..."), DEBUG_BASIC);
    delay(2500);
    pinMode(_resetPin, OUTPUT);
    digitalWrite(_resetPin, LOW);
}

// ========================================
// GSM FUNCTIONS
// ========================================
String GeoLinkerLite::modemSendAT(const String& cmd, uint32_t timeout, const char* expect) {
    if (!_modemSerial) return "";
    
    // Clear RX buffer
    while (_modemSerial->available()) _modemSerial->read();
    
    if (cmd.length() > 0) {
        _modemSerial->println(cmd);
        debugPrint(">> " + cmd, DEBUG_VERBOSE);
    }
    
    unsigned long start = millis();
    String response = "";
    
    while (millis() - start < timeout) {
        while (_modemSerial->available()) {
            char c = _modemSerial->read();
            response += c;
            
            if (response.length() > 512) {
                response = response.substring(response.length() - 256);
            }
        }
        
        if (expect && response.indexOf(expect) != -1) {
            break;
        }
        delay(10);
    }
    
    debugPrint("<< " + response, DEBUG_VERBOSE);
    return response;
}

bool GeoLinkerLite::checkNetworkRegistration() {
    modemSendAT("AT+CIPCLOSE", modem_cmdTimeout, "CLOSE");
    modemSendAT("AT+CIPSHUT", modem_cmdTimeout, "SHUT OK");
    String resp = modemSendAT("AT+CREG?", modem_cmdTimeout, "+CREG:");
    int idx = resp.indexOf(',');
    int status = (idx != -1 && idx + 1 < resp.length()) ? resp.substring(idx + 1, idx + 2).toInt() : -1;
    
    debugPrint("Network reg status: " + String(status), DEBUG_BASIC);
    return (status == 1 || status == 5);
}

bool GeoLinkerLite::checkGprsContext() {
    String resp = modemSendAT("AT+CGATT?", modem_cmdTimeout, "+CGATT:");
    bool attached = (resp.indexOf("1") != -1);
    
    if (!attached) {
        modemSendAT("AT+CGATT=1", modem_cmdTimeout, "OK");
        resp = modemSendAT("AT+CGATT?", modem_cmdTimeout, "+CGATT:");
        attached = (resp.indexOf("1") != -1);
    }
    
    debugPrint("GPRS attached: " + String(attached ? "Yes" : "No"), DEBUG_BASIC);
    return attached;
}

bool GeoLinkerLite::modemHttpPost(const String& json, int& httpStatus, String& httpResponse) {
    debugPrint("Sending Data using GSM...", DEBUG_BASIC);
    
    // 0. Always close any existing session before new connection
    modemSendAT("AT+CIPCLOSE", modem_cmdTimeout, "CLOSE");
    modemSendAT("AT+CIPSHUT", modem_cmdTimeout, "SHUT OK");
    
    // 1. Configure GPRS
    modemSendAT("AT+CGATT=1", modem_cmdTimeout, "OK");
    modemSendAT("AT+CIPMUX=0", modem_cmdTimeout, "OK");
    modemSendAT("AT+CSTT=\"" + String(_modemAPN) + "\"", modem_cmdTimeout, "OK");
    modemSendAT("AT+CIICR", modem_cmdTimeout, "OK");
    modemSendAT("AT+CIFSR", modem_cmdTimeout, "."); // Get IP
    
    // 2. Open TCP connection to server (port 80 for HTTP)
    modemSendAT(F("AT+CIPSTART=\"TCP\",\"www.circuitdigest.cloud\",80"), modem_cmdTimeout, "OK");
    
    // 3. Wait for "CONNECT"
    unsigned long connectStart = millis();
    bool isConnected = false;
    while (millis() - connectStart < 15000) {
        if (_modemSerial->find("CONNECT")) {
            isConnected = true;
            debugPrint_P(PSTR("Connected!"), DEBUG_BASIC);
            break;
        }
        delay(10);
    }

    if (!isConnected) {
        debugPrint_P(PSTR("TCP connection failed!"), DEBUG_BASIC);
        modemSendAT(F("AT+CIPCLOSE"), modem_cmdTimeout, "CLOSE");
        modemSendAT(F("AT+CIPSHUT"), modem_cmdTimeout, "SHUT OK");
        httpStatus = 0;
        return false;
    }

    // 4. Send HTTP request
    modemSendAT("AT+CIPSEND", modem_cmdTimeout, ">");
    delay(50); // Allow time for prompt
    
    String crlf = String(char(13)) + String(char(10));
    _modemSerial->print("POST /geolinker HTTP/1.1"); _modemSerial->print(crlf);
    _modemSerial->print("Host: www.circuitdigest.cloud"); _modemSerial->print(crlf);
    _modemSerial->print("Authorization: "); _modemSerial->print(_apiKey); _modemSerial->print(crlf);
    _modemSerial->print("Content-Type: application/json"); _modemSerial->print(crlf);
    _modemSerial->print("Content-Length: "); _modemSerial->print(String(json.length())); _modemSerial->print(crlf);
    _modemSerial->print(crlf);
    _modemSerial->print(json);
    _modemSerial->write(0x1A);
    
    // 5. Wait for response and extract HTTP status code
    unsigned long startTime = millis();
    httpStatus = 0; // Default to 0 if we can't parse
    bool foundHttpHeader = false;
    char statusBuffer[4] = {0}; // Buffer for 3-digit status code + null terminator
    
    while (millis() - startTime < modem_httpTimeout) {
        if (_modemSerial->find("HTTP/1.1 ")) {
            // Read next 3 characters (status code)
            for (int i = 0; i < 3; i++) {
                while (!_modemSerial->available()) {
                    if (millis() - startTime > modem_httpTimeout) {
                        goto finish;
                    }
                    delay(1);
                }
                statusBuffer[i] = _modemSerial->read();
            }
            statusBuffer[3] = '\0';
            httpStatus = atoi(statusBuffer);
            foundHttpHeader = true;
            break;
        }
        delay(10);
    }

finish:
    // Close connection
    modemSendAT("AT+CIPCLOSE", modem_cmdTimeout, "CLOSE");
    modemSendAT("AT+CIPSHUT", modem_cmdTimeout, "SHUT OK");
    
    debugPrint("HTTP status: " + String(httpStatus), DEBUG_BASIC);
    return (httpStatus >= 200 && httpStatus < 300);
}

void GeoLinkerLite::handleGSMMode() {
    debugPrint_P(PSTR("GSM Mode: Sending data to server..."), DEBUG_BASIC);
    
    // Allocate GSM resources
    SoftwareSerial* modemSerial = new SoftwareSerial(_gsmRxPin, _gsmTxPin);
    modemSerial->begin(9600);
    _modemSerial = modemSerial;
    
    // Read GPS data from EEPROM as strings
    char latStr[LAT_STR_LENGTH];
    char lonStr[LON_STR_LENGTH];
    char timestamp[TIME_STR_LENGTH];
    
    readGPSDataFromEEPROM(latStr, lonStr, timestamp);
    float lat = atof(latStr);
    float lon = atof(lonStr);
    
    debugPrint("Loaded GPS data: Lat=" + String(latStr) + " Lon=" + String(lonStr) + " Time=" + String(timestamp), DEBUG_BASIC);
    
    // Build JSON payload using string values directly
    String json = "{";
    json += "\"device_id\":\"" + String(_deviceID) + "\",";
    json += "\"lat\":[" + String(lat, 6) + "],";  // Array format with 6 decimal places
    json += "\"long\":[" + String(lon, 6) + "],"; // Array format with 6 decimal places
    json += "\"timestamp\":[\"" + String(timestamp) + "\"]";  // Array format for timestamp
    json += "}";
    
    debugPrint("JSON: " + json, DEBUG_VERBOSE);
    
    bool success = false;
    uint8_t retryCount = 0;
    String lastError = "";
    
    while (!success && retryCount < _maxRetries) {
        retryCount++;
        debugPrint("Attempt " + String(retryCount) + "/" + String(_maxRetries), DEBUG_BASIC);
        
        // Check network registration
        if (!checkNetworkRegistration()) {
            lastError = "Network not registered";
            debugPrint(lastError, DEBUG_BASIC);
            delay(2000);
            continue;
        }
        
        // Check GPRS context
        if (!checkGprsContext()) {
            lastError = "GPRS context error";
            debugPrint(lastError, DEBUG_BASIC);
            delay(2000);
            continue;
        }
        
        // Send HTTP request
        int httpStatus;
        String httpResponse;
        
        if (modemHttpPost(json, httpStatus, httpResponse)) {
            debugPrint_P(PSTR("Data sent successfully!"), DEBUG_BASIC);
            success = true;
        } else {
            lastError = "HTTP Error: " + String(httpStatus);
            debugPrint(lastError, DEBUG_BASIC);
            if (retryCount < _maxRetries) {
                delay(3000); // Wait before retry
            }
        }
    }
    
    // Clean up GSM resources
    delete modemSerial;
    _modemSerial = nullptr;
    
    // Print final result
    if (success) {
        debugPrint_P(PSTR("SUCCESS: Data transmission completed"), DEBUG_BASIC);
    } else {
        debugPrint("FAILED: " + lastError + " (Max retries reached)", DEBUG_BASIC);
    }
    
    // Clear EEPROM data
    clearEEPROMData();
    debugPrint_P(PSTR("EEPROM data cleared"), DEBUG_BASIC);
    
    // Wait and trigger reset
    debugPrint_P(PSTR("GSM mode complete, resetting..."), DEBUG_BASIC);
    delay(3500);
    pinMode(_resetPin, OUTPUT);
    digitalWrite(_resetPin, LOW);
}
