#include <optional>
#include <iostream>
#include <cassert>
#include <sensor_temperature.pb.h>
#include "mreq/interface.hpp"

int main() {
    std::cout << "[EXAMPLE] Multi-Subscriber & Ring Buffer Test\n";
    // 4 elemanlı ring buffer ile topic
    Topic<SensorTemperature, 4> temperatureTopic;

    // Maksimum abone limiti (MREQ_MAX_SUBSCRIBERS) kadar abone olalım
    std::array<std::optional<size_t>, MREQ_MAX_SUBSCRIBERS> tokens;
    for (size_t i = 0; i < MREQ_MAX_SUBSCRIBERS; ++i) {
        tokens[i] = temperatureTopic.subscribe();
        assert(tokens[i].has_value());
    }
    // Limiti aşan abone subscribe edemez
    auto overLimit = temperatureTopic.subscribe();
    assert(!overLimit.has_value());
    std::cout << "[Multi] Abone limiti testi başarılı.\n";

    // Ring buffer'a 4 farklı mesaj yayınla
    for (int i = 0; i < 4; ++i) {
        SensorTemperature temp;
        temp.id = 100 + i;
        temp.temperature = 20.0f + i;
        temp.timestamp = 1000000 + i;
        temperatureTopic.publish(temp);
    }

    // Her abone en güncel veriyi okuyabilir
    for (size_t i = 0; i < MREQ_MAX_SUBSCRIBERS; ++i) {
        auto val = temperatureTopic.read(tokens[i].value());
        assert(val);
        std::cout << "[Multi] Abone " << i << ": id=" << val->id << ", temp=" << val->temperature << std::endl;
        // Okuduktan sonra tekrar okursa veri gelmemeli
        assert(!temperatureTopic.read(tokens[i].value()));
        temperatureTopic.unsubscribe(tokens[i].value());
    }
    std::cout << "[EXAMPLE] Multi-Subscriber & Ring Buffer Test completed successfully.\n";
    return 0;
} 
