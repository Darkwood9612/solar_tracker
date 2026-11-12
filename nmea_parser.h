#ifndef GPS_PARSER_H
#define GPS_PARSER_H

#include "gps_data.h"
#include "nmea_string.h"

namespace solar_tracker
{
 void parse_nmea(const nmea_string_type& nmea, GPSData& gps_data);
}

#endif
