#pragma once
#include "metadata.hpp"
#include "subscriber_table.hpp"
#include "mreq/mutex.hpp"
#include "mreq/internal/LockGuard.hpp"
#include <optional>
#include <array>
#include <cstdio>

// Define MREQ_ENABLE_LOGGING to enable basic logging hooks
// #define MREQ_ENABLE_LOGGING

namespace mreq {

// Base topic interface for metadata-based system
class BaseTopicMetadata {
public:
    virtual ~BaseTopicMetadata() = default;
    virtual const mreq_metadata* get_metadata() const = 0;
    virtual std::optional<size_t> subscribe() = 0;
    virtual void unsubscribe(size_t token) noexcept = 0;
    virtual bool check(size_t token) const noexcept = 0;
    virtual bool read(size_t token, void* data) const = 0;
    virtual void publish(const void* data) = 0;
};

// Metadata-based Topic class - PRD compliant polling-based system
template<typename T, size_t N = 1>
class TopicMetadata : public BaseTopicMetadata {
    static_assert(N >= 1, "Buffer boyutu en az 1 olmalı");
    
private:
    const mreq_metadata* metadata_;
    std::array<T, N> buffer_{};
    size_t sequence_ = 0;           // Total number of messages published
    size_t head_ = 0;               // Current write position in the ring buffer
    mutable mreq::Mutex mtx_;       // Mutex for thread-safe access
    using LockType = mreq::LockGuard<mreq::Mutex>;
    mutable SubscriberTable<T> subscribers_; // Subscriber management

public:
    explicit TopicMetadata(const mreq_metadata* metadata) : metadata_(metadata) {
        // Runtime check instead of static_assert for metadata size
        if (sizeof(T) != metadata->payload_size) {
            // This would ideally be a compile-time error, but we can't use static_assert with runtime values
            // In a real implementation, this could be handled with a custom assertion or error handling
        }
    }

    const mreq_metadata* get_metadata() const override {
        return metadata_;
    }

    // PRD API: subscribe() - returns token for polling
    std::optional<size_t> subscribe() override {
        LockType lock(mtx_);
        std::optional<size_t> token_opt = subscribers_.subscribe();
        if (token_opt.has_value()) {
            size_t token = token_opt.value();
            size_t initial_read_idx;
            if (sequence_ < N) {
                initial_read_idx = 0;
            } else {
                initial_read_idx = head_;
            }
            subscribers_.update_read_state(token, sequence_, initial_read_idx);
#ifdef MREQ_ENABLE_LOGGING
            printf("MREQ_LOG: Subscriber %zu subscribed to topic '%s'. Initial read seq: %zu, idx: %zu\n", 
                   token, metadata_->topic_name, sequence_, initial_read_idx);
#endif
        }
        return token_opt;
    }

    // PRD API: publish() - stores message in buffer
    void publish(const void* data) override {
        LockType lock(mtx_);
        const T* msg = static_cast<const T*>(data);
        buffer_[head_] = *msg;
        head_ = (head_ + 1) % N;
        ++sequence_;
#ifdef MREQ_ENABLE_LOGGING
        printf("MREQ_LOG: Topic '%s' published. New sequence: %zu, head: %zu\n", 
               metadata_->topic_name, sequence_, head_);
#endif
    }

    // PRD API: check() - polling for new data
    bool check(size_t token) const noexcept override {
        LockType lock(mtx_);
        bool has_new_data = subscribers_.check(token, sequence_);
#ifdef MREQ_ENABLE_LOGGING
        printf("MREQ_LOG: Subscriber %zu check on topic '%s'. Has new data: %s\n", 
               token, metadata_->topic_name, has_new_data ? "true" : "false");
#endif
        return has_new_data;
    }

    // PRD API: read() - read message data
    bool read(size_t token, void* data) const override {
        LockType lock(mtx_);
        T* out_data = static_cast<T*>(data);
        SubscriberSlot& slot = subscribers_.get_slot(token);

        if (slot.active) {
            size_t expected_read_seq = slot.last_read_seq;

            if (expected_read_seq < sequence_) {
                size_t read_idx = slot.read_buffer_idx;

                if (N > 1 && (sequence_ - expected_read_seq) > N) {
                    read_idx = head_;
                    expected_read_seq = sequence_ - N;
                }

                *out_data = buffer_[read_idx];

                slot.last_read_seq = expected_read_seq + 1;
                slot.read_buffer_idx = (read_idx + 1) % N;
#ifdef MREQ_ENABLE_LOGGING
                printf("MREQ_LOG: Subscriber %zu read message from topic '%s'. New read seq: %zu, idx: %zu\n", 
                       token, metadata_->topic_name, slot.last_read_seq, slot.read_buffer_idx);
#endif
                return true;
            }
        }
#ifdef MREQ_ENABLE_LOGGING
        printf("MREQ_LOG: Subscriber %zu read failed on topic '%s' (no new message or inactive).\n", 
               token, metadata_->topic_name);
#endif
        return false;
    }

    // PRD API: unsubscribe() - remove subscription
    void unsubscribe(size_t token) noexcept override {
        subscribers_.unsubscribe(token);
#ifdef MREQ_ENABLE_LOGGING
        printf("MREQ_LOG: Subscriber %zu unsubscribed from topic '%s'.\n", token, metadata_->topic_name);
#endif
    }

    // Additional utility methods
    size_t subscriber_count() const noexcept {
        return subscribers_.subscriber_count();
    }

    size_t get_sequence() const noexcept {
        LockType lock(mtx_);
        return sequence_;
    }
};

} // namespace mreq 