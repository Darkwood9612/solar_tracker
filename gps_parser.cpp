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

#include "gps_parser.h"

static int split_string_by_tokens(const String& str, char delimiter, String tokens[], int max_tokens) {
  int token_count = 0;
  int start = 0;
  int end = str.indexOf(delimiter);

  while (end != -1 && token_count < max_tokens - 1) {
    tokens[token_count++] = str.substring(start, end);
    start = end + 1;
    end = str.indexOf(delimiter, start);
  }

  tokens[token_count++] = str.substring(start);
  return token_count;
}

static int split_nmea_sentence(const String& str, String tokens[], int max_tokens) {

  int token_count = split_string_by_tokens(str, ',', tokens, max_tokens);

  int star_index = tokens[token_count - 1].indexOf('*');
  if (star_index != -1 && token_count < max_tokens) {
    String data_part = tokens[token_count - 1].substring(0, star_index);
    String checksum_part = tokens[token_count - 1].substring(star_index + 1);
    tokens[token_count - 1] = data_part;
    tokens[token_count++] = checksum_part;
  }

  return token_count;
}

static float convert_nmea_coordinate(const String& coordinate_string, char direction, bool is_longitude) {
  if (coordinate_string.length() == 0) {
    return 0.0;
  }

  int deg_length = is_longitude ? 3 : 2;
  float degrees = coordinate_string.substring(0, deg_length).toFloat();
  float minutes = coordinate_string.substring(deg_length).toFloat();
  float decimal_degrees = degrees + minutes / 60.0;

  return direction == 'S' || direction == 'W' ? -decimal_degrees : decimal_degrees;
}

// $--RMC,hhmmss.ss,A,llll.ll,a,yyyyy.yy,a,x.x,x.x,xxxx,x.x,a*hh
//  1) Time (UTC)
//  2) Status, V = Navigation receiver warning
//  3) Latitude
//  4) N or S
//  5) Longitude
//  6) E or W
//  7) Speed over ground, knots
//  8) Track made good, degrees true
//  9) Date, ddmmyy
// 10) Magnetic Variation, degrees
// 11) E or W
// 12) Checksum
static bool parse_rmc(const String& nmea, GPSData& utc_gps_data) {
  String tokens[13];
  int token_count = split_nmea_sentence(nmea, tokens, 13);

  if (token_count != 13) {
    return false;
  }

  bool is_valid = (tokens[2] == "A");

  if (is_valid && tokens[1].length() >= 6) {
    utc_gps_data.hour = tokens[1].substring(0, 2).toInt();
    utc_gps_data.minute = tokens[1].substring(2, 4).toInt();
    utc_gps_data.second = tokens[1].substring(4, 6).toInt();
    utc_gps_data.validity.time = is_time_valid(utc_gps_data);
  } else {
    utc_gps_data.validity.time = false;
  }

  if (is_valid && tokens[3].length() > 0 && tokens[4].length() > 0) {
    utc_gps_data.lat_direction = tokens[4].charAt(0);
    float lat = convert_nmea_coordinate(tokens[3], utc_gps_data.lat_direction, false);
    utc_gps_data.latitude = fabs(lat);
    utc_gps_data.validity.latitude = true;
  } else {
    utc_gps_data.validity.latitude = false;
  }

  if (is_valid && tokens[5].length() > 0 && tokens[6].length() > 0) {
    utc_gps_data.lon_direction = tokens[6].charAt(0);
    float lon = convert_nmea_coordinate(tokens[5], utc_gps_data.lon_direction, true);
    utc_gps_data.longitude = fabs(lon);
    utc_gps_data.validity.longitude = true;
  } else {
    utc_gps_data.validity.longitude = false;
  }

  if (is_valid && tokens[9].length() == 6) {
    utc_gps_data.day = tokens[9].substring(0, 2).toInt();
    utc_gps_data.month = tokens[9].substring(2, 4).toInt();
    utc_gps_data.year = 2000 + tokens[9].substring(4, 6).toInt();
    utc_gps_data.validity.date = is_date_valid(utc_gps_data);
  } else {
    utc_gps_data.validity.date = false;
  }

  return is_valid && utc_gps_data.validity.time && utc_gps_data.validity.latitude && utc_gps_data.validity.longitude && utc_gps_data.validity.date;
}

