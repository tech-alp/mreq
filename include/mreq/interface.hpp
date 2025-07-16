#pragma once
#include "topic.hpp"
#include <array>
#include <string_view> // For std::string_view (C++17)
#include <type_traits> // For std::aligned_storage
#include <optional>
#include <functional>
#include <cstdio> // For printf
#include "topic.hpp"
#include "mreq/mutex.hpp"
#include "mreq/internal/LockGuard.hpp"
#include <cassert> // For assert

// Define MREQ_ENABLE_LOGGING to enable basic logging hooks
// #define MREQ_ENABLE_LOGGING

#ifndef MREQ_MAX_TOPICS
#define MREQ_MAX_TOPICS 16 // Varsayılan maksimum topic sayısı
#endif

// TopicRegistry, gömülü sistemlerde heap kullanımını minimize etmek için
// std::map ve std::unique_ptr yerine statik bir yapıya dönüştürülmüştür.
class TopicRegistry {
    // Statik bellek havuzu için Topic nesnelerini tutacak array
    // En büyük Topic<T> boyutunu ve hizalamasını bulmak için bir mekanizma gerekebilir.
    // Şimdilik, BaseTopic'in en büyük türevinin boyutunu varsayalım veya
    // daha güvenli bir yaklaşım için compile-time hesaplama yapalım.
    // Basitlik adına, şimdilik Topic<T> nesnelerinin doğrudan depolanabileceği
    // yeterince büyük bir aligned_storage kullanacağız.
    // Gerçek bir implementasyonda, her Topic<T> için ayrı bir aligned_storage
    // veya daha sofistike bir bellek havuzu yönetimi gerekebilir.
    // Şimdilik, BaseTopic'in en büyük türevinin boyutunu ve hizalamasını varsayalım.
    // Bu, Topic<T, N> boyutuna bağlı olacaktır. En kötü durum senaryosu için
    // yeterince büyük bir boyut seçmeliyiz.
    static constexpr size_t MAX_TOPIC_SIZE = sizeof(Topic<int, 100>); // Örnek: En büyük olabilecek bir Topic boyutu
    static constexpr size_t MAX_TOPIC_ALIGN = alignof(Topic<int, 1>); // Topic hizalaması

    std::array<typename std::aligned_storage<MAX_TOPIC_SIZE, MAX_TOPIC_ALIGN>::type, MREQ_MAX_TOPICS> topic_storage;
    std::array<const char*, MREQ_MAX_TOPICS> topic_names; // Topic isimlerini tutmak için
    std::array<bool, MREQ_MAX_TOPICS> topic_active; // Slotun dolu olup olmadığını gösterir
    size_t next_free_slot = 0; // Bir sonraki boş slotun indeksi

    mutable mreq::Mutex mtx;
    using LockType = mreq::LockGuard<mreq::Mutex>;

    TopicRegistry() = default; // Singleton

public:
    // Singleton erişim
    static TopicRegistry& instance() noexcept {
        static TopicRegistry inst;
        return inst;
    }

    // Topic'i kaydet (manuel veya otomatik)
    // Eğer topic zaten varsa, mevcut olanı döndürür.
    template<typename T, size_t N>
    Topic<T, N>& register_topic(const std::string_view& name) {
        LockType lock(mtx);

        for (size_t i = 0; i < next_free_slot; ++i) {
            if (topic_active[i] && topic_names[i] == name) {
#ifdef MREQ_ENABLE_LOGGING
                printf("MREQ_LOG: Topic '%s' already registered. Returning existing instance.\n", name.data());
#endif
                return *static_cast<Topic<T, N>*>(reinterpret_cast<BaseTopic*>(&topic_storage[i]));
            }
        }

        if (next_free_slot >= MREQ_MAX_TOPICS) {
#ifdef MREQ_ENABLE_LOGGING
            printf("MREQ_LOG: ERROR: Maximum topic count reached! Cannot register topic '%s'.\n", name.data());
#endif
            assert(false && "MREQ: Maximum topic count reached!");
        }

        Topic<T, N>* new_topic = new (&topic_storage[next_free_slot]) Topic<T, N>();
        topic_names[next_free_slot] = name.data();
        topic_active[next_free_slot] = true;

        next_free_slot++;
#ifdef MREQ_ENABLE_LOGGING
        printf("MREQ_LOG: Topic '%s' registered at slot %zu.\n", name.data(), next_free_slot - 1);
#endif
        return *new_topic;
    }

    // İsimle ve tip güvenli topic erişimi
    // Topic bulunamazsa std::optional<std::reference_wrapper<Topic<T>>> döndürür
    // Böylece çağıran taraf hatayı güvenli bir şekilde ele alabilir.
    template<typename T, size_t N>
    std::optional<std::reference_wrapper<Topic<T, N>>> get_topic(const std::string_view& name) {
        LockType lock(mtx);
        for (size_t i = 0; i < next_free_slot; ++i) {
            if (topic_active[i] && topic_names[i] == name) {
#ifdef MREQ_ENABLE_LOGGING
                printf("MREQ_LOG: Found topic '%s' at slot %zu.\n", name.data(), i);
#endif
                return std::make_optional(std::ref(*static_cast<Topic<T, N>*>(reinterpret_cast<BaseTopic*>(&topic_storage[i]))));
            }
        }
#ifdef MREQ_ENABLE_LOGGING
        printf("MREQ_LOG: Topic '%s' not found.\n", name.data());
#endif
        return std::nullopt; // Topic bulunamadı
    }

    // Sadece isimle erişim (tip kontrolü olmadan, örnek kullanım için)
    // Topic bulunamazsa nullptr döndürür. Çağıranın bu nullptr'ı doğru şekilde ele alması beklenir.
    BaseTopic* get(const std::string_view& name) {
        LockType lock(mtx);
        for (size_t i = 0; i < next_free_slot; ++i) {
            if (topic_active[i] && topic_names[i] == name) {
#ifdef MREQ_ENABLE_LOGGING
                printf("MREQ_LOG: Found BaseTopic '%s' at slot %zu.\n", name.data(), i);
#endif
                return reinterpret_cast<BaseTopic*>(&topic_storage[i]);
            }
        }
#ifdef MREQ_ENABLE_LOGGING
        printf("MREQ_LOG: BaseTopic '%s' not found.\n", name.data());
#endif
        return nullptr;
    }
};
