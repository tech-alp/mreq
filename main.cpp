#include <optional>
#include <iostream>
#include <cassert>
#include <example.pb.h>
#include <sensor.pb.h>
#include "src/mreq/interface.hpp"

// Tek topic objesiyle publish/subscribe örneği
void test_example_message() {
    std::cout << "[TEST] ExampleMessage başlıyor...\n";
    Topic<ExampleMessage> exampleTopic;

    auto exampleToken = exampleTopic.subscribe();

    ExampleMessage msg;
    msg.id = 42;
    msg.data = 3.14f;
    exampleTopic.publish(msg);

    // Modern read
    auto received = exampleTopic.read(exampleToken.value());
    assert(received);
    std::cout << "[Example] id: " << received->id << ", data: " << received->data << '\n';

    assert(!exampleTopic.read(exampleToken.value())); // Veri okundu, bir daha gelmez

    exampleTopic.unsubscribe(exampleToken.value());
    std::cout << "[TEST] ExampleMessage tamamlandı.\n";
}

// Çoklu abone örneği
void test_multi_subscriber() {
    std::cout << "[TEST] MultiSubscriber başlıyor...\n";
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
    std::cout << "[TEST] MultiSubscriber tamamlandı.\n";
}

// Farklı sensör/mesaj türleriyle test
void test_pressure_sensor() {
    std::cout << "[TEST] PressureSensor başlıyor...\n";
    Topic<PressureSensor> pressureTopic;

    auto pressureToken = pressureTopic.subscribe();

    PressureSensor pres;
    pres.id = 202;
    pres.pressure = 1013.25f;
    pres.timestamp = 987654321;
    pressureTopic.publish(pres);

    auto out = pressureTopic.read(pressureToken.value());
    assert(out);
    std::cout << "[Pressure] id: " << out->id << ", pressure: " << out->pressure << std::endl;
    assert(out->id == pres.id && out->pressure == pres.pressure && out->timestamp == pres.timestamp);

    assert(!pressureTopic.read(pressureToken.value()));

    pressureTopic.unsubscribe(pressureToken.value());
    std::cout << "[TEST] PressureSensor tamamlandı.\n";
}

void test_imu_sensor() {
    std::cout << "[TEST] ImuSensor başlıyor...\n";
    Topic<ImuSensor> imuTopic;
    auto imuToken = imuTopic.subscribe();

    ImuSensor imu;
    imu.id = 303;
    imu.accel_x = 0.1f;
    imu.accel_y = 0.2f;
    imu.accel_z = 0.3f;
    imu.gyro_x = 1.1f;
    imu.gyro_y = 1.2f;
    imu.gyro_z = 1.3f;
    imu.timestamp = 555555555;
    imuTopic.publish(imu);

    auto out = imuTopic.read(imuToken.value());
    assert(out);

    std::cout << "[IMU] id: " << out->id << ", accel: (" << out->accel_x << "," << out->accel_y << "," << out->accel_z
              << "), gyro: (" << out->gyro_x << "," << out->gyro_y << "," << out->gyro_z << "), ts: " << out->timestamp << std::endl;

    assert(out->id == imu.id && out->accel_x == imu.accel_x && out->accel_y == imu.accel_y && out->accel_z == imu.accel_z &&
           out->gyro_x == imu.gyro_x && out->gyro_y == imu.gyro_y && out->gyro_z == imu.gyro_z && out->timestamp == imu.timestamp);

    assert(!imuTopic.read(imuToken.value()));

    imuTopic.unsubscribe(imuToken.value());
    std::cout << "[TEST] ImuSensor tamamlandı.\n";
}

// Seri publish/read test (bir abone arka arkaya çok mesaj alır)
void test_sequential_publish() {
    std::cout << "[TEST] SequentialPublish başlıyor...\n";
    Topic<ExampleMessage> exampleTopic;
    auto exampleToken = exampleTopic.subscribe();

    for (int i = 0; i < 10; ++i) {
        ExampleMessage msg;
        msg.id = i;
        msg.data = i * 1.5f;
        exampleTopic.publish(msg);

        auto out = exampleTopic.read(exampleToken.value());
        assert(out);
        std::cout << "[Seq] id: " << out->id << ", data: " << out->data << std::endl;
        assert(out->id == msg.id && out->data == msg.data);

        assert(!exampleTopic.read(exampleToken.value()));
    }
    exampleTopic.unsubscribe(exampleToken.value());
    std::cout << "[TEST] SequentialPublish tamamlandı.\n";
}

int main() {
    test_example_message();
    test_multi_subscriber();
    test_pressure_sensor();
    test_imu_sensor();
    test_sequential_publish();

    std::cout << "All tests passed!" << std::endl;
    return 0;
}