// $--GGA,hhmmss.ss,llll.ll,a,yyyyy.yy,a,x,xx,x.x,x.x,M,x.x,M,x.x,xxxx*hh
//  1) Time (UTC)
//  2) Latitude
//  3) N or S (North or South)
//  4) Longitude
//  5) E or W (East or West)
//  6) GPS Quality Indicator, 0 - fix not available, 1 - GPS fix, 2 - Differential GPS fix
//  7) Number of satellites in view, 00 - 12
//  8) Horizontal Dilution of precision
//  9) Antenna Altitude above/below mean-sea-level (geoid)
// 10) Units of antenna altitude, meters
// 11) Geoidal separation, the difference between the WGS-84 earth ellipsoid and mean-sea-level (geoid), "-" means mean-sea-level below ellipsoid
// 12) Units of geoidal separation, meters
// 13) Age of differential GPS data, time in seconds since last SC104 type 1 or 9 update, null field when DGPS is not used
// 14) Differential reference station ID, 0000-1023
// 15) Checksum
static bool parse_gga(const String& nmea, GPSData& utc_gps_data) {
  String tokens[16];
  int token_count = split_nmea_sentence(nmea, tokens, 16);

  if (token_count != 16) {
    return false;
  }

  if (tokens[1].length() >= 6) {
    utc_gps_data.hour = tokens[1].substring(0, 2).toInt();
    utc_gps_data.minute = tokens[1].substring(2, 4).toInt();
    utc_gps_data.second = tokens[1].substring(4, 6).toInt();
    utc_gps_data.validity.time = is_time_valid(utc_gps_data);
  } else {
    utc_gps_data.validity.time = false;
  }

  if (tokens[2].length() > 0 && tokens[3].length() > 0) {
    utc_gps_data.lat_direction = tokens[3].charAt(0);
    float lat = convert_nmea_coordinate(tokens[2], utc_gps_data.lat_direction, false);
    utc_gps_data.latitude = fabs(lat);
    utc_gps_data.validity.latitude = true;
  } else {
    utc_gps_data.validity.latitude = false;
  }

  if (tokens[4].length() > 0 && tokens[5].length() > 0) {
    utc_gps_data.lon_direction = tokens[5].charAt(0);
    float lon = convert_nmea_coordinate(tokens[4], utc_gps_data.lon_direction, true);
    utc_gps_data.longitude = fabs(lon);
    utc_gps_data.validity.longitude = true;
  } else {
    utc_gps_data.validity.longitude = false;
  }

  int fix_quality = tokens[6].toInt();
  bool is_fix_valid = (fix_quality > 0);

  if (is_fix_valid && tokens[9].length() > 0) {
    utc_gps_data.altitude = tokens[9].toFloat();
    utc_gps_data.validity.altitude = true;
  } else {
    utc_gps_data.validity.altitude = false;
  }

  return utc_gps_data.validity.time && utc_gps_data.validity.latitude && utc_gps_data.validity.longitude && utc_gps_data.validity.altitude;
}

