#pragma once
#include "topic.hpp"
#include <typeindex>
#include <map>
#include <memory>
#include <mutex>

class TopicRegistry {
    std::map<std::type_index, std::unique_ptr<BaseTopic>> topics;
    std::mutex mtx;
public:
    template<typename T>
    Topic<T>& get_topic() {
        std::lock_guard<std::mutex> lock(mtx);
        auto idx = std::type_index(typeid(T));
        if (topics.count(idx) == 0) {
            topics[idx] = std::make_unique<Topic<T>>();
        }
        return *static_cast<Topic<T>*>(topics[idx].get());
    }
};

inline TopicRegistry topic_registry;
