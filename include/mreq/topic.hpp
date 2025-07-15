#pragma once
#include <optional>
#include <array>
#include "subscriber_table.hpp"
#include "mreq/mutex.hpp"
#include "mreq/internal/LockGuard.hpp"

class BaseTopic {
public:
    virtual ~BaseTopic() = default;
};

// Varsayılan: Tekli buffer, Opsiyonel: Ring buffer
// N=1: klasik, N>1: ring buffer

template<typename T, size_t N = 1>
class Topic : public BaseTopic {
    static_assert(N >= 1, "Buffer boyutu en az 1 olmalı");
    std::array<T, N> buffer{};
    size_t sequence = 0;
    size_t head = 0; // ring buffer için
    mreq::Mutex mtx;
    using LockType = mreq::LockGuard<mreq::Mutex>;
    mutable SubscriberTable<T> subscribers;

public:
    void publish(const T& msg) {
        LockType lock(mtx);
        buffer[head] = msg;
        head = (head + 1) % N;
        ++sequence;
        subscribers.notify_publish(sequence);
    }

    std::optional<size_t> subscribe() {
        return subscribers.subscribe();
    }

    // Abone için en güncel veriyi döndürür (ring buffer'da en son okunmayanı)
    std::optional<T> read(size_t token) {
        LockType lock(mtx);
        if (subscribers.check(token, sequence)) {
            subscribers.update_read_seq(token, sequence);
            // Ring buffer'da en güncel veriyi döndür
            size_t idx = (head + N - 1) % N;
            return buffer[idx];
        }
        return std::nullopt;
    }

    void unsubscribe(size_t token) {
        subscribers.unsubscribe(token);
    }

    bool check(size_t token) const {
        LockType lock(const_cast<mreq::Mutex&>(mtx));
        return subscribers.check(token, sequence);
    }
};
