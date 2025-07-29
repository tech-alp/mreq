#pragma once
#include "topic_metadata.hpp"
#include "mreq/mutex.hpp"
#include "mreq/internal/LockGuard.hpp"
#include <unordered_map>
#include <memory>
#include <cstdio>

namespace mreq {

// Topic Registry for metadata-based system - PRD compliant
class TopicRegistryMetadata {
private:
    // Topic storage using metadata pointer as key
    std::unordered_map<const mreq_metadata*, std::unique_ptr<BaseTopicMetadata>> topics_;
    
    // Thread safety
    mutable mreq::Mutex mutex_;
    using LockType = mreq::LockGuard<mreq::Mutex>;
    
    // Singleton pattern
    TopicRegistryMetadata() = default;

public:
    // Singleton access
    static TopicRegistryMetadata& instance() noexcept {
        static TopicRegistryMetadata inst;
        return inst;
    }

    // Register a topic with metadata
    template<typename T, size_t N = 1>
    TopicMetadata<T, N>& register_topic(const mreq_metadata* metadata) {
        LockType lock(mutex_);
        
        // Check if topic already exists
        auto it = topics_.find(metadata);
        if (it != topics_.end()) {
            // Return existing topic
            auto* existing_topic = dynamic_cast<TopicMetadata<T, N>*>(it->second.get());
            if (existing_topic) {
#ifdef MREQ_ENABLE_LOGGING
                printf("MREQ_LOG: Topic '%s' already registered. Returning existing instance.\n", 
                       metadata->topic_name);
#endif
                return *existing_topic;
            }
        }

        // Create new topic
        auto new_topic = std::make_unique<TopicMetadata<T, N>>(metadata);
        auto* topic_ptr = new_topic.get();
        topics_[metadata] = std::move(new_topic);
        
#ifdef MREQ_ENABLE_LOGGING
        printf("MREQ_LOG: Topic '%s' registered with buffer size %zu.\n", 
               metadata->topic_name, N);
#endif
        
        return *topic_ptr;
    }

    // Get topic by metadata
    template<typename T, size_t N = 1>
    std::optional<std::reference_wrapper<TopicMetadata<T, N>>> get_topic(const mreq_metadata* metadata) {
        LockType lock(mutex_);
        
        auto it = topics_.find(metadata);
        if (it != topics_.end()) {
            auto* topic = dynamic_cast<TopicMetadata<T, N>*>(it->second.get());
            if (topic) {
#ifdef MREQ_ENABLE_LOGGING
                printf("MREQ_LOG: Found topic '%s'.\n", metadata->topic_name);
#endif
                return std::make_optional(std::ref(*topic));
            }
        }
        
#ifdef MREQ_ENABLE_LOGGING
        printf("MREQ_LOG: Topic '%s' not found.\n", metadata->topic_name);
#endif
        return std::nullopt;
    }

    // Get base topic by metadata (type-unsafe, for generic operations)
    BaseTopicMetadata* get(const mreq_metadata* metadata) {
        LockType lock(mutex_);
        
        auto it = topics_.find(metadata);
        if (it != topics_.end()) {
#ifdef MREQ_ENABLE_LOGGING
            printf("MREQ_LOG: Found BaseTopic '%s'.\n", metadata->topic_name);
#endif
            return it->second.get();
        }
        
#ifdef MREQ_ENABLE_LOGGING
        printf("MREQ_LOG: BaseTopic '%s' not found.\n", metadata->topic_name);
#endif
        return nullptr;
    }

    // Check if topic exists
    bool has_topic(const mreq_metadata* metadata) const {
        LockType lock(mutex_);
        return topics_.find(metadata) != topics_.end();
    }

    // Get topic count
    size_t topic_count() const {
        LockType lock(mutex_);
        return topics_.size();
    }

    // Clear all topics
    void clear() {
        LockType lock(mutex_);
        topics_.clear();
#ifdef MREQ_ENABLE_LOGGING
        printf("MREQ_LOG: All topics cleared from registry.\n");
#endif
    }
};

// Global convenience functions for PRD API
template<typename T, size_t N = 1>
inline TopicMetadata<T, N>& register_topic(const mreq_metadata* metadata) {
    return TopicRegistryMetadata::instance().register_topic<T, N>(metadata);
}

template<typename T, size_t N = 1>
inline std::optional<std::reference_wrapper<TopicMetadata<T, N>>> get_topic(const mreq_metadata* metadata) {
    return TopicRegistryMetadata::instance().get_topic<T, N>(metadata);
}

inline BaseTopicMetadata* get_topic(const mreq_metadata* metadata) {
    return TopicRegistryMetadata::instance().get(metadata);
}

} // namespace mreq 