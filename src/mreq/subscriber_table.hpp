#pragma once
#include <atomic>
#include <array>

// SubscriberTable<T>: Her abone için güncelleme bayrağı takibi
// PRD'ye göre, her topic için abone başına flag tutulacak.

template<typename T, size_t MaxSubs = 8>
struct SubscriberTable {
    static inline std::array<std::atomic<bool>, MaxSubs> flags = {};

    static void set(size_t idx) { flags[idx].store(true, std::memory_order_release); }
    static void clear(size_t idx) { flags[idx].store(false, std::memory_order_release); }
    static bool check(size_t idx) { return flags[idx].load(std::memory_order_acquire); }
};

