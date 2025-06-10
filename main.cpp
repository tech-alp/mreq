#include <iostream>
#include <atomic>
#include <cstring>
#include <example.pb.h>
#include <sensor.pb.h>
#include "src/mreq/interface.hpp"
#include <cassert>

// ExampleMessage için fonksiyonların implementasyonu
static ExampleMessage example_buffer;
static std::atomic<bool> example_updated = false;

template<>
void publish<ExampleMessage>(const ExampleMessage& msg) {
    std::memcpy(&example_buffer, &msg, sizeof(ExampleMessage));
    example_updated.store(true, std::memory_order_release);
}

template<>
void subscribe<ExampleMessage>(size_t subscriber_idx) {
    // Gerçek sistemde abone kaydı yapılır, burada gerek yok
}

template<>
bool check<ExampleMessage>(size_t subscriber_idx) {
    return example_updated.load(std::memory_order_acquire);
}

template<>
void copy<ExampleMessage>(size_t subscriber_idx, ExampleMessage& out) {
    std::memcpy(&out, &example_buffer, sizeof(ExampleMessage));
    example_updated.store(false, std::memory_order_release);
}

void test_publish_and_check() {
    ExampleMessage msg;
    msg.id = 1;
    msg.data = 1.23f;
    size_t sub = 0;
    subscribe<ExampleMessage>(sub);
    publish(msg);
    bool updated = check<ExampleMessage>(sub);
    std::cout << "[TEST] publish/check: " << (updated ? "OK" : "FAIL") << std::endl;
    assert(updated);
}

void test_copy() {
    ExampleMessage msg;
    msg.id = 7;
    msg.data = 7.77f;
    size_t sub = 0;
    subscribe<ExampleMessage>(sub);
    publish(msg);
    ExampleMessage out;
    copy<ExampleMessage>(sub, out);
    std::cout << "[TEST] copy: id=" << out.id << ", data=" << out.data << std::endl;
    assert(out.id == msg.id && out.data == msg.data);
    // Sonra tekrar check false olmalı
    bool updated = check<ExampleMessage>(sub);
    std::cout << "[TEST] check after copy: " << (!updated ? "OK" : "FAIL") << std::endl;
    assert(!updated);
}

void test_temperature_sensor() {
    TemperatureSensor temp;
    temp.id = 101;
    temp.temperature = 36.5f;
    temp.timestamp = 123456789;
    size_t sub = 1;
    subscribe<TemperatureSensor>(sub);
    publish(temp);
    assert(check<TemperatureSensor>(sub));
    TemperatureSensor out;
    copy<TemperatureSensor>(sub, out);
    std::cout << "[TEMP] id=" << out.id << ", temp=" << out.temperature << ", ts=" << out.timestamp << std::endl;
    assert(out.id == temp.id && out.temperature == temp.temperature && out.timestamp == temp.timestamp);
}

void test_pressure_sensor() {
    PressureSensor pres;
    pres.id = 202;
    pres.pressure = 1013.25f;
    pres.timestamp = 987654321;
    size_t sub = 2;
    subscribe<PressureSensor>(sub);
    publish(pres);
    assert(check<PressureSensor>(sub));
    PressureSensor out;
    copy<PressureSensor>(sub, out);
    std::cout << "[PRESS] id=" << out.id << ", pressure=" << out.pressure << ", ts=" << out.timestamp << std::endl;
    assert(out.id == pres.id && out.pressure == pres.pressure && out.timestamp == pres.timestamp);
}

void test_imu_sensor() {
    ImuSensor imu;
    imu.id = 303;
    imu.accel_x = 0.1f;
    imu.accel_y = 0.2f;
    imu.accel_z = 0.3f;
    imu.gyro_x = 1.1f;
    imu.gyro_y = 1.2f;
    imu.gyro_z = 1.3f;
    imu.timestamp = 555555555;
    size_t sub = 3;
    subscribe<ImuSensor>(sub);
    publish(imu);
    assert(check<ImuSensor>(sub));
    ImuSensor out;
    copy<ImuSensor>(sub, out);
    std::cout << "[IMU] id=" << out.id << ", ax=" << out.accel_x << ", ay=" << out.accel_y << ", az=" << out.accel_z
              << ", gx=" << out.gyro_x << ", gy=" << out.gyro_y << ", gz=" << out.gyro_z << ", ts=" << out.timestamp << std::endl;
    assert(out.id == imu.id && out.accel_x == imu.accel_x && out.accel_y == imu.accel_y && out.accel_z == imu.accel_z &&
           out.gyro_x == imu.gyro_x && out.gyro_y == imu.gyro_y && out.gyro_z == imu.gyro_z && out.timestamp == imu.timestamp);
}

// TemperatureSensor için abone rolleri
enum class TempSubscribers : size_t {
    Logger = 0,
    Control = 1,
    UI = 2
};

void multi_subscriber_example() {
    // İki farklı abone (subscriber)
    size_t sub1 = static_cast<size_t>(TempSubscribers::Logger);
    size_t sub2 = static_cast<size_t>(TempSubscribers::Control);

    // Her abone subscribe oluyor (flag'leri temizleniyor)
    subscribe<TemperatureSensor>(sub1);
    subscribe<TemperatureSensor>(sub2);

    // Bir mesaj yayınlanıyor
    TemperatureSensor temp;
    temp.id = 42;
    temp.temperature = 25.5f;
    temp.timestamp = 123456789;
    publish(temp);

    // Her iki abone de güncellemeyi görebilir
    if (check<TemperatureSensor>(sub1)) {
        TemperatureSensor out1;
        copy<TemperatureSensor>(sub1, out1);
        std::cout << "[LOGGER] id=" << out1.id << ", temp=" << out1.temperature << std::endl;
    }

    if (check<TemperatureSensor>(sub2)) {
        TemperatureSensor out2;
        copy<TemperatureSensor>(sub2, out2);
        std::cout << "[CONTROL] id=" << out2.id << ", temp=" << out2.temperature << std::endl;
    }

    // Artık her iki abone de mesajı aldığı için tekrar check false döner
    std::cout << std::boolalpha <<  "Tekrar kontrol: Logger=" << check<TemperatureSensor>(sub1)
              << ", Control=" << check<TemperatureSensor>(sub2) << std::endl;
}

int main() {
    multi_subscriber_example();
    test_publish_and_check();
    test_copy();
    test_temperature_sensor();
    test_pressure_sensor();
    test_imu_sensor();
    std::cout << "TemperatureSensor Logger: " << std::boolalpha <<  check<TemperatureSensor>(static_cast<size_t>(TempSubscribers::Logger)) << '\n';
    std::cout << "TemperatureSensor Control: " << std::boolalpha <<  check<TemperatureSensor>(static_cast<size_t>(TempSubscribers::Control)) << '\n';


    ExampleMessage msg;
    msg.id = 42;
    msg.data = 3.14f;
    size_t subscriber_idx = 0;
    subscribe<ExampleMessage>(subscriber_idx);
    publish(msg);
    if (check<ExampleMessage>(subscriber_idx)) {
        ExampleMessage received;
        copy<ExampleMessage>(subscriber_idx, received);
        std::cout << "Alınan id: " << received.id << ", " << received.data << std::endl;
    } else {
        std::cout << "Yeni mesaj yok." << std::endl;
    }
    return 0;
}
