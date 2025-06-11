#pragma once
#include <mutex>
#include <optional>
#include "subscriber_table.hpp"

class BaseTopic {
public:
    virtual ~BaseTopic() = default;
};

template<typename T>
class Topic : public BaseTopic {
    T buffer;
    size_t sequence = 0;
    mutable std::mutex mtx;
    SubscriberTable<T> subscribers;

public:
    void publish(const T& msg) {
        std::lock_guard<std::mutex> lock(mtx);
        buffer = msg;
        ++sequence;
        subscribers.notify_publish(sequence);
    }

    std::optional<size_t> subscribe() {
        return subscribers.subscribe();
    }

    // Modern ve ergonomik read fonksiyonu
    std::optional<T> read(size_t token) {
        std::lock_guard<std::mutex> lock(mtx);
        if (subscribers.check(token, sequence)) {
            subscribers.update_read_seq(token, sequence);
            return buffer;
        }
        return std::nullopt;
    }

    
    void unsubscribe(size_t token) {
        subscribers.unsubscribe(token);
    }

    // İsterseniz hala check kullanmak için bırakabilirsin
    bool check(size_t token) const {
        std::lock_guard<std::mutex> lock(mtx);
        return subscribers.check(token, sequence);
    }
};
