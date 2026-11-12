#include <SoftwareSerial.h>

#include "nmea_parser.h"
#include "sun_position.h"

constexpr uint8_t GPS_RX_PIN = 2;
constexpr uint8_t GPS_TX_PIN = 3;

constexpr long GPS_DATA_EXCHANGE_RATE = 9600;

solar_tracker::GPSData utc_gps_data;

SoftwareSerial gps_serial(GPS_RX_PIN, GPS_TX_PIN);

bool update_utc_gps_data(solar_tracker::GPSData& utc_gps_data) {
  if (!gps_serial.available()) {
    return false;
  }

  const String nmea_line = gps_serial.readStringUntil('\n');
  const bool is_success = solar_tracker::parse_nmea(nmea_line, utc_gps_data);

  //String success = is_success ? "] = SUCCESS" : "] = FAIL";
  //Serial.println("Parse NMEA - [" +  nmea_line + success);

  return solar_tracker::is_gps_data_valid(utc_gps_data);
}

void setup() {
  Serial.begin(GPS_DATA_EXCHANGE_RATE);
  gps_serial.begin(GPS_DATA_EXCHANGE_RATE);
}

void loop() {

  if (update_utc_gps_data(utc_gps_data) == false) {
    solar_tracker::print_validity_of_gps_data(utc_gps_data);
    return;
  }

  //print_gps_data(local_gps_data);

  double azimuth = 0.0, elevation = 0.0;
  solar_tracker::calculate_sun_position(utc_gps_data, azimuth, elevation);

  Serial.print(F("Azimuth: "));
  Serial.print(azimuth);
  Serial.print(F(" | Elevation: "));
  Serial.println(elevation);

}
