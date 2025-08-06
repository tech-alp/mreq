#pragma once

#include <array>
#include <optional>
#include <functional>
#include <type_traits>
#include <cassert>
#include <any>
#include "mutex.hpp"
#include "topic.hpp"
#include "metadata.hpp"
#include "internal/LockGuard.hpp"
#include "internal/NonCopyable.hpp"

#ifndef MREQ_MAX_TOPICS
#define MREQ_MAX_TOPICS 16
#endif

namespace mreq {

class TopicRegistry : private internal::NonCopyable {
private:
    // Use message_id for ultra-fast comparison instead of pointer comparison
    std::array<size_t, MREQ_MAX_TOPICS> message_ids_{};           // 8 bytes per entry
    std::array<const mreq_metadata*, MREQ_MAX_TOPICS> metadata_ptrs_{}; // 8 bytes per entry  
    uint8_t topic_count_ = 0;                                      // 1 byte
    mutable mreq::Mutex mtx_{};                                    // Platform specific
    
    TopicRegistry() = default;

public:
    static TopicRegistry& instance() noexcept {
        static TopicRegistry inst;
        return inst;
    }
    
    // ULTRA-FAST: Hot path - inline for maximum performance
    // O(1) average case with small linear search for embedded systems
    inline const mreq_metadata* find_by_id(size_t message_id) noexcept {
        // Fast path: try without lock first (read-only access)
        // Safe if registry is populated during initialization phase
        for (uint8_t i = 0; i < topic_count_; ++i) {
            if (message_ids_[i] == message_id) {
                return metadata_ptrs_[i];
            }
        }
        return nullptr;
    }
    
    // Legacy support: find by metadata pointer (slower)
    inline const mreq_metadata* find_by_metadata_ptr(const mreq_metadata* metadata_ptr) noexcept {
        if (!metadata_ptr) return nullptr;
        
        for (uint8_t i = 0; i < topic_count_; ++i) {
            if (metadata_ptrs_[i] == metadata_ptr) {
                return metadata_ptrs_[i];
            }
        }
        return nullptr;
    }
    
    // Thread-safe registration (called during initialization)
    bool register_topic(const mreq_metadata* metadata) noexcept {
        if (!metadata) return false;
        
        mreq::LockGuard<mreq::Mutex> lock(mtx_);
        
        if (topic_count_ >= MREQ_MAX_TOPICS) {
            return false; // Registry full
        }
        
        // Check for duplicate message_id (much faster than pointer comparison)
        size_t new_id = metadata->message_id;
        for (uint8_t i = 0; i < topic_count_; ++i) {
            if (message_ids_[i] == new_id) {
                return false; // Duplicate ID
            }
        }
        
        // Add new entry
        message_ids_[topic_count_] = new_id;
        metadata_ptrs_[topic_count_] = metadata;
        ++topic_count_;
        
#ifdef MREQ_ENABLE_LOGGING
        printf("REGISTRY: Registered topic '%s' (ID: %zu) at index %u\n", 
               metadata->topic_name, new_id, topic_count_ - 1);
#endif
        
        return true;
    }
    
    // Utility functions
    inline uint8_t size() const noexcept { 
        return topic_count_; 
    }
    
    inline bool full() const noexcept { 
        return topic_count_ >= MREQ_MAX_TOPICS; 
    }
    
    inline bool empty() const noexcept {
        return topic_count_ == 0;
    }
    
    // Get all registered topics (for diagnostics/monitoring)
    uint8_t get_all_topics(const mreq_metadata** out_array, uint8_t max_count) const noexcept {
        mreq::LockGuard<mreq::Mutex> lock(mtx_);
        
        uint8_t copy_count = (topic_count_ < max_count) ? topic_count_ : max_count;
        for (uint8_t i = 0; i < copy_count; ++i) {
            out_array[i] = metadata_ptrs_[i];
        }
        return copy_count;
    }
    
    // Get topic by index (for iteration)
    const mreq_metadata* get_topic_by_index(uint8_t index) const noexcept {
        if (index < topic_count_) {
            return metadata_ptrs_[index];
        }
        return nullptr;
    }
    
    // For debugging/testing - use with caution in production
    void clear() noexcept {
        mreq::LockGuard<mreq::Mutex> lock(mtx_);
        
        topic_count_ = 0;
        message_ids_.fill(0);
        metadata_ptrs_.fill(nullptr);
        
#ifdef MREQ_ENABLE_LOGGING
        printf("REGISTRY: All topics cleared\n");
#endif
    }
    
    // Memory usage diagnostics
    size_t get_memory_usage() const noexcept {
        return sizeof(TopicRegistry) + (topic_count_ * (sizeof(size_t) + sizeof(void*)));
    }
    
    // Performance diagnostics
    void print_diagnostics() const noexcept {
#ifdef MREQ_ENABLE_LOGGING
        mreq::LockGuard<mreq::Mutex> lock(mtx_);
        
        printf("=== TOPIC REGISTRY DIAGNOSTICS ===\n");
        printf("Total topics: %u/%u\n", topic_count_, MREQ_MAX_TOPICS);
        printf("Memory usage: %zu bytes\n", get_memory_usage());
        printf("Load factor: %.1f%%\n", (topic_count_ * 100.0f) / MREQ_MAX_TOPICS);
        
        for (uint8_t i = 0; i < topic_count_; ++i) {
            printf("  [%u] ID: %zu, Name: '%s'\n", 
                   i, message_ids_[i], 
                   metadata_ptrs_[i] ? metadata_ptrs_[i]->topic_name : "unknown");
        }
        printf("=================================\n");
#endif
    }
};

// Ultra-fast topic lookup by message ID
inline const mreq_metadata* find_topic_metadata(size_t message_id) noexcept {
    return TopicRegistry::instance().find_by_id(message_id);
}

// Legacy support
inline const mreq_metadata* find_topic_metadata(const mreq_metadata* metadata_ptr) noexcept {
    return TopicRegistry::instance().find_by_metadata_ptr(metadata_ptr);
}

// Register topic metadata
inline bool register_topic_metadata(const mreq_metadata* metadata) noexcept {
    return TopicRegistry::instance().register_topic(metadata);
}

} // namespace mreq

#define MREQ_TOPIC_DECLARE(MSGTYPE, NAME, BUFFER_SIZE) \
    extern mreq::Topic<MSGTYPE, BUFFER_SIZE> NAME##_topic_instance;

#define MREQ_TOPIC_DEFINE(MSGTYPE, NAME, BUFFER_SIZE) \
    mreq::Topic<MSGTYPE, BUFFER_SIZE> NAME##_topic_instance; \
    namespace { \
        struct NAME##_topic_initializer { \
            NAME##_topic_initializer() { \
                NAME##_topic_instance.bind_metadata(MREQ_GET_METADATA(NAME)); \
                bool success = ::mreq::register_topic_metadata(MREQ_GET_METADATA(NAME)); \
                assert(success && "Topic registry full or duplicate registration for " #NAME); \
                (void)success; /* Suppress unused variable warning in release */ \
            } \
        }; \
        [[maybe_unused]] static NAME##_topic_initializer NAME##_init_instance; \
    }

// Updated macros compatible with your existing API
#define REGISTER_TOPIC(MSGTYPE, NAME) \
    MREQ_TOPIC_DEFINE(MSGTYPE, NAME, 1)

#define REGISTER_TOPIC_WITH_BUFFER(MSGTYPE, NAME, BUFFER_SIZE) \
    MREQ_TOPIC_DEFINE(MSGTYPE, NAME, BUFFER_SIZE)