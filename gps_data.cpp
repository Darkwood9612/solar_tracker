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

#include "gps_data.h"

uint8_t days_in_month(uint16_t year, uint8_t month) {
  switch (month) {
    case 1:
    case 3:
    case 5:
    case 7:
    case 8:
    case 10:
    case 12:
      return 31;
    case 4:
    case 6:
    case 9:
    case 11:
      return 30;
    case 2:
      return is_leap_year(year) ? 29 : 28;
    default:
      return 30;
  }
}

static void increment_date_by_one_day(GPSData& gps_data) {

  uint8_t days_in_current_monts = days_in_month(gps_data.year, gps_data.month);

  gps_data.day++;

  if (gps_data.day <= days_in_current_monts) {
    return;
  }

  gps_data.day = 1;
  gps_data.month++;

  if (gps_data.month <= 12) {
    return;
  }

  gps_data.month = 1;
  gps_data.year++;
}

static void decrement_date_by_one_day(GPSData& gps_data) {

  gps_data.day--;

  if (gps_data.day >= 1) {
    return;
  }

  gps_data.month--;

  if (gps_data.month < 1) {
    gps_data.month = 12;
    gps_data.year--;
  }

  gps_data.day = days_in_month(gps_data.year, gps_data.month);
}

static GPSData adjust_time_zone(const GPSData& utc_gps_data, const int offset_hours) {

  GPSData local_gps_data = utc_gps_data;
  long total_seconds = utc_gps_data.hour * 3600L + utc_gps_data.minute * 60L + utc_gps_data.second + offset_hours * 3600L;

  while (total_seconds < 0L) {
    total_seconds += 24L * 3600L;
    decrement_date_by_one_day(local_gps_data);
  }
  while (total_seconds >= 24L * 3600L) {
    total_seconds -= 24L * 3600L;
    increment_date_by_one_day(local_gps_data);
  }

  local_gps_data.hour = total_seconds / 3600L;
  local_gps_data.minute = (total_seconds % 3600L) / 60L;
  local_gps_data.second = total_seconds % 60L;

  return local_gps_data;
}

static int calculate_time_zone_offset(float signed_longitude) {
  float offset = signed_longitude / 15.0f;
  return offset >= 0 ? (int)(offset + 0.5f) : (int)(offset - 0.5f);
}

GPSData shift_to_local_time_zone(const GPSData& utc_gps_data) {
  float signed_longitude = utc_gps_data.longitude;

  if (utc_gps_data.lon_direction == 'W') {
    signed_longitude = -signed_longitude;
  }

  return adjust_time_zone(utc_gps_data, calculate_time_zone_offset(signed_longitude));
}

void print_gps_data(const GPSData& gps_data) {
  Serial.println(F("----- GPS Data -----"));

  Serial.print(F("Latitude: "));
  Serial.print(gps_data.latitude, 6);
  Serial.print(F(" "));
  Serial.println(gps_data.lat_direction);

  Serial.print(F("Longitude: "));
  Serial.print(gps_data.longitude, 6);
  Serial.print(F(" "));
  Serial.println(gps_data.lon_direction);

  Serial.print(F("Altitude: "));
  Serial.println(gps_data.altitude);

  Serial.print(F("Time: "));
  if (gps_data.hour < 10) Serial.print('0');
  Serial.print(gps_data.hour);
  Serial.print(F(":"));
  if (gps_data.minute < 10) Serial.print('0');
  Serial.print(gps_data.minute);
  Serial.print(F(":"));
  if (gps_data.second < 10) Serial.print('0');
  Serial.println(gps_data.second);

  Serial.print(F("Date: "));
  if (gps_data.day < 10) Serial.print('0');
  Serial.print(gps_data.day);
  Serial.print(F("/"));
  if (gps_data.month < 10) Serial.print('0');
  Serial.print(gps_data.month);
  Serial.print(F("/"));
  Serial.println(gps_data.year);

  Serial.println(F("--------------------------"));
}

void print_validity_of_gps_data(const GPSData& gps_data) {
  Serial.println(F("----- Validity of GPS Data -----"));

  Serial.print(F(" Latitude: "));
  Serial.println(gps_data.validity.latitude ? F("OK") : F("FAIL"));

  Serial.print(F(" Longitude: "));
  Serial.println(gps_data.validity.longitude ? F("OK") : F("FAIL"));

  Serial.print(F(" Time: "));
  Serial.println(gps_data.validity.time ? F("OK") : F("FAIL"));

  Serial.print(F(" Date: "));
  Serial.println(gps_data.validity.date ? F("OK") : F("FAIL"));

  Serial.print(F(" Altitude: "));
  Serial.println(gps_data.validity.altitude ? F("OK") : F("FAIL"));

  Serial.println(F("--------------------------"));
}