// $--ZDA,hhmmss.ss,xx,xx,xxxx,xx,xx*hh
// 1) Local zone minutes description, same sign as local hours
// 2) Local zone description, 00 to +/- 13 hours
// 3) Year
// 4) Month, 01 to 12
// 5) Day, 01 to 31
// 6) Time (UTC)
// 7) Checksum
static bool parse_zda(const String& nmea, GPSData& utc_gps_data) {
  String tokens[8];
  int token_count = split_nmea_sentence(nmea, tokens, 8);

  if (token_count != 8) {
    return false;
  }

  if (tokens[1].length() >= 6) {
    utc_gps_data.hour = tokens[1].substring(0, 2).toInt();
    utc_gps_data.minute = tokens[1].substring(2, 4).toInt();
    utc_gps_data.second = tokens[1].substring(4, 6).toInt();
    utc_gps_data.validity.time = is_time_valid(utc_gps_data);
  } else {
    utc_gps_data.validity.time = false;
  }

  utc_gps_data.day = tokens[2].toInt();
  utc_gps_data.month = tokens[3].toInt();
  utc_gps_data.year = tokens[4].toInt();
  utc_gps_data.validity.date = is_date_valid(utc_gps_data);

  return utc_gps_data.validity.time && utc_gps_data.validity.date;
}

// $--GLL,llll.ll,a,yyyyy.yy,a,hhmmss.ss,A*hh
// 1) Latitude
// 2) N or S (North or South)
// 3) Longitude
// 4) E or W (East or West)
// 5) Time (UTC)
// 6) Status A - Data Valid, V - Data Invalid
// 7) Checksum
static bool parse_gll(const String& nmea, GPSData& utc_gps_data) {
  String tokens[8];
  int token_count = split_nmea_sentence(nmea, tokens, 8);

  if (token_count != 8) {
    return false;
  }

  if (tokens[1].length() > 0 && tokens[2].length() > 0) {
    utc_gps_data.lat_direction = tokens[2].charAt(0);
    float lat = convert_nmea_coordinate(tokens[1], utc_gps_data.lat_direction, false);
    utc_gps_data.latitude = fabs(lat);
    utc_gps_data.validity.latitude = true;
  } else {
    utc_gps_data.validity.latitude = false;
  }

  if (tokens[3].length() > 0 && tokens[4].length() > 0) {
    utc_gps_data.lon_direction = tokens[4].charAt(0);
    float lon = convert_nmea_coordinate(tokens[3], utc_gps_data.lon_direction, true);
    utc_gps_data.longitude = fabs(lon);
    utc_gps_data.validity.longitude = true;
  } else {
    utc_gps_data.validity.longitude = false;
  }

  if (tokens[6] == "A" && tokens[5].length() >= 6) {
    utc_gps_data.hour = tokens[5].substring(0, 2).toInt();
    utc_gps_data.minute = tokens[5].substring(2, 4).toInt();
    utc_gps_data.second = tokens[5].substring(4, 6).toInt();
    utc_gps_data.validity.time = is_time_valid(utc_gps_data);
  } else {
    utc_gps_data.validity.time = false;
  }

  return utc_gps_data.validity.latitude && utc_gps_data.validity.longitude && utc_gps_data.validity.time;
}

bool parse_nmea(const String& nmea, GPSData& utc_gps_data) {

  int start = 0;
  bool overall_valid = false;

  while (true) {
    int pos = nmea.indexOf('$', start);

    if (pos == -1) {
      break;
    }

    int nextPos = nmea.indexOf('$', pos + 1);
    String sentence;

    if (nextPos == -1) {
      sentence = nmea.substring(pos);
      start = nmea.length();
    } else {
      sentence = nmea.substring(pos, nextPos);
      start = nextPos;
    }

    if (sentence.charAt(0) == '$' && sentence.length() >= 6) {
      String msgType = sentence.substring(3, 6);
      if (msgType == "RMC") {
        overall_valid |= parse_rmc(sentence, utc_gps_data);
      } else if (msgType == "GGA") {
        overall_valid |= parse_gga(sentence, utc_gps_data);
      } else if (msgType == "ZDA") {
        overall_valid |= parse_zda(sentence, utc_gps_data);
      } else if (msgType == "GLL") {
        overall_valid |= parse_gll(sentence, utc_gps_data);
      }
    }
  }

  return overall_valid;
}
