
/* 
 * GeoLinkerLite Library
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
    geoLinker.setDeviceID("arduino_tracker");    // Unique device ID / Device name
    geoLinker.setMaxRetries(3);                  // Max retry attempts to send a data ponit via GPRS
    geoLinker.setDebugLevel(1);                  // Debug level
    geoLinker.setTimeOffset(5, 30);              // Timezone: Eg. india +5:30 hours
    
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