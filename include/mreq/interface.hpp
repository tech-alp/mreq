#pragma once
#include "topic.hpp"
#include <map>
#include <string>
#include <memory>
#include "mreq/mutex.hpp"
#include "mreq/internal/LockGuard.hpp"

class TopicRegistry {
    std::map<std::string, std::unique_ptr<BaseTopic>> topics;
    mreq::Mutex mtx;
    using LockType = mreq::LockGuard<mreq::Mutex>;
    TopicRegistry() = default;
public:
    // Singleton erişim
    static TopicRegistry& instance() {
        static TopicRegistry inst;
        return inst;
    }

    // Topic'i kaydet (manuel veya otomatik)
    template<typename T>
    Topic<T>& register_topic(const std::string& name) {
        LockType lock(mtx);
        if (topics.count(name) == 0) {
            topics[name] = std::make_unique<Topic<T>>();
        }
        return *static_cast<Topic<T>*>(topics[name].get());
    }

    // İsimle ve tip güvenli topic erişimi
    template<typename T>
    Topic<T>& get_topic(const std::string& name) {
        LockType lock(mtx);
        if (topics.count(name) == 0) {
            // Hata: topic yok
            while(1); // veya assert(false);
        }
        return *static_cast<Topic<T>*>(topics[name].get());
    }

    // Sadece isimle erişim (tip kontrolü olmadan, örnek kullanım için)
    BaseTopic* get(const std::string& name) {
        LockType lock(mtx);
        if (topics.count(name) == 0) return nullptr;
        return topics[name].get();
    }
};
