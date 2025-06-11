#include <optional>
#include <iostream>
#include <cassert>
#include <sensor.pb.h>
#include "mreq/interface.hpp"

int main() {
    std::cout << "[EXAMPLE] Basic Example TemperatureSensor Test\n";
    
    // Get topic from registry
    auto& tempTopic = topic_registry.get_topic<TemperatureSensor>();

    auto tempToken = tempTopic.subscribe();

    TemperatureSensor msg;
    msg.id = 42;
    msg.temperature = 36.5f;
    msg.timestamp = 123456789;
    tempTopic.publish(msg);

    // Modern read
    auto received = tempTopic.read(tempToken.value());
    assert(received);
    std::cout << "[Example] id: " << received->id << ", temperature: " << received->temperature << ", timestamp: " << received->timestamp << '\n';

    assert(!tempTopic.read(tempToken.value())); // Veri okundu, bir daha gelmez

    tempTopic.unsubscribe(tempToken.value());
    std::cout << "[EXAMPLE] Basic Example TemperatureSensor Test completed successfully.\n";
    return 0;
} 