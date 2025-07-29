#include <optional>
#include <iostream>
#include <cassert>
#include <sensor_temperature.pb.h>

int main() {
    std::cout << "[EXAMPLE] Sequential Publish Test\n";
    
    // Topic'i registry'den al (MREQ_ID ile otomatik üretilen registry'den)
    auto& temperatureTopic = mreq::get_topic<SensorTemperature>(MREQ_ID(SensorTemperature)).value().get();
    
    auto temperatureToken = temperatureTopic.subscribe();

    for (int i = 0; i < 10; ++i) {
        SensorTemperature msg;
        msg.set_id(i);
        msg.set_temperature(i * 1.5f);
        msg.set_timestamp(123456789 + i);
        temperatureTopic.publish(msg);

        auto out = temperatureTopic.read(temperatureToken.value());
        assert(out);
        std::cout << "[Seq] id: " << out->id() << ", temperature: " << out->temperature() << ", timestamp: " << out->timestamp() << std::endl;
        assert(out->id() == msg.id() && out->temperature() == msg.temperature() && out->timestamp() == msg.timestamp());

        assert(!temperatureTopic.read(temperatureToken.value()));
    }
    temperatureTopic.unsubscribe(temperatureToken.value());
    std::cout << "[EXAMPLE] Sequential Publish Test completed successfully.\n";
    return 0;
} 
