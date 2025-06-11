#pragma once
#include "topic.hpp"
#include <typeindex>
#include <map>
#include <memory>
#include <mutex>

// Custom deleter ile unique_ptr: doğru tipten delete yapılır!
class TopicRegistry {
    std::map<std::type_index, std::unique_ptr<void, void(*)(void*)>> topics;
    std::mutex mtx;
public:
    template<typename T>
    Topic<T>& get_topic() {
        std::lock_guard<std::mutex> lock(mtx);
        auto idx = std::type_index(typeid(T));
        if (topics.count(idx) == 0) {
            topics[idx] = { new Topic<T>(), [](void* ptr){ delete static_cast<Topic<T>*>(ptr); } };
        }
        return *reinterpret_cast<Topic<T>*>(topics[idx].get());
    }
    // Artık destructor'a gerek yok: unique_ptr otomatik temizler.
};

inline TopicRegistry topic_registry;
