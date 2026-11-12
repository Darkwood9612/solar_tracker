#ifndef GPS_DATA_H
#define GPS_DATA_H

#include <cstdint>

namespace solar_tracker
{
    struct GPSValidity {
        bool latitude = false;
        bool longitude = false;
        bool date = false;
        bool time = false;
        bool altitude = false;
    };

    struct GPSData {
        double latitude = 0.0;
        double longitude = 0.0;

        float altitude = 0.0f;

        uint16_t year = 0;
        uint8_t month = 0;
        uint8_t day = 0;

        uint8_t hour = 0;
        uint8_t minute = 0;
        uint8_t second = 0;

        char lat_direction = '\0';
        char lon_direction = '\0';

        GPSValidity validity;
    };

    uint8_t days_in_month(uint16_t year, uint8_t month);

    inline bool is_date_valid(const GPSData& gps_data) {
        const uint8_t num_days_in_month = days_in_month(gps_data.year, gps_data.month);
        return gps_data.year >= 2025 && gps_data.month != 0 && gps_data.month <= 12 && num_days_in_month > 0 && gps_data.day <= num_days_in_month;
    }

    inline bool is_time_valid(const GPSData& gps_data) {
        return gps_data.hour < 24 && gps_data.minute < 60 && gps_data.second < 60;
    }

    inline bool is_gps_data_valid(const GPSData& gps_data) {
        const bool is_all_validity_flags_true = gps_data.validity.latitude && gps_data.validity.longitude && gps_data.validity.time && gps_data.validity.date && gps_data.validity.altitude;
        return is_all_validity_flags_true && is_date_valid(gps_data) && is_time_valid(gps_data);
    }

    void print_gps_data(const GPSData& gps_data);
    void print_validity_of_gps_data(const GPSData& gps_data);
}

#endif
