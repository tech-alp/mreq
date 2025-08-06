#include <optional>
#include <iostream>
#include <cassert>
#include <sensor_temperature.pb.h>
#include "mreq/mreq.hpp"
#include "topic_registry_autogen.hpp"

int main() {
    using namespace mreq;
    using namespace mreq::autogen;

    std::cout << "[EXAMPLE] Sequential Publish Test\n";

    auto temperatureToken = MREQ_SUBSCRIBE(sensor_temperature);

    for (int i = 0; i < 10; ++i) {
        SensorTemperature msg;
        msg.id = i;
        msg.temperature = i * 1.5f;
        msg.timestamp = (123456789 + i);
        MREQ_PUBLISH(sensor_temperature, msg);

        auto out = MREQ_READ(sensor_temperature, *temperatureToken);
        assert(out);
        std::cout << "[Seq] id: " << out->id << ", temperature: " << out->temperature << ", timestamp: " << out->timestamp << std::endl;
        assert(out->id == msg.id && out->temperature == msg.temperature && out->timestamp == msg.timestamp);

        assert(!MREQ_READ(sensor_temperature, *temperatureToken));
    }
    MREQ_UNSUBSCRIBE(sensor_temperature, *temperatureToken);
    std::cout << "[EXAMPLE] Sequential Publish Test completed successfully.\n";
    return 0;
}
