#ifndef SUN_POSITION_H
#define SUN_POSITION_H

#include "gps_data.h"

namespace solar_tracker
{
 void calculate_sun_position(const GPSData& gps_data, double& azimuth, double& elevation);
}

#endif
