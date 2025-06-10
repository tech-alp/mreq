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
void publish(const T& msg);

template<typename T>
void subscribe();

template<typename T>
bool check();

template<typename T>
void copy(T& out);
