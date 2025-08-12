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

### 1. Mesaj ve Topic Tanımlayın

Önce mesaj yapısını ve metadata'sını tanımlayın. Ardından `MREQ_TOPIC_DEFINE` makrosu ile topic'i oluşturun. Bu makro, topic'i otomatik olarak registry'ye kaydeder.

```cpp
// messages.hpp
#include "mreq/mreq.hpp"

struct SensorData {
    float temperature;
    float humidity;
    uint64_t timestamp;
};

// Metadata ve Topic tanımı
MREQ_MESSAGE_TYPE(SensorData, "sensor_data");
MREQ_TOPIC_DEFINE(SensorData, sensor_data, 1); // 1 elemanlı buffer
```

### 2. Topic'e Erişin ve Abone Olun

Topic'e erişmek için `_topic_instance` sonekli değişkeni kullanın ve abone olmak için `subscribe()` metodunu çağırın.

```cpp
// main.cpp
#include "messages.hpp"
#include <iostream>

// Topic'e erişim (otomatik olarak oluşturulur)
extern mreq::Topic<SensorData, 1> sensor_data_topic_instance;

// Abone ol ve polling token'ı al
auto token_opt = sensor_data_topic_instance.subscribe();
if (!token_opt) {
    // Abone olunamadı
    return -1;
}
auto token = token_opt.value();
```

### 3. Mesaj Yayınlayın

`publish()` metodu ile mesaj yayınlayın.

```cpp
SensorData sensor_msg{
    .temperature = 25.5f,
    .humidity = 60.0f,
    .timestamp = 1234567890
};

sensor_data_topic_instance.publish(sensor_msg);
```

### 4. Polling ile Mesaj Okuyun

`check()` ile yeni mesaj olup olmadığını kontrol edin ve `read()` ile mesajı okuyun. `read()` metodu `std::optional<SensorData>` döndürür.

```cpp
// Polling ile yeni mesaj kontrol et
if (sensor_data_topic_instance.check(token)) {
    if (auto msg_opt = sensor_data_topic_instance.read(token)) {
        const auto& received_data = msg_opt.value();
        std::cout << "Sıcaklık: " << received_data.temperature << "°C" << std::endl;
    }
}
```

## 🔧 API Referansı

### Mesaj ve Topic Tanımlama

```cpp
// Mesaj yapısını tanımlayın
struct MyMessage { /* ... */ };

// Mesaj için metadata oluşturun
MREQ_MESSAGE_TYPE(MyMessage, "my_message");

// Topic'i 10 elemanlı bir buffer ile tanımlayın ve kaydedin
MREQ_TOPIC_DEFINE(MyMessage, my_message, 10);

// Başka bir dosyada topic'i kullanmak için bildirin
MREQ_TOPIC_DECLARE(MyMessage, my_message, 10);
```

### Topic Erişimi ve Kullanımı

Topic'ler `_topic_instance` sonekiyle doğrudan erişilebilen statik nesnelerdir.

```cpp
// Topic'e erişim
extern mreq::Topic<MyMessage, 10> my_message_topic_instance;

// Abone ol (token al)
std::optional<Token> token_opt = my_message_topic_instance.subscribe();
Token token = token_opt.value();

// Mesaj yayınla
MyMessage msg = { /* ... */ };
my_message_topic_instance.publish(msg);

// Yeni mesaj var mı kontrol et
bool new_message = my_message_topic_instance.check(token);

// Mesaj oku (std::optional<MyMessage> döner)
if (auto msg_opt = my_message_topic_instance.read(token)) {
    // ... mesajı kullan
}

// Aboneliği iptal et
my_message_topic_instance.unsubscribe(token);
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