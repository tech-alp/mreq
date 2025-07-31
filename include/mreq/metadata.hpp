#pragma once
#include <cstddef>
#include <cstdint>
#include <functional>

#include "pb.h"

namespace mreq {

// Forward declaration
struct mreq_metadata;

// Genel nanopb encode/decode fonksiyonları
bool nanopb_encode_wrapper(const mreq_metadata& metadata, const void* data, void* buffer, size_t buffer_size, size_t* message_length);
bool nanopb_decode_wrapper(const mreq_metadata& metadata, const void* buffer, size_t buffer_size, void* data);


// nanopb uyumlu mesaj metadata yapısı
struct mreq_metadata {
    const char* topic_name;                    // Topic adı (örn: "sensor_accel")
    size_t payload_size;                       // Mesaj payload boyutu (nanopb struct size)
    size_t message_id;                         // Benzersiz mesaj ID'si (derleme zamanında hesaplanır)
    const pb_msgdesc_t* fields;                // nanopb mesaj tanımlayıcısı

    // nanopb serialization/deserialization fonksiyonları
    bool encode(const void* data, void* buffer, size_t buffer_size, size_t* message_length) const {
        return nanopb_encode_wrapper(*this, data, buffer, buffer_size, message_length);
    }

    bool decode(const void* buffer, size_t buffer_size, void* data) const {
        return nanopb_decode_wrapper(*this, buffer, buffer_size, data);
    }
    
    // Metadata karşılaştırma için
    constexpr bool operator==(const mreq_metadata& other) const {
        return message_id == other.message_id;
    }
    
    constexpr bool operator!=(const mreq_metadata& other) const {
        return !(*this == other);
    }
};

// Derleme zamanında metadata oluşturmak için yardımcı fonksiyon

// Mesaj ID'sini hesaplamak için hash fonksiyonu
constexpr size_t hash_string(const char* str) {
    size_t hash = 5381;
    while (*str) {
        hash = ((hash << 5) + hash) + static_cast<size_t>(*str++); // hash * 33 + c
    }
    return hash;
}

// Metadata oluşturma makrosu
#define MREQ_METADATA_DECLARE(name) \
    extern const mreq::mreq_metadata __mreq_##name;

#define MREQ_METADATA_DEFINE(type, name, topic_name_str, fields_ptr) \
    const mreq::mreq_metadata __mreq_##name = { \
        .topic_name = topic_name_str, \
        .payload_size = sizeof(type), \
        .message_id = mreq::hash_string(topic_name_str), \
        .fields = fields_ptr \
    };

// Topic ID makrosu - metadata pointer'ını döndürür
#define MREQ_ID(name) (&__mreq_##name)

} // namespace mreq 
