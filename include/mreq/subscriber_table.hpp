// src/mreq/subscriber_table.hpp
#pragma once
#include <algorithm>
#include <cstddef>
#include <atomic>
#include <optional>
#include <array>
#include "mreq/mutex.hpp"
#include "mreq/internal/LockGuard.hpp"

#ifndef MREQ_MAX_SUBSCRIBERS
#define MREQ_MAX_SUBSCRIBERS 8
#endif

// Abone için kayıt yapısı
struct SubscriberSlot {
    bool active = false;
    size_t last_read_seq = 0;
    // (İstersek thread_id, vs. eklenebilir)
};

template<typename T>
class SubscriberTable {
    std::array<SubscriberSlot, MREQ_MAX_SUBSCRIBERS> slots{};
    mreq::Mutex mtx;
    using LockType = mreq::LockGuard<mreq::Mutex>;

public:
    // Abone olmak isteyen için slot ayır, token/id döndür
    std::optional<size_t> subscribe() {
        LockType lock(mtx);
        for (size_t i = 0; i < slots.size(); ++i) {
            if (!slots[i].active) {
                slots[i].active = true;
                slots[i].last_read_seq = 0;
                return i;
            }
        }
        // Hiç boş slot yoksa abone alınamaz
        return std::nullopt;
    }

    // Abone çıkışı: slotu pasif yap
    void unsubscribe(size_t idx) {
        LockType lock(mtx);
        if (idx < slots.size()) {
            slots[idx].active = false;
            slots[idx].last_read_seq = 0;
        }
    }

    // (Publisher tarafından çağrılır)
    void notify_publish(size_t seq) {
        LockType lock(mtx);
        for (auto& slot : slots) {
            if (slot.active) {
                // Burada ekstra flag/seq logic eklenebilir
            }
        }
    }

    // Abone güncel veri var mı diye bakar
    bool check(size_t idx, size_t current_seq) {
        LockType lock(mtx);
        if (idx < slots.size() && slots[idx].active) {
            return slots[idx].last_read_seq < current_seq;
        }
        return false;
    }

    // Abone veri kopyaladıktan sonra kendi seq'ini günceller
    void update_read_seq(size_t idx, size_t current_seq) {
        LockType lock(mtx);
        if (idx < slots.size() && slots[idx].active) {
            slots[idx].last_read_seq = current_seq;
        }
    }

    // (Slot sayısını ve durumlarını göstermek için ek)
    size_t subscriber_count() const {
        LockType lock(const_cast<mreq::Mutex&>(mtx));
        return std::count_if(slots.begin(), slots.end(),
            [](const SubscriberSlot& s) { return s.active; });
    }
};
