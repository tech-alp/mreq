# MREQ – Modular Real-Time Embedded Queue

MREQ, gerçek zamanlı gömülü sistemler için hafif, modüler ve tip güvenli bir publish-subscribe iletişim framework'üdür. FreeRTOS ve baremetal ortamlarda görevler arası güvenli ve verimli veri paylaşımını sağlar.

## Klasör Yapısı

```
/proto/                   → .proto tanımları
/gen/                     → nanopb tarafından üretilen kodlar
/include/mreq/
  ├── mreq.hpp
  ├── topic.hpp
  ├── subscriber_table.hpp
  ├── interface.hpp
  ├── topic_registry.hpp
/tools/
  ├── proto_registry.py
  └── nanopb_generator.py
```

## Temel Özellikler
- Gerçek zamanlı, görev güvenli iletişim (opsiyonel thread-safety)
- .proto dosyaları ve nanopb ile tipli mesajlar
- Dispatcher/callback yok, saf polling tabanlı erişim
- Statik veya ring buffer ile sıfır kopya veri yönetimi
- Compile-time abone limiti (CMake ile ayarlanabilir)
- Otomatik kod üretimi (Python script + CMake)
- STM32 ve ESP32 (FreeRTOS/baremetal) uyumlu

## Derleme Zamanı Seçenekleri
- **MREQ_MAX_SUBSCRIBERS:** Maksimum abone sayısı (örn. CMake'de: `add_compile_definitions(MREQ_MAX_SUBSCRIBERS=8)`)

## Ring Buffer Kullanımı
Varsayılan olarak her topic tekli buffer ile çalışır. Çoklu örnek saklamak için:
```cpp
#include "mreq/topic.hpp"
Topic<MyMsg, 4> myTopic; // 4 elemanlı ring buffer
```

## Abone Limiti
Abone sayısı compile-time'da belirlenir. Limiti aşan subscribe çağrısı başarısız olur:
```cpp
std::array<std::optional<size_t>, MREQ_MAX_SUBSCRIBERS> tokens;
for (size_t i = 0; i < MREQ_MAX_SUBSCRIBERS; ++i)
    tokens[i] = myTopic.subscribe();
assert(!myTopic.subscribe().has_value()); // Limit aşıldı
```

## Temel API Kullanımı
```cpp
#include "mreq/mreq.hpp"
// Topic kaydı (opsiyonel, registry ile)
auto& topic = mreq::register_topic<MyMsg>("my_topic");
auto token = topic.subscribe();
MyMsg msg = ...;
topic.publish(msg);
if (topic.check(token.value())) {
    auto rx = topic.read(token.value());
    // ...
}
topic.unsubscribe(token.value());
```

## Otomatik Kod Üretimi
1. `proto/` klasörüne .proto dosyanızı ekleyin.
2. nanopb ile `gen/` klasörüne C kaynaklarını üretin.
3. `tools/proto_registry.py` ile trait ve registry dosyalarını oluşturun.
4. CMake ile projeyi derleyin.

## Gereksinimler
- C++17
- FreeRTOS (veya baremetal)
- nanopb

Daha fazla bilgi ve gelişmiş kullanım için `prd.md` dosyasına bakınız. 