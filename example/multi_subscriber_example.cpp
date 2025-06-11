#include <optional>
#include <iostream>
#include <cassert>
#include <sensor.pb.h>
#include "mreq/interface.hpp"

int main() {
    std::cout << "[EXAMPLE] Multi-Subscriber Test\n";
    Topic<TemperatureSensor> temperatureTopic;

    auto loggerToken = temperatureTopic.subscribe();
    auto uiToken = temperatureTopic.subscribe();

    TemperatureSensor temp;
    temp.id = 101;
    temp.temperature = 25.4f;
    temp.timestamp = 1234567;
    temperatureTopic.publish(temp);

    // Modern read
    auto loggerVal = temperatureTopic.read(loggerToken.value());
    auto uiVal = temperatureTopic.read(uiToken.value());

    assert(loggerVal && uiVal);
    std::cout << "[Multi] logger: " << loggerVal->temperature << ", ui: " << uiVal->temperature << std::endl;

    assert(loggerVal->id == temp.id && loggerVal->temperature == temp.temperature && loggerVal->timestamp == temp.timestamp);
    assert(uiVal->id == temp.id && uiVal->temperature == temp.temperature && uiVal->timestamp == temp.timestamp);

    assert(!temperatureTopic.read(loggerToken.value()));
    assert(!temperatureTopic.read(uiToken.value()));

    temperatureTopic.unsubscribe(loggerToken.value());
    temperatureTopic.unsubscribe(uiToken.value());
    std::cout << "[EXAMPLE] Multi-Subscriber Test completed successfully.\n";
    return 0;
} 