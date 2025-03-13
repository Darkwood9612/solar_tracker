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

#ifndef GPS_DATA_H
#define GPS_DATA_H

#include <Arduino.h>

struct GPSValidity {
  bool latitude = false;
  bool longitude = false;
  bool date = false;
  bool time = false;
  bool altitude = false;
};

struct GPSData {
  float latitude = 0.0f;
  char lat_direction = '0';
  float longitude = 0.0f;
  char lon_direction = '0';
  float altitude = 0.0f;
  uint8_t hour = 0;
  uint8_t minute = 0;
  uint8_t second = 0;
  uint16_t year = 0;
  uint8_t month = 0;
  uint8_t day = 0;
  GPSValidity validity;
};

uint8_t days_in_month(uint16_t year, uint8_t month);

void print_gps_data(const GPSData& gps_data);
void print_validity_of_gps_data(const GPSData& gps_data);

GPSData shift_to_local_time_zone(const GPSData& utc_gps_data);

inline bool is_leap_year(uint16_t year) {
  return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

inline bool is_date_valid(const GPSData& gps_data) {
  return gps_data.year >= 2025 && gps_data.month <= 12 && gps_data.day <= days_in_month(gps_data.year, gps_data.month);
}

inline bool is_time_valid(const GPSData& gps_data) {
  return gps_data.hour < 24 && gps_data.minute < 60 && gps_data.second < 60;
}

inline bool is_gps_data_valid(const GPSData& gps_data) {
  return gps_data.validity.latitude && gps_data.validity.longitude && gps_data.validity.time && gps_data.validity.date && gps_data.validity.altitude;
}

#endif
