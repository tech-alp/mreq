#pragma once

#include <optional>
#include "metadata.hpp"
#include "topic_registry.hpp"
#include "topic.hpp"

namespace mreq
{
    // Initialization
    void init();
    
    // Subscribe - simple and fast
    static std::optional<Token> subscribe(const mreq_metadata* metadata) noexcept {
        auto* topic = TopicRegistry::instance().find(metadata);
        return topic ? topic->subscribe() : std::nullopt;
    }
    
    // Publish - direct cast, maximum performance
    template<typename T>
    static bool publish(const mreq_metadata* metadata, const T& message) noexcept {
        auto* topic = TopicRegistry::instance().find(metadata);
        if (!topic) return false;
        
        // Direct cast to your Topic<T> template and call publish
        static_cast<Topic<T>*>(topic)->publish(message);
        return true;
    }
    
    // Read - zero overhead, uses your existing read method
    template<typename T>
    static std::optional<T> read(const mreq_metadata* metadata, Token token) noexcept {
        auto* topic = TopicRegistry::instance().find(metadata);
        return topic ? static_cast<Topic<T>*>(topic)->read(token) : std::nullopt;
    }
    
    // Multi-sample read - leverages your read_multiple method
    template<typename T>
    static size_t read_multiple(const mreq_metadata* metadata, Token token, T* out_buffer, size_t count) noexcept {
        auto* topic = TopicRegistry::instance().find(metadata);
        return topic ? static_cast<Topic<T>*>(topic)->read_multiple(token, out_buffer, count) : 0;
    }
    
    // Check token validity - uses your check method
    static bool check(const mreq_metadata* metadata, Token token) noexcept {
        auto* topic = TopicRegistry::instance().find(metadata);
        return topic ? topic->check(token) : false;
    }
    
    // Unsubscribe
    static bool unsubscribe(const mreq_metadata* metadata, Token token) noexcept {
        auto* topic = TopicRegistry::instance().find(metadata);
        if (!topic) return false;
        
        topic->unsubscribe(token);
        return true;
    }
    
    // Utility - check if topic exists
    static bool exists(const mreq_metadata* metadata) noexcept {
        return TopicRegistry::instance().find(metadata) != nullptr;
    }

} // namespace mreq
