#pragma once

// interface.hpp: API katmanı
// publish, subscribe, check, copy fonksiyon prototipleri

#include "topic.hpp"
#include "topic_traits.hpp"
#include "subscriber_table.hpp"
#include <atomic>
#include <cstring>
#include <type_traits>

// TODO: publish<T>, subscribe<T>, check<T>, copy<T> fonksiyon prototipleri

// Sadece şablon prototipleri (örnek implementasyon main.cpp'de olacak)

template<typename T>
void publish(const T& msg) {
    Topic<T>::publish(msg);
    // Tüm abonelere güncelleme işareti
    for (size_t i = 0; i < SubscriberTable<T>::flags.size(); ++i) {
        SubscriberTable<T>::set(i);
    }
}

template<typename T>
void subscribe(size_t subscriber_idx = 0) {
    // Gerçek sistemde abone kaydı yapılır, burada gerek yok
    // Sadece flag temizlenebilir
    SubscriberTable<T>::clear(subscriber_idx);
}

template<typename T>
bool check(size_t subscriber_idx = 0) {
    return SubscriberTable<T>::check(subscriber_idx);
}

template<typename T>
void copy(size_t subscriber_idx, T& out) {
    Topic<T>::copy(out);
    SubscriberTable<T>::clear(subscriber_idx);
}
