#include <iostream>
#include <cassert>
#include "mreq/mreq.hpp"
#include "sensor_baro.pb.h"
#include "topic_registry_autogen.hpp"

int main() {
    std::cout << "[EXAMPLE] Multi Topic Example SensorBaro Test\n";
    
    using namespace mreq::autogen;

    // Kütüphaneyi başlat ve topicleri kaydet
    mreq::init();
    mreq::autogen::register_topics();

    // "sensor_baro" topic'ine abone ol
    auto token_raw = mreq::subscribe(MREQ_ID(sensor_baro));
    if (!token_raw) {
        std::cerr << "Failed to subscribe to sensor_baro." << std::endl;
        return 1;
    }

    // "sensor_baro_filtered" topic'ine abone ol
    auto token_filtered = mreq::subscribe(MREQ_ID(sensor_baro_filtered));
    if (!token_filtered) {
        std::cerr << "Failed to subscribe to sensor_baro_filtered." << std::endl;
        return 1;
    }

    // "sensor_baro" topic'ine mesaj yayınla
    SensorBaro msg_raw = {};
    msg_raw.pressure = 1013.25f;
    msg_raw.timestamp = 123456789;
    mreq::publish(MREQ_ID(sensor_baro), msg_raw);

    // "sensor_baro_filtered" topic'ine mesaj yayınla
    SensorBaro msg_filtered = {};
    msg_filtered.pressure = 1012.80f;
    msg_filtered.timestamp = 123456790;
    mreq::publish(MREQ_ID(sensor_baro_filtered), msg_filtered);

    // "sensor_baro" mesajını oku
    if (mreq::check(MREQ_ID(sensor_baro), *token_raw)) {
        auto received = mreq::read<SensorBaro>(MREQ_ID(sensor_baro), *token_raw);
        assert(received);
        std::cout << "[Example] RAW: pressure: " << received->pressure << ", timestamp: " << received->timestamp << '\n';
    }

    // "sensor_baro_filtered" mesajını oku
    if (mreq::check(MREQ_ID(sensor_baro_filtered), *token_filtered)) {
        auto received = mreq::read<SensorBaro>(MREQ_ID(sensor_baro_filtered), *token_filtered);
        assert(received);
        std::cout << "[Example] FILTERED: pressure: " << received->pressure << ", timestamp: " << received->timestamp << '\n';
    }

    mreq::unsubscribe(MREQ_ID(sensor_baro), *token_raw);
    mreq::unsubscribe(MREQ_ID(sensor_baro_filtered), *token_filtered);

    std::cout << "[EXAMPLE] Multi Topic Example SensorBaro Test completed successfully.\n";
    return 0;
} 