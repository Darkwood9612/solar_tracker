/********************************************************************************
 *  Copyright (c) 2025, Vladyslav Merais
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *
 *  2. Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *
 *  3. Neither the name of Vladyslav Merais nor the names of its
 *     contributors may be used to endorse or promote products derived from
 *     this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 *  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 *  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 *  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 *  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 *  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 *  OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 *  WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 *  OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 *  EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 ********************************************************************************/

#include <SoftwareSerial.h>

#include "gps_parser.h"
#include "sun_position.h"

constexpr uint8_t GPS_RX_PIN = 2;
constexpr uint8_t GPS_TX_PIN = 3;

constexpr long GPS_DATA_EXCHANGE_RATE = 9600;

GPSData utc_gps_data;

SoftwareSerial gps_serial(GPS_RX_PIN, GPS_TX_PIN);

bool update_utc_gps_data(GPSData& utc_gps_data) {
  if (!gps_serial.available()) {
    return false;
  }

  const String nmea_line = gps_serial.readStringUntil('\n');
  const bool is_success = parse_nmea(nmea_line, utc_gps_data);

  //String success = is_success ? "] = SUCCESS" : "] = FAIL";
  //Serial.println("Parse NMEA - [" +  nmea_line + success);

  return is_gps_data_valid(utc_gps_data);
}

void setup() {
  Serial.begin(GPS_DATA_EXCHANGE_RATE);
  gps_serial.begin(GPS_DATA_EXCHANGE_RATE);
}

void loop() {

  if (update_utc_gps_data(utc_gps_data) == false) {
    //print_validity_of_gps_data(utc_gps_data);
    return;
  }

  GPSData local_gps_data = shift_to_local_time_zone(utc_gps_data);;

  //print_gps_data(local_gps_data);

  float azimuth = 0.0f, elevation = 0.0f;
  calculate_sun_position(local_gps_data, azimuth, elevation);

  Serial.print(F("Azimuth: "));
  Serial.print(azimuth);
  Serial.print(F(" | Elevation: "));
  Serial.println(elevation);

}
