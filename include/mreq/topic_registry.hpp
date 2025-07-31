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
    std::array<const mreq_metadata*, MREQ_MAX_TOPICS> metadata_ids_{};
    std::array<ITopic*, MREQ_MAX_TOPICS> topic_ptrs_{};
    uint8_t topic_count_ = 0;
    mutable mreq::Mutex mtx_{};
    
    TopicRegistry() = default;

public:
    static TopicRegistry& instance() noexcept {
        static TopicRegistry inst;
        return inst;
    }
    
    // Hot path - inline for maximum performance
    ITopic* find(const mreq_metadata* id) noexcept {
        mreq::LockGuard<mreq::Mutex> lock(mtx_);
        
        // Linear search - optimal for small arrays in embedded systems
        for (uint8_t i = 0; i < topic_count_; ++i) {
            if (metadata_ids_[i] == id) {
                return topic_ptrs_[i];
            }
        }
        return nullptr;
    }
    
    bool add(const mreq_metadata* id, ITopic* topic_ptr) noexcept {
        mreq::LockGuard<mreq::Mutex> lock(mtx_);
        
        if (topic_count_ >= MREQ_MAX_TOPICS) {
            return false;
        }
        
        // Duplicate check
        for (uint8_t i = 0; i < topic_count_; ++i) {
            if (metadata_ids_[i] == id) {
                return false;  // Already exists
            }
        }
        
        metadata_ids_[topic_count_] = id;
        topic_ptrs_[topic_count_] = std::move(topic_ptr);
        ++topic_count_;
        return true;
    }
    
    // Utility functions
    uint8_t size() const noexcept { return topic_count_; }
    bool full() const noexcept { return topic_count_ >= MREQ_MAX_TOPICS; }
    
    // For debugging/monitoring
    void clear() noexcept {
        mreq::LockGuard<mreq::Mutex> lock(mtx_);
        topic_count_ = 0;
        metadata_ids_.fill(nullptr);
        topic_ptrs_.fill(nullptr);
    }
};

} // namespace mreq


#define REGISTER_TOPIC(type, name) \
    REGISTER_TOPIC_WITH_BUFFER(type, name, 1)

#define REGISTER_TOPIC_WITH_BUFFER(type, name, buffer_size) \
do { \
    static mreq::Topic<type, buffer_size> topic_instance_##name; \
    [[maybe_unused]] bool success = mreq::TopicRegistry::instance().add( \
        MREQ_ID(name), &topic_instance_##name); \
    assert(success && "Topic registry full or duplicate registration"); \
} while (false)

