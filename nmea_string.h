#ifndef NMEA_STRING_H
#define NMEA_STRING_H

namespace solar_tracker
{

#ifdef ARDUINO
#include <Arduino.h>
 using nmea_string_type = String;
#else

#include <string>

 template<typename string_type>
 class nmea_string
 {
 public:
  int indexOf(char, int b = 0) const;
  int indexOf(nmea_string, int b  =0) const;
  unsigned int length() const;
  nmea_string substring(int , int to =0) const;

  char charAt(int) const;

  int toInt() const;
  float toFloat() const;
  double toDouble() const;

  void trim();

  bool operator==(const string_type&) const;
 };

 using nmea_string_type = nmea_string<std::string_view>;
#endif
}

#endif