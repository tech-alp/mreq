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
    size_t last_read_seq = 0;    // Sequence number of the last message read by this subscriber
    size_t read_buffer_idx = 0;  // Index in the topic's ring buffer for this subscriber's next read
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
                // last_read_seq ve read_buffer_idx, Topic::subscribe() tarafından ayarlanacak
                // böylece abone sadece abonelik sonrası yayınlanan mesajları okur.
                slots[i].last_read_seq = 0;
                slots[i].read_buffer_idx = 0;
                return i;
            }
        }
        // Hiç boş slot yoksa abone alınamaz
        return std::nullopt;
    }

    // Abone çıkışı: slotu pasif yap
    void unsubscribe(size_t idx) noexcept {
        LockType lock(mtx);
        if (idx < slots.size()) {
            slots[idx].active = false;
            slots[idx].last_read_seq = 0;
            slots[idx].read_buffer_idx = 0;
        }
    }

    // Publisher tarafından çağrılır, ancak polling tabanlı sistemde SubscriberTable'ın doğrudan bir işlevi yoktur.
    // Sadece mutex koruması altında bir işlem yapılması gerekiyorsa kullanılabilir.
    // Mevcut durumda boş bir döngü içeriyor, bu kaldırılabilir veya amacı netleştirilmelidir.

    // Abone için yeni veri olup olmadığını kontrol eder
    // current_topic_seq: Topic'in en son yayınladığı mesajın sequence numarası
    bool check(size_t idx, size_t current_topic_seq) noexcept {
        LockType lock(mtx);
        if (idx < slots.size() && slots[idx].active) {
            return slots[idx].last_read_seq < current_topic_seq;
        }
        return false;
    }

    // Abonenin okuma sequence ve buffer indeksini günceller
    void update_read_state(size_t idx, size_t new_topic_seq, size_t new_buffer_idx) noexcept {
        LockType lock(mtx);
        if (idx < slots.size() && slots[idx].active) {
            slots[idx].last_read_seq = new_topic_seq;
            slots[idx].read_buffer_idx = new_buffer_idx;
        }
    }

    // Abone slotuna erişim (Topic sınıfı tarafından kullanılacak)
    SubscriberSlot& get_slot(size_t idx) noexcept {
        // Hata kontrolü eklenebilir (idx < slots.size() ve active kontrolü)
        return slots[idx];
    }

    // (Slot sayısını ve durumlarını göstermek için ek)
    size_t subscriber_count() const noexcept {
        LockType lock(mtx); // mtx artık mutable, const_cast'e gerek yok
        return std::count_if(slots.begin(), slots.end(),
            [](const SubscriberSlot& s) { return s.active; });
    }
};
