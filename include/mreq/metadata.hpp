#pragma once
#include <cstddef>
#include <cstdint>
#include <functional>

namespace mreq {

// nanopb uyumlu mesaj metadata yapısı
struct mreq_metadata {
    const char* topic_name;                    // Topic adı (örn: "sensor_accel")
    size_t payload_size;                       // Mesaj payload boyutu (nanopb struct size)
    size_t message_id;                         // Benzersiz mesaj ID'si (derleme zamanında hesaplanır)
    
    // nanopb serialization/deserialization fonksiyon pointer'ları
    using nanopb_encode_func_t = bool(*)(const void* data, void* buffer, size_t buffer_size, size_t* message_length);
    using nanopb_decode_func_t = bool(*)(const void* buffer, size_t buffer_size, void* data);
    
    nanopb_encode_func_t nanopb_encode;        // nanopb encode fonksiyonu
    nanopb_decode_func_t nanopb_decode;        // nanopb decode fonksiyonu
    
    // Metadata karşılaştırma için
    constexpr bool operator==(const mreq_metadata& other) const {
        return message_id == other.message_id;
    }
    
    constexpr bool operator!=(const mreq_metadata& other) const {
        return !(*this == other);
    }
};

// Derleme zamanında metadata oluşturmak için yardımcı fonksiyon
template<typename T>
constexpr mreq_metadata create_metadata(const char* name) {
    return mreq_metadata{
        .topic_name = name,
        .payload_size = sizeof(T),
        .message_id = 0, // Derleme zamanında hesaplanacak
        .nanopb_encode = nullptr, // nanopb encode fonksiyonu
        .nanopb_decode = nullptr  // nanopb decode fonksiyonu
    };
}

// Mesaj ID'sini hesaplamak için hash fonksiyonu
constexpr size_t hash_string(const char* str) {
    size_t hash = 5381;
    while (*str) {
        hash = ((hash << 5) + hash) + static_cast<size_t>(*str++); // hash * 33 + c
    }
    return hash;
}

// Metadata oluşturma makrosu
#define MREQ_METADATA_DECLARE(type, name) \
    extern const mreq::mreq_metadata __mreq_##type;

#define MREQ_METADATA_DEFINE(type, name) \
    const mreq::mreq_metadata __mreq_##type = { \
        .topic_name = name, \
        .payload_size = sizeof(type), \
        .message_id = mreq::hash_string(name), \
        .nanopb_encode = nullptr, \
        .nanopb_decode = nullptr \
    };

// Topic ID makrosu - metadata pointer'ını döndürür
#define MREQ_ID(type) (&__mreq_##type)

// Mesaj tipi için metadata tanımlama makrosu
#define MREQ_MESSAGE_TYPE(type, name) \
    MREQ_METADATA_DECLARE(type, name) \
    MREQ_METADATA_DEFINE(type, name)

// nanopb ile uyumlu metadata tanımlama makrosu
#define MREQ_NANOPB_MESSAGE_TYPE(type, name, encode_func, decode_func) \
    extern const mreq_metadata __mreq_##type; \
    const mreq_metadata __mreq_##type = { \
        .topic_name = name, \
        .payload_size = sizeof(type), \
        .message_id = mreq::hash_string(name), \
        .nanopb_encode = encode_func, \
        .nanopb_decode = decode_func \
    };

} // namespace mreq 
