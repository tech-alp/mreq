#pragma once

#include "topic_registry.hpp"
#include <optional>
#include <functional>

namespace mreq {

// Forward declaration
namespace autogen {
void register_topics();
}

inline void init() {
    autogen::register_topics();
}

template <typename T, size_t N = 1>
void register_topic(const mreq_metadata* id) {
    // Topic'in kendisi statik olarak oluşturulur.
    static Topic<T, N> topic_instance;
    
    TopicMetadata meta;
    meta.topic_ptr = &topic_instance;
    meta.subscribe_fn = [&]() { return topic_instance.subscribe(); };
    meta.check_fn = [&](size_t token) { return topic_instance.check(token); };
    meta.unsubscribe_fn = [&](size_t token) { topic_instance.unsubscribe(token); };
    
    TopicRegistry::instance().add(id, std::move(meta));
}

inline std::optional<size_t> subscribe(const mreq_metadata* id) {
    if (auto* meta = TopicRegistry::instance().find(id)) {
        return meta->subscribe_fn();
    }
    return std::nullopt;
}

inline bool check(const mreq_metadata* id, size_t token) {
    if (auto* meta = TopicRegistry::instance().find(id)) {
        return meta->check_fn(token);
    }
    return false;
}

inline void unsubscribe(const mreq_metadata* id, size_t token) {
    if (auto* meta = TopicRegistry::instance().find(id)) {
        meta->unsubscribe_fn(token);
    }
}

// read ve publish fonksiyonları, T tipini bilmek zorunda oldukları için
// templated kalmaya devam eder.
template <typename T>
void publish(const mreq_metadata* id, const T& msg) {
    if (auto* meta = TopicRegistry::instance().find(id)) {
        // topic_ptr'ı doğru tipe cast et.
        static_cast<Topic<T>*>(meta->topic_ptr)->publish(msg);
    }
}

template <typename T>
std::optional<T> read(const mreq_metadata* id, size_t token) {
    if (auto* meta = TopicRegistry::instance().find(id)) {
        return static_cast<Topic<T>*>(meta->topic_ptr)->read(token);
    }
    return std::nullopt;
}

} // namespace mreq