#include <optional>
#include <iostream>
#include <cassert>
#include <sensor_temperature.pb.h>
#include "mreq/interface.hpp"

int main() {
    std::cout << "[EXAMPLE] Sequential Publish Test\n";
    Topic<SensorTemperature> temperatureTopic;
    auto temperatureToken = temperatureTopic.subscribe();

    for (int i = 0; i < 10; ++i) {
        SensorTemperature msg;
        msg.id = i;
        msg.temperature = i * 1.5f;
        msg.timestamp = 123456789 + i;
        temperatureTopic.publish(msg);

        auto out = temperatureTopic.read(temperatureToken.value());
        assert(out);
        std::cout << "[Seq] id: " << out->id << ", temperature: " << out->temperature << ", timestamp: " << out->timestamp << std::endl;
        assert(out->id == msg.id && out->temperature == msg.temperature && out->timestamp == msg.timestamp);

        assert(!temperatureTopic.read(temperatureToken.value()));
    }
    temperatureTopic.unsubscribe(temperatureToken.value());
    std::cout << "[EXAMPLE] Sequential Publish Test completed successfully.\n";
    return 0;
} 
