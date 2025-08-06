#include <iostream>
#include <cassert>
#include "mreq/mreq.hpp"
#include "sensor_temperature.pb.h"
#include "topic_registry_autogen.hpp"

int main() {
    std::cout << "[EXAMPLE] Basic Example TemperatureSensor Test\n";
    
    using namespace mreq::autogen;

    // Subscribe to the temperature topic using the new simplified API
    auto token = MREQ_SUBSCRIBE(sensor_temperature);

    if (!token) {
        std::cerr << "Failed to subscribe to topic." << std::endl;
        return 1;
    }

    // Publish a message
    SensorTemperature msg = {42, 36.5f, 123456789};
    MREQ_PUBLISH(sensor_temperature, msg);

    // Check for a new message
    if (MREQ_CHECK(sensor_temperature, *token)) {
        // Read the message
        auto received = MREQ_READ(sensor_temperature, *token);
        assert(received);
        std::cout << "[Example] id: " << received->id << ", temperature: " << received->temperature << ", timestamp: " << received->timestamp << '\n';
    }

    assert(!MREQ_READ(sensor_temperature, *token)); // Veri okundu, bir daha gelmez

    MREQ_UNSUBSCRIBE(sensor_temperature, *token);
    
    std::cout << "[EXAMPLE] Basic Example TemperatureSensor Test completed successfully.\n";
    return 0;
}
