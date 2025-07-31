#include <optional>
#include <iostream>
#include <cassert>
#include <sensor_temperature.pb.h>
#include "mreq/mreq.hpp"
#include "topic_registry_autogen.hpp"

int main() {
    std::cout << "[EXAMPLE] Multi-Subscriber & Ring Buffer Test\n";
    
    using namespace mreq::autogen;
    
    mreq::init();
    mreq::autogen::register_topics();

    // Topic'i registry'den al (MREQ_ID ile otomatik üretilen registry'den)
    auto token = mreq::subscribe(MREQ_ID(sensor_temperature));
    assert(token);

    // Maksimum abone limiti (MREQ_MAX_SUBSCRIBERS) kadar abone olalım
    // Ring buffer'a 4 farklı mesaj yayınla
    for (int i = 0; i < 4; ++i) {
        SensorTemperature temp = {};
        temp.id = 100 + i;
        temp.temperature = 20.0f + i;
        temp.timestamp = 1000000 + i;
        mreq::publish(MREQ_ID(sensor_temperature), temp);
    }

    // Her abone en güncel veriyi okuyabilir
    if (mreq::check(MREQ_ID(sensor_temperature), *token)) {
        auto val = mreq::read<SensorTemperature>(MREQ_ID(sensor_temperature), *token);
        assert(val);
        std::cout << "[Multi] Abone: id=" << val->id << ", temp=" << val->temperature << std::endl;
        // Okuduktan sonra tekrar okursa veri gelmemeli
        assert(!mreq::read<SensorTemperature>(MREQ_ID(sensor_temperature), *token));
    }
    mreq::unsubscribe(MREQ_ID(sensor_temperature), *token);
    std::cout << "[EXAMPLE] Multi-Subscriber & Ring Buffer Test completed successfully.\n";
    return 0;
} 
