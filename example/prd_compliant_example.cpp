#include "mreq/interface.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <sensor_accel.pb.h>
#include <sensor_baro.pb.h>
#include <sensor_temperature.pb.h>

// Logging'i etkinleştir
#define MREQ_ENABLE_LOGGING

int main() {
    std::cout << "=== PRD Uyumlu MREQ Polling Örneği ===" << std::endl;
    
    // Topic'leri registry'den al (MREQ_ID ile otomatik üretilen registry'den)
    auto& accel_topic = mreq::get_topic<SensorAccel>(MREQ_ID(SensorAccel)).value().get();
    auto& baro_topic = mreq::get_topic<SensorBaro>(MREQ_ID(SensorBaro)).value().get();
    auto& temp_topic = mreq::get_topic<SensorTemperature>(MREQ_ID(SensorTemperature)).value().get();
    
    std::cout << "Topic'ler registry'den alındı." << std::endl;
    
    // Abone ol (polling için token al)
    auto accel_token = accel_topic.subscribe();
    auto baro_token = baro_topic.subscribe();
    auto temp_token = temp_topic.subscribe();
    
    if (!accel_token || !baro_token || !temp_token) {
        std::cout << "HATA: Abone olamadı!" << std::endl;
        return 1;
    }
    
    std::cout << "Abone token'ları alındı:" << std::endl;
    std::cout << "  SensorAccel: " << accel_token.value() << std::endl;
    std::cout << "  SensorBaro: " << baro_token.value() << std::endl;
    std::cout << "  SensorTemperature: " << temp_token.value() << std::endl;
    
    // Mesajları yayınla
    std::cout << "\n=== Mesaj Yayınlama ===" << std::endl;
    
    SensorAccel accel_msg;
    accel_msg.set_timestamp(1234567890);
    accel_msg.set_device_id(1);
    accel_msg.set_x(1.5f);
    accel_msg.set_y(-0.8f);
    accel_msg.set_z(9.81f);
    accel_msg.set_temperature(25.0f);
    accel_topic.publish(accel_msg);
    std::cout << "SensorAccel mesajı yayınlandı." << std::endl;
    
    SensorBaro baro_msg;
    baro_msg.set_timestamp(1234567891);
    baro_msg.set_device_id(2);
    baro_msg.set_pressure(1013.25f);
    baro_msg.set_temperature(22.5f);
    baro_msg.set_altitude(100.5f);
    baro_topic.publish(baro_msg);
    std::cout << "SensorBaro mesajı yayınlandı." << std::endl;
    
    SensorTemperature temp_msg;
    temp_msg.set_id(3);
    temp_msg.set_temperature(36.5f);
    temp_msg.set_timestamp(1234567892);
    temp_topic.publish(temp_msg);
    std::cout << "SensorTemperature mesajı yayınlandı." << std::endl;
    
    // Polling ile mesajları oku
    std::cout << "\n=== Polling ile Mesaj Okuma ===" << std::endl;
    
    // SensorAccel polling
    if (accel_topic.check(accel_token.value())) {
        auto received_accel = accel_topic.read(accel_token.value());
        if (received_accel) {
            std::cout << "SensorAccel alındı: x=" << received_accel->x() 
                      << ", y=" << received_accel->y() 
                      << ", z=" << received_accel->z() 
                      << ", timestamp=" << received_accel->timestamp() << std::endl;
        }
    }
    
    // SensorBaro polling
    if (baro_topic.check(baro_token.value())) {
        auto received_baro = baro_topic.read(baro_token.value());
        if (received_baro) {
            std::cout << "SensorBaro alındı: pressure=" << received_baro->pressure() 
                      << " hPa, temperature=" << received_baro->temperature() 
                      << "°C, altitude=" << received_baro->altitude() 
                      << "m, timestamp=" << received_baro->timestamp() << std::endl;
        }
    }
    
    // SensorTemperature polling
    if (temp_topic.check(temp_token.value())) {
        auto received_temp = temp_topic.read(temp_token.value());
        if (received_temp) {
            std::cout << "SensorTemperature alındı: id=" << received_temp->id() 
                      << ", temperature=" << received_temp->temperature() 
                      << "°C, timestamp=" << received_temp->timestamp() << std::endl;
        }
    }
    
    // İkinci kez polling yap (mesajlar zaten okunduğu için false dönmeli)
    std::cout << "\n=== İkinci Polling (Mesajlar Okundu) ===" << std::endl;
    std::cout << "SensorAccel check: " << (accel_topic.check(accel_token.value()) ? "true" : "false") << std::endl;
    std::cout << "SensorBaro check: " << (baro_topic.check(baro_token.value()) ? "true" : "false") << std::endl;
    std::cout << "SensorTemperature check: " << (temp_topic.check(temp_token.value()) ? "true" : "false") << std::endl;
    
    // Yeni mesaj yayınla ve tekrar polling yap
    std::cout << "\n=== Yeni Mesaj Yayınlama ve Polling ===" << std::endl;
    
    SensorAccel accel_msg2;
    accel_msg2.set_timestamp(1234567893);
    accel_msg2.set_device_id(1);
    accel_msg2.set_x(2.0f);
    accel_msg2.set_y(-1.2f);
    accel_msg2.set_z(9.75f);
    accel_msg2.set_temperature(26.0f);
    accel_topic.publish(accel_msg2);
    std::cout << "İkinci SensorAccel mesajı yayınlandı." << std::endl;
    
    // Kısa bir bekleme
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    
    // Tekrar polling
    if (accel_topic.check(accel_token.value())) {
        auto received_accel2 = accel_topic.read(accel_token.value());
        if (received_accel2) {
            std::cout << "İkinci SensorAccel alındı: x=" << received_accel2->x() 
                      << ", y=" << received_accel2->y() 
                      << ", z=" << received_accel2->z() 
                      << ", timestamp=" << received_accel2->timestamp() << std::endl;
        }
    }
    
    // Abonelikleri iptal et
    std::cout << "\n=== Abonelik İptali ===" << std::endl;
    accel_topic.unsubscribe(accel_token.value());
    baro_topic.unsubscribe(baro_token.value());
    temp_topic.unsubscribe(temp_token.value());
    std::cout << "Tüm abonelikler iptal edildi." << std::endl;
    
    // İstatistikler
    std::cout << "\n=== İstatistikler ===" << std::endl;
    std::cout << "SensorAccel abone sayısı: " << accel_topic.subscriber_count() << std::endl;
    std::cout << "SensorBaro abone sayısı: " << baro_topic.subscriber_count() << std::endl;
    std::cout << "SensorTemperature abone sayısı: " << temp_topic.subscriber_count() << std::endl;
    std::cout << "Registry'deki topic sayısı: " << mreq::TopicRegistryMetadata::instance().topic_count() << std::endl;
    
    std::cout << "\n=== Örnek tamamlandı ===" << std::endl;
    
    return 0;
} 