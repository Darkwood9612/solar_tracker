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

#include "sun_position.h"

#include <math.h>

constexpr float SECONDS_IN_HOUR = 3600.0f;
constexpr float MINUTES_IN_HOUR = 60.0f;
constexpr float DEGREES_PER_HOUR = 15.0f;
constexpr float JULIAN_DAY_OFFSET = 2451545.0f;

static float calculate_julian_date(const GPSData& gps_data) {
  int year = gps_data.year;
  int month = gps_data.month;
  int day = gps_data.day;

  if (month <= 2) {
    year--;
    month += 12;
  }

  int A = year / 100;
  int B = 2 - A + A / 4;

  float julian_date = floor(365.25f * (year + 4716)) + floor(30.6001f * (month + 1)) + day + B - 1524.5f;
  float time_fraction = (gps_data.hour + gps_data.minute / MINUTES_IN_HOUR + gps_data.second / SECONDS_IN_HOUR) / 24.0f;

  return julian_date + time_fraction;
}

void calculate_sun_position(const GPSData& gps_data, float& azimuth, float& elevation) {
  float julian_date = calculate_julian_date(gps_data);
  float D = julian_date - JULIAN_DAY_OFFSET;

  float mean_anomaly = fmod(357.529f + 0.98560028f * D, 360.0f);
  float mean_longitude = fmod(280.459f + 0.98564736f * D, 360.0f);
  float ecliptic_longitude = fmod(mean_longitude + 1.915f * sin(mean_anomaly * DEG_TO_RAD) + 0.020f * sin(2 * mean_anomaly * DEG_TO_RAD), 360.0f);

  float obliquity = 23.439f - 0.00000036f * D;
  float declination = asin(sin(obliquity * DEG_TO_RAD) * sin(ecliptic_longitude * DEG_TO_RAD)) * RAD_TO_DEG;

  float time_in_hours = gps_data.hour + gps_data.minute / MINUTES_IN_HOUR + gps_data.second / SECONDS_IN_HOUR;
  float hour_angle = (time_in_hours - 12.0f) * DEGREES_PER_HOUR;

  azimuth = atan2(sin(hour_angle * DEG_TO_RAD),
                  cos(hour_angle * DEG_TO_RAD) * sin(gps_data.latitude * DEG_TO_RAD) - tan(declination * DEG_TO_RAD) * cos(gps_data.latitude * DEG_TO_RAD))
            * RAD_TO_DEG;

  azimuth = fmod(azimuth + 180.0f, 360.0f);

  if (azimuth < 0) {
    azimuth += 360.0f;
  }

  elevation = asin(sin(gps_data.latitude * DEG_TO_RAD) * sin(declination * DEG_TO_RAD) + cos(gps_data.latitude * DEG_TO_RAD) * cos(declination * DEG_TO_RAD) * cos(hour_angle * DEG_TO_RAD)) * RAD_TO_DEG;
}
