#include "sun_position.h"

#include <cmath>

namespace
{
  constexpr double pi = 3.141592653589793238462643383279502884;
  constexpr double degrees_to_radians = pi / 180.0;
  constexpr double radians_to_degrees = 180.0 / pi;

  double normalize_angle(double angle)
  {
    angle = fmod(angle, 360.0);

    if (angle < 0.0)
      angle += 360.0;

    return angle;
  }

  double calculate_julian_date(const solar_tracker::GPSData& gps_data) {

    int year = gps_data.year;
    int month = gps_data.month;
    const int day = gps_data.day;

    if (month <= 2) {
      year--;
      month += 12;
    }

    // Gregorian calendar correction
    const int century = year / 100;
    const int gregorian_correction = 2 - century + century / 4;

    const double julian_date = floor(365.25 * (year + 4716)) + floor(30.6001 * (month + 1)) + day + gregorian_correction - 1524.5;

    constexpr double seconds_in_hour = 3600.0;
    constexpr double minutes_in_hour = 60.0;
    const double time_fraction = (gps_data.hour + (gps_data.minute / minutes_in_hour) + (gps_data.second / seconds_in_hour)) / 24.0;

    return julian_date + time_fraction;
  }

  double calculate_right_ascension_in_radians(const double ecliptic_longitude_in_radians, const double ecliptic_obliquity_in_radians)
  {
    return atan2(cos(ecliptic_obliquity_in_radians) * sin(ecliptic_longitude_in_radians), cos(ecliptic_longitude_in_radians));
  }

  double calculate_hour_angle_in_radians(const solar_tracker::GPSData& gps_data, const double days_since_j2000, const double right_ascension_in_radians)
  {
    const double greenwich_sidereal_time = normalize_angle(280.46061837 + 360.98564736629 * days_since_j2000);

    const double local_sidereal_time = normalize_angle(greenwich_sidereal_time + gps_data.longitude);

    const double right_ascension = normalize_angle(right_ascension_in_radians * radians_to_degrees);

    const double hour_angle = normalize_angle(local_sidereal_time - right_ascension);

    return hour_angle * degrees_to_radians;
  }

} // anonymous namespace

namespace solar_tracker
{
  void calculate_sun_position(const GPSData& gps_data, double& out_azimuth, double& out_elevation)
  {
    const double julian_date = calculate_julian_date(gps_data);

    constexpr double julian_date_offset = 2451545.0;
    const double days_since_j2000 = julian_date - julian_date_offset;

    const double mean_anomaly_in_radians = fmod(357.529 + 0.98560028 * days_since_j2000, 360.0) * degrees_to_radians;
    const double sun_mean_longitude = fmod(280.459 + 0.98564736 * days_since_j2000, 360.0);

    const double equation_of_center = 1.915 * sin(mean_anomaly_in_radians) + 0.020 * sin(2 * mean_anomaly_in_radians);
    const double ecliptic_longitude = fmod(sun_mean_longitude + equation_of_center, 360.0);
    const double ecliptic_longitude_in_radians = ecliptic_longitude * degrees_to_radians;

    const double ecliptic_obliquity_in_radians = (23.439 - 0.00000036 * days_since_j2000) * degrees_to_radians;

    const double declination_in_radians = asin(sin(ecliptic_obliquity_in_radians) * sin(ecliptic_longitude_in_radians));

    const double right_ascension_in_radians = calculate_right_ascension_in_radians(ecliptic_longitude_in_radians, ecliptic_obliquity_in_radians);
    const double hour_angle_in_radians = calculate_hour_angle_in_radians(gps_data, days_since_j2000, right_ascension_in_radians);

    const double latitude_in_radians = gps_data.latitude * degrees_to_radians;

    const double azimuth_in_radians = atan2(sin(hour_angle_in_radians), cos(hour_angle_in_radians) * sin(latitude_in_radians) - tan(declination_in_radians) * cos(latitude_in_radians));

    out_azimuth = normalize_angle(azimuth_in_radians * radians_to_degrees + 180.0);

    const double elevation_in_radians = asin(sin(latitude_in_radians) * sin(declination_in_radians) + cos(latitude_in_radians) * cos(declination_in_radians) * cos(hour_angle_in_radians));

    out_elevation = elevation_in_radians * radians_to_degrees;
  }
}
