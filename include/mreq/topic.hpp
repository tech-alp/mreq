#pragma once
#include <optional>
#include <array>
#include <cstdio> // For printf
#include "subscriber_table.hpp"
#include "mreq/mutex.hpp"
#include "mreq/internal/LockGuard.hpp"

// Define MREQ_ENABLE_LOGGING to enable basic logging hooks
// #define MREQ_ENABLE_LOGGING

using Token = size_t;

namespace mreq {

template<typename T, size_t N = 1>
class Topic {
public:
    using value_type = T;
private:
    static_assert(N >= 1, "Buffer boyutu en az 1 olmalı");
    std::array<T, N> buffer_{};
    size_t sequence_ = 0;
    size_t head_ = 0;
    mutable mreq::Mutex mtx_;
    using LockType = mreq::LockGuard<mreq::Mutex>;
    mutable SubscriberTable<T> subscribers_;
    
    // Metadata pointer for this topic instance
    const mreq_metadata* metadata_ = nullptr;

public:
    // Constructor with metadata binding
    explicit Topic(const mreq_metadata* metadata = nullptr) : metadata_(metadata) {}
    
    // Bind metadata after construction
    void bind_metadata(const mreq_metadata* metadata) {
        metadata_ = metadata;
    }
    
    const mreq_metadata* get_metadata() const {
        return metadata_;
    }
    
    // All your existing methods remain the same
    void publish(const T& msg) {
        LockType lock(mtx_);
        buffer_[head_] = msg;
        head_ = (head_ + 1) % N;
        ++sequence_;
        
#ifdef MREQ_ENABLE_LOGGING
        printf("TOPIC[%s]: Published seq=%zu\n", 
               metadata_ ? metadata_->topic_name : "unknown", sequence_);
#endif
    }

    std::optional<Token> subscribe() {
        LockType lock(mtx_);
        std::optional<Token> token_opt = subscribers_.subscribe();
        if (token_opt.has_value()) {
            Token token = token_opt.value();
            size_t initial_read_idx = (sequence_ < N) ? 0 : head_;
            subscribers_.update_read_state(token, sequence_, initial_read_idx);
        }
        return token_opt;
    }

    std::optional<T> read(Token token) const {
        LockType lock(mtx_);
        SubscriberSlot& slot = subscribers_.get_slot(token);

        if (slot.active && slot.last_read_seq < sequence_) {
            size_t read_idx = slot.read_buffer_idx;
            
            if (N > 1 && (sequence_ - slot.last_read_seq) > N) {
                read_idx = head_;
                slot.last_read_seq = sequence_ - N;
            }

            T msg_to_return = buffer_[read_idx];
            slot.last_read_seq++;
            slot.read_buffer_idx = (read_idx + 1) % N;
            
            return msg_to_return;
        }
        return std::nullopt;
    }

    size_t read_multiple(Token token, T* out_buffer, size_t count) const {
        LockType lock(mtx_);
        SubscriberSlot& slot = subscribers_.get_slot(token);
        size_t messages_read = 0;

        if (!slot.active) return 0;

        for (size_t i = 0; i < count && slot.last_read_seq < sequence_; ++i) {
            size_t read_idx = slot.read_buffer_idx;
            
            if (N > 1 && (sequence_ - slot.last_read_seq) > N) {
                read_idx = head_;
                slot.last_read_seq = sequence_ - N;
            }

            out_buffer[messages_read++] = buffer_[read_idx];
            slot.last_read_seq++;
            slot.read_buffer_idx = (read_idx + 1) % N;
        }
        
        return messages_read;
    }

    void unsubscribe(Token token) noexcept {
        subscribers_.unsubscribe(token);
    }

    bool check(Token token) const noexcept {
        LockType lock(mtx_);
        return subscribers_.check(token, sequence_);
    }

    // Static functions for metadata function pointers
    static std::optional<Token> static_subscribe(void* topic_ptr) {
        return static_cast<Topic<T, N>*>(topic_ptr)->subscribe();
    }
    
    static void static_unsubscribe(void* topic_ptr, Token token) {
        static_cast<Topic<T, N>*>(topic_ptr)->unsubscribe(token);
    }
    
    static bool static_check(void* topic_ptr, Token token) {
        return static_cast<Topic<T, N>*>(topic_ptr)->check(token);
    }
    
    static void static_publish(void* topic_ptr, const void* data) {
        static_cast<Topic<T, N>*>(topic_ptr)->publish(*static_cast<const T*>(data));
    }
    
    static void* static_read(void* topic_ptr, Token token, void* result) {
        auto opt_result = static_cast<Topic<T, N>*>(topic_ptr)->read(token);
        if (opt_result.has_value()) {
            *static_cast<T*>(result) = *opt_result;
            return result;
        }
        return nullptr;
    }
    
    static size_t static_read_multiple(void* topic_ptr, Token token, void* buffer, size_t count) {
        return static_cast<Topic<T, N>*>(topic_ptr)->read_multiple(token, static_cast<T*>(buffer), count);
    }
};

}
