# MREQ - Derleme Zamanında Tip Güvenli Mesajlaşma Framework'ü

MREQ, gömülü sistemler ve gerçek zamanlı uygulamalar için tasarlanmış, **tamamen polling tabanlı** ve derleme zamanında tip güvenli bir mesajlaşma framework'üdür. Framework, runtime topic lookup'larını (string veya integer ID) derleme zamanında sabit tanımlayıcılarla değiştiren bir metadata sistemi kullanır.

## 🚀 Özellikler

- **Tamamen Polling Tabanlı**: Callback/dispatcher yok, sadece polling ile mesaj okuma
- **Derleme Zamanında Tip Güvenliği**: Runtime string lookup'ları yerine derleme zamanında sabit metadata pointer'ları
- **nanopb Uyumlu**: .proto dosyalarından üretilen struct'lar ile uyumlu
- **Hafif Tasarım**: Minimal bellek kullanımı ve overhead
- **Platform Bağımsız**: POSIX, FreeRTOS ve Bare Metal platformları destekler
- **Thread Safe**: Çoklu thread ortamlarında güvenli kullanım
- **C++17 Uyumlu**: Modern C++ özelliklerini kullanır
- **Ring Buffer Desteği**: Opsiyonel çoklu mesaj saklama
- **Dinamik Topic Registry**: Scope bağımsız topic yönetimi

## 📋 Gereksinimler

- C++17 uyumlu derleyici
- CMake 3.16+
- POSIX sistemler için pthread

## 🏗️ Kurulum

```bash
git clone <repository-url>
cd mreq
mkdir build && cd build
cmake ..
make
```

## 🎯 Hızlı Başlangıç

### 1. Mesaj Yapılarını Tanımlayın

```cpp
#include "mreq/mreq.hpp"

namespace mreq {

struct SensorData {
    float temperature;
    float humidity;
    uint64_t timestamp;
};

// Metadata tanımları
MREQ_MESSAGE_TYPE(SensorData, "sensor_data");

} // namespace mreq
```

### 2. Topic'i Registry'ye Kaydedin

```cpp
// Topic'i registry'ye kaydet
auto& sensor_topic = mreq::register_topic<mreq::SensorData>(MREQ_ID(SensorData));
```

### 3. Abone Olun (Polling için Token Alın)

```cpp
// Abone ol ve polling token'ı al
auto token = sensor_topic.subscribe();
if (!token) {
    // Abone olamadı
    return;
}
```

### 4. Mesaj Yayınlayın

```cpp
mreq::SensorData sensor_msg{
    .temperature = 25.5f,
    .humidity = 60.0f,
    .timestamp = 1234567890
};

sensor_topic.publish(&sensor_msg);
```

### 5. Polling ile Mesaj Okuyun

```cpp
// Polling ile yeni mesaj kontrol et
if (sensor_topic.check(token.value())) {
    mreq::SensorData received_data;
    if (sensor_topic.read(token.value(), &received_data)) {
        std::cout << "Sıcaklık: " << received_data.temperature << "°C" << std::endl;
    }
}
```

## 🔧 API Referansı

### Mesaj Tanımlama

```cpp
// Mesaj yapısı tanımlayın
struct MyMessage {
    // ... mesaj alanları
};

// Metadata tanımlayın
MREQ_MESSAGE_TYPE(MyMessage, "my_message");
```

### Topic Registry

```cpp
// Topic'i registry'ye kaydet
auto& topic = mreq::register_topic<MyMessage>(MREQ_ID(MyMessage));

// Topic'i registry'den al
auto topic_opt = mreq::get_topic<MyMessage>(MREQ_ID(MyMessage));
```

### Polling API

```cpp
// Abone ol (token al)
std::optional<size_t> subscribe();

// Mesaj yayınla
void publish(const void* data);

// Yeni mesaj var mı kontrol et
bool check(size_t token) const noexcept;

// Mesaj oku
bool read(size_t token, void* data) const;

// Aboneliği iptal et
void unsubscribe(size_t token) noexcept;
```

## 📁 Proje Yapısı

```
mreq/
├── include/mreq/
│   ├── metadata.hpp                    # Metadata sistemi
│   ├── topic_metadata.hpp              # Polling tabanlı Topic sınıfı
│   ├── topic_registry_metadata.hpp     # Topic Registry
│   ├── subscriber_table.hpp            # Abone yönetimi
│   ├── messages.hpp                    # Örnek mesaj yapıları
│   ├── mreq.hpp                        # Ana header
│   └── platform/                       # Platform spesifik kodlar
├── example/
│   ├── prd_compliant_example.cpp       # PRD uyumlu örnek
│   ├── metadata_example.cpp            # Eski callback örneği
│   └── CMakeLists.txt
└── README.md
```

## 🎮 Örnekler

### PRD Uyumlu Polling Örneği

```bash
cd example/build
./prd_compliant_example
```

Bu örnek PRD'deki gereksinimlere tam uyumlu:
- Tamamen polling tabanlı
- Callback/dispatcher yok
- Topic registry kullanımı
- Ring buffer desteği

## 🔍 Metadata Sistemi

MREQ'nin kalbi, derleme zamanında oluşturulan metadata sistemidir:

```cpp
struct mreq_metadata {
    const char* topic_name;                    // Topic adı
    size_t payload_size;                       // Mesaj boyutu
    size_t message_id;                         // Benzersiz ID (hash)
    nanopb_encode_func_t nanopb_encode;        // nanopb encode fonksiyonu
    nanopb_decode_func_t nanopb_decode;        // nanopb decode fonksiyonu
};
```

Her mesaj tipi için otomatik olarak oluşturulan metadata, derleme zamanında sabit bir adrese sahiptir ve bu adres topic identifier olarak kullanılır.

## 🚀 Performans Avantajları

- **Sıfır Runtime Overhead**: String lookup yok
- **Derleme Zamanında Optimizasyon**: Tüm topic ID'leri sabit
- **Minimal Bellek Kullanımı**: Sadece gerekli metadata
- **Hızlı Mesaj Yönlendirme**: Hash tabanlı lookup
- **Polling Optimizasyonu**: Callback overhead'i yok

## 🔧 Platform Desteği

### POSIX (Linux/macOS)

```cpp
#define MREQ_PLATFORM_POSIX
#include "mreq/mreq.hpp"
```

### FreeRTOS

```cpp
#define MREQ_PLATFORM_FREERTOS
#include "mreq/mreq.hpp"
```

### Bare Metal

```cpp
#define MREQ_PLATFORM_BAREMETAL
#include "mreq/mreq.hpp"
```

## 🐛 Hata Ayıklama

Logging'i etkinleştirmek için:

```cpp
#define MREQ_ENABLE_LOGGING
#include "mreq/mreq.hpp"
```

## 📄 Lisans

Bu proje MIT lisansı altında lisanslanmıştır.

## 🤝 Katkıda Bulunma

1. Fork yapın
2. Feature branch oluşturun (`git checkout -b feature/amazing-feature`)
3. Commit yapın (`git commit -m 'Add amazing feature'`)
4. Push yapın (`git push origin feature/amazing-feature`)
5. Pull Request oluşturun

## 📞 İletişim

Sorularınız için issue açabilir veya pull request gönderebilirsiniz. 