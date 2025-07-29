#pragma once

#include <array>
#include <optional>
#include <functional>
#include <type_traits>
#include <cassert>
#include "topic.hpp"
#include "metadata.hpp"
#include "mutex.hpp"
#include "internal/LockGuard.hpp"
#include "internal/NonCopyable.hpp"

#ifndef MREQ_MAX_TOPICS
#define MREQ_MAX_TOPICS 16 // Varsayılan maksimum topic sayısı
#endif

namespace mreq {

// Bu yapı, TopicRegistry'nin farklı Topic<T> türlerini
// türden bağımsız olarak yönetmesini sağlar.
struct TopicMetadata {
    ITopic* topic_ptr;
    std::function<std::optional<size_t>()> subscribe_fn;
    std::function<bool(size_t)> check_fn;
    std::function<void(size_t)> unsubscribe_fn;
};

// Gömülü sistemler için tasarlanmış, statik bellek kullanan TopicRegistry.
// Dinamik bellek ayırımından (heap) kaçınır.
class TopicRegistry : private internal::NonCopyable {
private:
    // Her topic için gerekli meta verileri ve type-erased fonksiyonları saklar.
    std::array<TopicMetadata, MREQ_MAX_TOPICS> topic_slots_{};
    // Her slotun metadata ID'sini saklar.
    std::array<const mreq_metadata*, MREQ_MAX_TOPICS> metadata_ptrs_{};
    size_t topic_count_ = 0;
    mutable mreq::Mutex mtx_{};

    using LockType = mreq::LockGuard<mreq::Mutex>;

    TopicRegistry() = default;

public:
    static TopicRegistry& instance() {
        static TopicRegistry inst;
        return inst;
    }

    // ID ile bir TopicMetadata bulur.
    TopicMetadata* find(const mreq_metadata* id) {
        LockType lock(mtx_);
        for (size_t i = 0; i < topic_count_; ++i) {
            if (metadata_ptrs_[i] == id) {
                return &topic_slots_[i];
            }
        }
        return nullptr;
    }

    // Yeni bir Topic'i ve metadata'sını registry'e ekler.
    void add(const mreq_metadata* id, TopicMetadata&& metadata) {
        LockType lock(mtx_);
        if (topic_count_ >= MREQ_MAX_TOPICS) {
            assert(false && "MREQ: Maksimum topic sayısına ulaşıldı!");
            return;
        }

        // Zaten eklenmiş mi diye kontrol et
        for (size_t i = 0; i < topic_count_; ++i) {
            if (metadata_ptrs_[i] == id) {
                return; // Zaten var, tekrar ekleme.
            }
        }

        metadata_ptrs_[topic_count_] = id;
        topic_slots_[topic_count_] = std::move(metadata);
        topic_count_++;
    }
};

} // namespace mreq
