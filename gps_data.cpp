#include "gps_data.h"

#ifdef ARDUINO
#include <Arduino.h>
#define PRINT_DEBUG(x) Serial.print(x)
#define PRINT_NEW_LINE_DEBUG(x) Serial.println(x)
#else
#include <iostream>
#define PRINT_DEBUG(x) std::cout << (x)
#define PRINT_NEW_LINE_DEBUG(x) std::cout << (x) << std::endl
#endif

namespace
{
  bool is_leap_year(const uint16_t year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
  }
}

namespace solar_tracker
{
  uint8_t days_in_month(const uint16_t year, const uint8_t month) {
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
      return 0;
    }
  }

  void print_gps_data(const GPSData& gps_data) {

    PRINT_NEW_LINE_DEBUG("----- GPS Data -----");

    PRINT_NEW_LINE_DEBUG("Latitude: ");
    PRINT_DEBUG(gps_data.latitude);
    PRINT_DEBUG(" : ");
    PRINT_DEBUG(gps_data.lat_direction);

    PRINT_NEW_LINE_DEBUG("Longitude: ");
    PRINT_DEBUG(gps_data.longitude);
    PRINT_DEBUG(" : ");
    PRINT_DEBUG(gps_data.lon_direction);

    PRINT_NEW_LINE_DEBUG("Altitude: ");
    PRINT_DEBUG(gps_data.altitude);

    PRINT_NEW_LINE_DEBUG("Time: ");
    PRINT_DEBUG(gps_data.hour);
    PRINT_DEBUG(":");
    PRINT_DEBUG(gps_data.minute);
    PRINT_DEBUG(":");
    PRINT_DEBUG(gps_data.second);

    PRINT_NEW_LINE_DEBUG("Date: ");
    PRINT_DEBUG(gps_data.day);
    PRINT_DEBUG(":");
    PRINT_DEBUG(gps_data.month);
    PRINT_DEBUG(":");
    PRINT_DEBUG(gps_data.year);
  }

  void print_validity_of_gps_data(const GPSData& gps_data) {

    PRINT_NEW_LINE_DEBUG("----- Validity of GPS Data -----");

    PRINT_NEW_LINE_DEBUG(" Latitude: ");
    PRINT_DEBUG(gps_data.validity.latitude ? "OK" : "FAIL");

    PRINT_NEW_LINE_DEBUG(" Longitude: ");
    PRINT_DEBUG(gps_data.validity.longitude ? "OK" : "FAIL");

    PRINT_NEW_LINE_DEBUG(" Time: ");
    PRINT_DEBUG(gps_data.validity.time ? "OK" : "FAIL");

    PRINT_NEW_LINE_DEBUG(" Date: ");
    PRINT_DEBUG(gps_data.validity.date ? "OK" : "FAIL");

    PRINT_NEW_LINE_DEBUG(" Altitude: ");
    PRINT_DEBUG(gps_data.validity.altitude ? "OK" : "FAIL");
  }
}
