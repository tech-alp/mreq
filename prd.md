# MREQ – Modular Real-Time Embedded Queue Library  
**Product Requirements Document (PRD)**  
**Version:** 1.0  
**Date:** 2025-06-12

---

## 1. Amaç

**MREQ**, gömülü sistemlerde (özellikle STM32, ESP32 ve FreeRTOS/baremetal platformlarda), görevler arası güvenli ve minimum bellek tüketimli, gerçek zamanlı, polling tabanlı, publish/subscribe iletişim altyapısı sunar.  
Hedef; uORB/ROS2 benzeri topic tabanlı haberleşmeyi, .proto ile tanımlanan tipli mesaj yapılarıyla, harici PC/ROS Agent’a ihtiyaç duymadan ve sıfır-kopya ile embedded ortamda gerçekleştirmektir.

---

## 2. Kapsam & Hedefler

- **Thread-safe** topic temelli mesajlaşma (FreeRTOS veya baremetal uyumlu)
- **C++17** ile taşınabilir, hafif ve header-only veya static library olarak eklenebilir yapı
- **.proto** tabanlı tipli mesajlar (nanopb desteği)
- Minimum RAM/stack kullanımı, opsiyonel ring buffer desteği
- **Tamamen polling tabanlı:** callback/dispatcher yok
- **Otomatik kod üretimi ve topic isimlendirme:**
  - `.proto` mesajı için otomatik topic oluşturma
  - **Kurallar:**  
    - Eğer mesaj tanımının üstünde `// @topic: topic_one topic_two ...` varsa, mesaj tipi için her isimde birer topic oluşturulur  
    - Yorum satırı yoksa, mesaj ismiyle aynı adda bir topic açılır
- Kolay ve açık API: publish(), subscribe(), check(), read(), unsubscribe()
- **Tam kütüphane izolasyonu:** Kullanıcı dosyalarına referans verilmez
- **Dinamik Topic Registry/Manager:** Her topic, merkezi registry ile farklı scope’lardan erişilebilir

---

## 3. Kullanıcı Hikâyeleri / Senaryolar

- Birden fazla görev veya modül, ortak bir topic üzerinden mesaj paylaşmak ister
- Mesaj yapısı .proto ile tipli ve derleme sırasında struct’a dönüştürülür
- Her görev güncel veriyi diğerini etkilemeden okuyabilir
- Veri, tüm subscriber’lar tarafından bağımsızca okunduktan sonra otomatik silinir
- Topic nesneleri farklı scope’larda oluşturulabilir ve isim/id ile registry’den erişilebilir

---

## 4. Fonksiyonel Gereksinimler

### 4.1. Topic & Subscriber Yönetimi

- Her `.proto` mesajı için topic oluşturma kuralları:
  - `// @topic: ...` varsa, yorumda geçen her isimde topic oluştur
  - Yoksa, sadece mesaj ismiyle bir topic aç
- Statik veya opsiyonel ring buffer, abone yönetimi, her aboneye özel okundu flag’i veya sequence/counter kullanımı
- Dinamik topic registry/manager ile topic’lere scope bağımsız erişim
- Publish/subscribe/check/read işlemleri registry’den alınan pointer/ref ile yapılır

### 4.2. API

- **subscribe(topic_name):** Yeni abone, token döndürür
- **publish(topic_name, const msg*):** Mesajı buffer’a yazar, aboneleri günceller
- **check(topic_name, token):** Abone için yeni veri var mı?
- **read(topic_name, token, msg* out):** Mesajı okur, flag’i sıfırlar
- **unsubscribe(topic_name, token):** Aboneliği bırakır, slot serbest kalır
- **register_topic(name, Topic<T>):** Topic’i registry’ye ekler
- **get_topic<T>(name):** Topic nesnesini isimle döndürür

### 4.3. Otomatik Kod Üretimi

- `msg/*.proto` dosyaları taranır
- nanopb ile header/source dosyaları (`${CMAKE_BINARY_DIR}/gen/` klasörüne) üretilir
- Python scripti, tüm topic adları ve mesaj tipleriyle topic_registry_autogen.hpp/cpp dosyalarını oluşturur
- Topic isimlendirme sırasında yukarıdaki kurallar uygulanır

### 4.4. Thread-Safety / RTOS Uyum

- **FreeRTOS:** Mutex veya task-safe kritik bölge koruması (opsiyonel disable)
- **Baremetal:** Minimum lock, atomic flag
- Bellek yönetiminde mümkün olan her yerde statik veya stack-allocation (heap sadece gerektiğinde)

### 4.5. Bellek & Performans

- Her topic için buffer ve abone tablosu compile-time veya ilk publish/subscribe anında tahsis edilir
- Struct pointer’ı, memcpy veya nanopb ile efficient serialization sağlanır
- Topic ve abone başına minimum RAM (hedef: ~200-300 byte/topic)
- Ring buffer ile eski veriyi saklama ve overwrite seçeneği

### 4.6. Polling Tabanlı Çalışma

- Her subscriber istediği sıklıkta check/read çağırır
- Callback/interrupt yok; uygulama tarafı döngüsünde polling ile kontrol

