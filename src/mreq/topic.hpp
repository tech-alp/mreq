#pragma once
#include <atomic>
#include <cstring>

// Topic<T>: Her mesaj tipi için şablon yapı
// PRD'ye göre, topic başına tek bir paylaşımlı bellek buffer'ı olacak.
// Detaylar ileride eklenecek.

template<typename T>
struct Topic {
    alignas(T) static inline unsigned char buffer[sizeof(T)];
    static inline std::atomic<bool> updated = false;

    static void publish(const T& msg) {
        std::memcpy(buffer, &msg, sizeof(T));
        updated.store(true, std::memory_order_release);
    }
    static void copy(T& out) {
        std::memcpy(&out, buffer, sizeof(T));
        updated.store(false, std::memory_order_release);
    }
    static bool check() {
        return updated.load(std::memory_order_acquire);
    }
};

