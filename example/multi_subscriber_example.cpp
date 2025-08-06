#include <iostream>
#include <thread>
#include <vector>
#include <cassert>
#include "mreq/mreq.hpp"
#include "sensor_temperature.pb.h"
#include "topic_registry_autogen.hpp"

int main() {
    using namespace mreq::autogen;

    std::cout << "[EXAMPLE] Multi-Subscriber Example\n";

    auto token = MREQ_SUBSCRIBE(sensor_temperature);
    if (!token) {
        std::cerr << "Failed to subscribe to topic." << std::endl;
        return 1;
    }

    std::thread publisher_thread([]() {
        for (int i = 0; i < 5; ++i) {
            SensorTemperature temp = {i, 25.0f + i, 0};
            MREQ_PUBLISH(sensor_temperature, temp);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    });

    std::thread subscriber_thread([&]() {
        for (int i = 0; i < 5; ++i) {
            if (MREQ_CHECK(sensor_temperature, *token)) {
                auto val = MREQ_READ(sensor_temperature, *token);
                assert(val);
                std::cout << "Subscriber received: " << val->temperature << std::endl;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(150));
        }
        MREQ_UNSUBSCRIBE(sensor_temperature, *token);
    });

    publisher_thread.join();
    subscriber_thread.join();

    std::cout << "[EXAMPLE] Multi-Subscriber Example completed successfully.\n";

    return 0;
}
