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

class ITopic {
public:
    virtual ~ITopic() = default;
    virtual bool check(Token token) const noexcept = 0;
    virtual void unsubscribe(Token token) noexcept = 0;
};

// Varsayılan: Tekli buffer, Opsiyonel: Ring buffer
// N=1: klasik, N>1: ring buffer

template<typename T, size_t N = 1>
class Topic : public ITopic {
    static_assert(N >= 1, "Buffer boyutu en az 1 olmalı");
    std::array<T, N> buffer{};
    size_t sequence = 0; // Total number of messages published
    size_t head = 0;     // Current write position in the ring buffer
    mutable mreq::Mutex mtx; // Mutex for thread-safe access, made mutable for const methods
    using LockType = mreq::LockGuard<mreq::Mutex>;
    mutable SubscriberTable<T> subscribers; // Subscriber management

public:
    void publish(const T& msg) {
        LockType lock(mtx);
        buffer[head] = msg;
        head = (head + 1) % N;
        ++sequence;
#ifdef MREQ_ENABLE_LOGGING
        printf("MREQ_LOG: Topic published. New sequence: %zu, head: %zu\n", sequence, head);
#endif
        // subscribers.notify_publish(sequence); // Polling tabanlı olduğu için doğrudan bir bildirime gerek yok
    }

    std::optional<Token> subscribe() {
        LockType lock(mtx); // Topic'in durumunu koru
        std::optional<Token> token_opt = subscribers.subscribe();
        if (token_opt.has_value()) {
            Token token = token_opt.value();
            size_t initial_read_idx;
            if (sequence < N) {
                initial_read_idx = 0;
            } else {
                initial_read_idx = head;
            }
            subscribers.update_read_state(token, sequence, initial_read_idx);
#ifdef MREQ_ENABLE_LOGGING
            printf("MREQ_LOG: Subscriber %zu subscribed. Initial read seq: %zu, idx: %zu\n", token, sequence, initial_read_idx);
#endif
        }
        return token_opt;
    }

    // Abone için okunmamış en eski veriyi döndürür (ring buffer'da)
    std::optional<T> read(Token token) {
        LockType lock(mtx);
        SubscriberSlot& slot = subscribers.get_slot(token);

        if (slot.active) {
            size_t expected_read_seq = slot.last_read_seq;

            if (expected_read_seq < sequence) {
                size_t read_idx = slot.read_buffer_idx;

                if (N > 1 && (sequence - expected_read_seq) > N) {
                    read_idx = head;
                    expected_read_seq = sequence - N;
                }

                T msg_to_return = buffer[read_idx];

                slot.last_read_seq = expected_read_seq + 1;
                slot.read_buffer_idx = (read_idx + 1) % N;
#ifdef MREQ_ENABLE_LOGGING
                printf("MREQ_LOG: Subscriber %zu read message. New read seq: %zu, idx: %zu\n", token, slot.last_read_seq, slot.read_buffer_idx);
#endif
                return msg_to_return;
            }
        }
#ifdef MREQ_ENABLE_LOGGING
        printf("MREQ_LOG: Subscriber %zu read failed (no new message or inactive).\n", token);
#endif
        return std::nullopt;
    }

    // Çoklu Örnek Okuma (Multi-Sample Read) Desteği
    // Abone için okunmamış verileri out_buffer'a kopyalar ve okunan mesaj sayısını döndürür.
    size_t read_multiple(Token token, T* out_buffer, size_t count) {
        LockType lock(mtx);
        SubscriberSlot& slot = subscribers.get_slot(token);
        size_t messages_read = 0;

        if (!slot.active) {
#ifdef MREQ_ENABLE_LOGGING
            printf("MREQ_LOG: Subscriber %zu read_multiple failed (inactive).\n", token);
#endif
            return 0; // Abone aktif değil
        }

        for (size_t i = 0; i < count; ++i) {
            size_t expected_read_seq = slot.last_read_seq;

            if (expected_read_seq < sequence) {
                size_t read_idx = slot.read_buffer_idx;

                if (N > 1 && (sequence - expected_read_seq) > N) {
                    read_idx = head;
                    expected_read_seq = sequence - N;
                }

                out_buffer[messages_read] = buffer[read_idx];
                messages_read++;

                slot.last_read_seq = expected_read_seq + 1;
                slot.read_buffer_idx = (read_idx + 1) % N;
            } else {
#ifdef MREQ_ENABLE_LOGGING
                printf("MREQ_LOG: Subscriber %zu read_multiple stopped (no more messages).\n", token);
#endif
                break; // Okunacak başka mesaj yok
            }
        }
#ifdef MREQ_ENABLE_LOGGING
        printf("MREQ_LOG: Subscriber %zu read_multiple completed. Read %zu messages.\n", token, messages_read);
#endif
        return messages_read;
    }

    void unsubscribe(Token token) noexcept override {
        subscribers.unsubscribe(token);
#ifdef MREQ_ENABLE_LOGGING
        printf("MREQ_LOG: Subscriber %zu unsubscribed.\n", token);
#endif
    }

    bool check(Token token) const noexcept override {
        LockType lock(mtx);
        bool has_new_data = subscribers.check(token, sequence);
#ifdef MREQ_ENABLE_LOGGING
        printf("MREQ_LOG: Subscriber %zu check. Has new data: %s\n", token, has_new_data ? "true" : "false");
#endif
        return has_new_data;
    }
};

}
