#include <iostream>
#include <cassert>
#include "mreq/mreq.hpp"
#include "sensor_temperature.pb.h"
#include "topic_registry_autogen.hpp"

int main() {
    std::cout << "[EXAMPLE] Basic Example TemperatureSensor Test\n";
    
    using namespace mreq::autogen;

    // Initialize the mreq library and register all topics
    mreq::init();
    mreq::autogen::register_topics();

    // Subscribe to the temperature topic using the new simplified API
    auto token = mreq::subscribe(MREQ_ID(SensorTemperature));

    if (!token) {
        std::cerr << "Failed to subscribe to topic." << std::endl;
        return 1;
    }

    // Publish a message
    SensorTemperature msg = {42, 36.5f, 123456789};
    mreq::publish(MREQ_ID(SensorTemperature), msg);

    // Check for a new message
    if (mreq::check(MREQ_ID(SensorTemperature), *token)) {
        // Read the message
        auto received = mreq::read<SensorTemperature>(MREQ_ID(SensorTemperature), *token);
        assert(received);
        std::cout << "[Example] id: " << received->id << ", temperature: " << received->temperature << ", timestamp: " << received->timestamp << '\n';
    }

    assert(!mreq::read<SensorTemperature>(MREQ_ID(SensorTemperature), *token)); // Veri okundu, bir daha gelmez

    mreq::unsubscribe(MREQ_ID(SensorTemperature), *token);
    std::cout << "[EXAMPLE] Basic Example TemperatureSensor Test completed successfully.\n";
    return 0;
}
