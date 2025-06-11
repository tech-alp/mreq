#include <optional>
#include <iostream>
#include <cassert>
#include <sensor.pb.h>
#include "mreq/interface.hpp"

void test_pressure_sensor() {
    std::cout << "[EXAMPLE] Pressure Sensor Test\n";
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
    std::cout << "[EXAMPLE] Pressure Sensor Test completed successfully.\n";
}

void test_imu_sensor() {
    std::cout << "[EXAMPLE] IMU Sensor Test\n";
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
    std::cout << "[EXAMPLE] IMU Sensor Test completed successfully.\n";
}

int main() {
    test_pressure_sensor();
    test_imu_sensor();
    return 0;
} 