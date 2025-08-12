#pragma once

#include <cstddef>
#include <optional>
#include "pb.h"
#include "pb_encode.h"
#include "pb_decode.h"

// Forward declarations
using Token = size_t;

namespace mreq {

struct mreq_metadata;

bool nanopb_encode_wrapper(const mreq_metadata& metadata, const void* data, void* buffer, size_t buffer_size, size_t* message_length);
bool nanopb_decode_wrapper(const mreq_metadata& metadata, const void* buffer, size_t buffer_size, void* data);

struct mreq_metadata {
    const char* topic_name;        // Topic adı (örn: "sensor_accel")
    size_t payload_size;           // Mesaj payload boyutu (nanopb struct size)
    size_t message_id;             // Benzersiz mesaj ID'si (compile-time hash)
    const pb_msgdesc_t* fields;    // nanopb mesaj tanımlayıcısı
    void* topic_instance;          // Type-erased topic pointer
    
    // Topic operations via function pointers (zero virtual call overhead)
    std::optional<Token> (*subscribe_fn)(void* topic);
    void (*unsubscribe_fn)(void* topic, Token token);
    bool (*check_fn)(void* topic, Token token);
    void (*publish_fn)(void* topic, const void* data);
    
    // Type-safe read operations (template specialization needed)
    void* (*read_fn)(void* topic, Token token, void* result);  // Returns result ptr if success
    size_t (*read_multiple_fn)(void* topic, Token token, void* buffer, size_t count);
    
    // nanopb serialization/deserialization fonksiyonları
    bool encode(const void* data, void* buffer, size_t buffer_size, size_t* message_length) const {
        return nanopb_encode_wrapper(*this, data, buffer, buffer_size, message_length);
    }
    
    bool decode(const void* buffer, size_t buffer_size, void* data) const {
        return nanopb_decode_wrapper(*this, buffer, buffer_size, data);
    }
    
    // ULTRA-FAST topic operations via direct function calls
    inline std::optional<Token> subscribe() const {
        return subscribe_fn ? subscribe_fn(topic_instance) : std::nullopt;
    }
    
    inline void unsubscribe(Token token) const {
        if (unsubscribe_fn) unsubscribe_fn(topic_instance, token);
    }
    
    inline bool check(Token token) const {
        return check_fn ? check_fn(topic_instance, token) : false;
    }
    
    inline void publish(const void* data) const {
        if (publish_fn) publish_fn(topic_instance, data);
    }
    
    // Type-safe read (caller must cast result)
    template<typename T>
    inline std::optional<T> read(Token token) const {
        if (!read_fn) return std::nullopt;
        
        T result;
        void* ret = read_fn(topic_instance, token, &result);
        return ret ? std::make_optional(result) : std::nullopt;
    }
    
    // Type-safe read multiple
    template<typename T>
    inline size_t read_multiple(Token token, T* buffer, size_t count) const {
        return read_multiple_fn ? read_multiple_fn(topic_instance, token, buffer, count) : 0;
    }
    
    // Metadata karşılaştırma için ID-based
    constexpr bool operator==(const mreq_metadata& other) const {
        return message_id == other.message_id;
    }
    
    constexpr bool operator!=(const mreq_metadata& other) const {
        return !(*this == other);
    }
    
    // Hash function for unordered_map (ID-based)
    struct Hash {
        constexpr size_t operator()(const mreq_metadata* metadata) const {
            return metadata->message_id;
        }
    };
    
    struct Equal {
        constexpr bool operator()(const mreq_metadata* a, const mreq_metadata* b) const {
            return a->message_id == b->message_id;
        }
    };
};

constexpr size_t constexpr_hash(const char* str) {
    size_t hash = 5381;
    while (*str) {
        hash = ((hash << 5) + hash) + static_cast<size_t>(*str++);
    }
    return hash;
}

inline bool nanopb_encode_wrapper(const mreq_metadata& metadata, const void* data, void* buffer, size_t buffer_size, size_t* message_length) {
    if (!metadata.fields) return false;

    pb_ostream_t stream = pb_ostream_from_buffer(static_cast<pb_byte_t*>(buffer), buffer_size);
    
    if (!pb_encode(&stream, metadata.fields, data)) {
        return false;
    }
    
    *message_length = stream.bytes_written;
    return true;
}

inline bool nanopb_decode_wrapper(const mreq_metadata& metadata, const void* buffer, size_t buffer_size, void* data) {
    if (!metadata.fields) return false;

    pb_istream_t stream = pb_istream_from_buffer(static_cast<const pb_byte_t*>(buffer), buffer_size);
    
    return pb_decode(&stream, metadata.fields, data);
}

} // namespace mreq

// Get metadata for runtime operations
#define MREQ_GET_METADATA(NAME) (&__mreq_##NAME)

// Get compile-time message ID
#define MREQ_GET_MESSAGE_ID(NAME) (__mreq_##NAME.message_id)

// Metadata oluşturma makrosu
#define MREQ_METADATA_DECLARE(name) \
    extern const mreq::mreq_metadata __mreq_##name;

#define MREQ_NANOPB_METADATA_DEFINE(type, name, buffer_size) \
    const mreq::mreq_metadata __mreq_##name = { \
        #name, \
        sizeof(type), \
        mreq::constexpr_hash(#name), \
        type##_fields, \
        &name##_topic_instance, \
        mreq::Topic<type, buffer_size>::static_subscribe, \
        mreq::Topic<type, buffer_size>::static_unsubscribe, \
        mreq::Topic<type, buffer_size>::static_check, \
        mreq::Topic<type, buffer_size>::static_publish, \
        mreq::Topic<type, buffer_size>::static_read, \
        mreq::Topic<type, buffer_size>::static_read_multiple \
    };

#define MREQ_METADATA_DEFINE(type, name, buffer_size) \
    const mreq::mreq_metadata __mreq_##name = { \
        #name, \
        sizeof(type), \
        mreq::constexpr_hash(#name), \
        nullptr, \
        &name##_topic_instance, \
        nullptr, \
        nullptr, \
        nullptr, \
        nullptr, \
        nullptr, \
        nullptr \
    };