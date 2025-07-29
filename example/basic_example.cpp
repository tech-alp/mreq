#include <iostream>
#include <cassert>
#include "mreq/mreq.hpp"
#include "sensor_temperature.pb.h"

int main() {
    std::cout << "[EXAMPLE] Basic Example TemperatureSensor Test\n";
    
    // Initialize the mreq library and register all topics
    mreq::init();

    // Subscribe to the temperature topic using the new simplified API
    auto token = mreq::subscribe(MREQ_ID(sensor_temperature));

    if (!token) {
        std::cerr << "Failed to subscribe to topic." << std::endl;
        return 1;
    }

    // Publish a message
    SensorTemperature msg = {42, 36.5f, 123456789};
    mreq::publish(MREQ_ID(sensor_temperature), msg);

    // Check for a new message
    if (mreq::check(MREQ_ID(sensor_temperature), *token)) {
        // Read the message
        auto received = mreq::read<SensorTemperature>(MREQ_ID(sensor_temperature), *token);
        assert(received);
        std::cout << "[Example] id: " << received->id << ", temperature: " << received->temperature << ", timestamp: " << received->timestamp << '\n';
    }

    assert(!mreq::read<SensorTemperature>(MREQ_ID(sensor_temperature), *token)); // Veri okundu, bir daha gelmez

    mreq::unsubscribe(MREQ_ID(sensor_temperature), *token);
    std::cout << "[EXAMPLE] Basic Example TemperatureSensor Test completed successfully.\n";
    return 0;
}
