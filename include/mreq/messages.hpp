#pragma once
#include "metadata.hpp"

namespace mreq {

// Örnek mesaj yapıları
struct SensorAccel {
    float x;
    float y;
    float z;
    uint64_t timestamp;
};

struct GpsPosition {
    double latitude;
    double longitude;
    float altitude;
    uint64_t timestamp;
};

// Metadata tanımları - derleme zamanında oluşturulur
MREQ_MESSAGE_TYPE(SensorAccel, "sensor_accel");
MREQ_MESSAGE_TYPE(GpsPosition, "gps_position");

} // namespace mreq 