### 4.7. CMake Entegrasyonu

- Uygulama kökünde:
  - `msg/*.proto` → `gen/*.pb.h/c` (nanopb)
  - Topic registry → `gen/topic_registry_autogen.hpp`
- Kütüphane `lib/mreq/` veya `third_party/mreq/` olarak eklenir
- Build sistemi kod üretimi adımlarını otomatik tetikler

---

## 5. Performans ve Test Kriterleri

- Publish işlemi O(N) veya O(1) (abone sayısına göre)
- Bellek kullanımı kolay izlenebilir (statik analiz, runtime assert)
- 10+ topic, 10+ abone için ~2kB RAM altı hedeflenir
- Her topic/abone farklı task’larda veri bütünlüğüyle çalışır
- STM32/ESP32 FreeRTOS ve baremetal test örnekleri sağlanır

---

## 6. Sınırlamalar & Kısıtlar

- Mesaj büyüklüğü compile-time’da sabit (nanopb struct size)
- Abone/buffer limiti config ile sınırlandırılabilir, compile-time veya runtime’da kontrol
- Sadece polling ile kullanılacak, event/callback/interrupt desteklenmez

---

## 7. Uzatılabilirlik / Gelecek Gelişimler

- Multi-sample ve ring buffer desteği
- Cross-core task/thread management
- Custom memory pool opsiyonu
- Otomatik topic trace/logger
- Gelişmiş dynamic topic registry/manager
- Gelişmiş dokümantasyon üretici/arayüz scriptleri

---

## 8. Dokümantasyon & Destek

- Fonksiyonel ve örnekli API dokümantasyonu
- Uygulama örnekleri (sensor → kontrol → log)
- Build/auto-generation örneği (CMake + Python script)
- STM32 ve ESP32 örnekleri
- Topic registry/manager ve paylaşım best-practice’leri,  
  **çoklu topic adı ile otomatik topic oluşturma kuralları** örneklerle gösterilir

---

## 9. Kaynaklar

- [uORB (PX4)](https://github.com/PX4/PX4-Autopilot/tree/main/src/modules/uORB)
- [nanopb](https://jpa.kapsi.fi/nanopb/)
- [FreeRTOS](https://www.freertos.org/)
- [ROS2 Topic API (intra-process)](https://docs.ros.org/en/foxy/Concepts/Intra-Process-Communication.html)

---

## 10. Sonuç

MREQ, embedded sistemlerde gerçek zamanlı, hafif, otomatik kod üreten ve topic-based publish/subscribe iletişimi sağlayan modern bir kütüphane olarak, minimum bağımlılık ve maksimum esneklik hedefiyle geliştirilmiştir.

---

## 11. Tavsiye Edilen Dosya Yapısı

```
your_project/
│
├── lib/
│   └── mreq/
│        ├── include/
│        │     └── mreq/
│        │            ├── mreq.hpp
│        │            ├── topic.hpp
│        │            ├── subscriber_table.hpp
│        │            └── ...
│        ├── src/
│        └── tools/
│              ├── proto_registry.py
│              └── nanopb_generator.py
│        └── README.md
│        └── LICENSE
│
├── msg/
│     ├── sensor_baro.proto
│     └── sensor_accel.proto
│
├── gen/
│     ├── sensor_baro.pb.h
│     ├── sensor_baro.pb.c
│     ├── sensor_accel.pb.h
│     ├── sensor_accel.pb.c
│     └── topic_registry_autogen.hpp
│
├── src/
│     └── main.cpp
│
├── CMakeLists.txt
└── ...
```

---

### Kütüphane Entegrasyon Akışı

```
.msg/ ----> [nanopb_generator.py] ----> gen/*.pb.h/c
      |
      |------> [proto_registry.py] ----> gen/topic_registry_autogen.hpp/cpp
                                              |
                      +-----------------------+----------------------+
                      |                      |                      |
                  topic.hpp            subscriber_table.hpp      mreq.hpp (API)
                      |                      |                      |
                      +------------------- Uygulama Kodu ---------------------+
                                               |
                      (publish/subscribe/dynamic registry üzerinden erişim)
```

---

### Dynamic Topic Registry Kullanımı (Örnek Kod)

```cpp
// 1. Bir kere, başlatıcı bir yerde:
auto& baro_topic = mreq::TopicRegistry::instance().register_topic<SensorBaro>("sensor_baro");
// veya init() fonksiyonunda çağırabilirsin.

// 2. Diğer her yerde sadece:
auto& baro_topic2 = mreq::TopicRegistry::instance().get("sensor_baro");
auto token = baro_topic2.subscribe();
if (baro_topic2.check(token)) {
    SensorBaro rx;
    baro_topic2.read(token, rx);
    // Kullan...
}

```
> Topic nesnesi nerede tanımlanırsa tanımlansın, uygulamanın her yerinden ismiyle erişilebilir.

---

**Bu doküman, MREQ'nin gerçek zamanlı, taşınabilir ve sürdürülebilir bir publish/subscribe kütüphanesi olarak kullanılmasını sağlar.**
