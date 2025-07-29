#pragma once

#include <optional>
#include "metadata.hpp"
#include "topic_registry.hpp"
#include "topic.hpp"

namespace mreq {

// Initialization
void init();

// Simplified API
template<size_t N = 10>
std::optional<Token> subscribe(const mreq_metadata* metadata) {
    if (!metadata) return std::nullopt;
    auto topic = TopicRegistry::instance().find(metadata);
    if (topic) {
        return topic->subscribe_fn();
    }
    return std::nullopt;
}

template<typename T>
bool publish(const mreq_metadata* metadata, const T& message) {
    if (!metadata) return false;
    auto topic = TopicRegistry::instance().find(metadata);
    if (topic) {
        topic->publish_fn(&message);
        return true;
    }
    return false;
}

bool check(const mreq_metadata* metadata, Token token);

template<typename T>
std::optional<T> read(const mreq_metadata* metadata, Token token) {
    if (!metadata) return std::nullopt;
    auto topic = TopicRegistry::instance().find(metadata);
    if (topic) {
        auto any_msg = topic->read_fn(token);
        if (any_msg) {
            return std::any_cast<T>(*any_msg);
        }
    }
    return std::nullopt;
}

void unsubscribe(const mreq_metadata* metadata, Token token);

} // namespace mreq
