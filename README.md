# MREQ – Modular Real-Time Embedded Queue

MREQ, gerçek zamanlı gömülü sistemler için hafif, modüler ve tip güvenli bir publish-subscribe iletişim framework'üdür. FreeRTOS tabanlı ortamlarda görevler arası güvenli ve verimli veri paylaşımını sağlar.

## Klasör Yapısı

```
/proto/                   → .proto tanımları
/gen/                     → nanopb tarafından üretilen kodlar
/src/mreq/
  ├── topic.hpp
  ├── topic_traits.hpp
  ├── subscriber_table.hpp
  ├── interface.hpp
  ├── topic_registry.hpp
/tools/
  ├── proto_registry.py
  └── nanopb_generator.py
```

## Temel Özellikler
- Gerçek zamanlı, görev güvenli iletişim
- .proto dosyaları ve nanopb ile tipli mesajlar
- Dispatcher/callback yok, saf polling tabanlı erişim
- Statik veya ring buffer ile sıfır kopya veri yönetimi
- Otomatik kod üretimi
- STM32 ve ESP32 (FreeRTOS) uyumlu

## Kullanım
1. `proto/` klasörüne .proto dosyanızı ekleyin.
2. nanopb ile `gen/` klasörüne C kaynaklarını üretin.
3. `tools/proto_registry.py` ile trait ve registry dosyalarını oluşturun.
4. CMake ile projeyi derleyin.

## Gereksinimler
- C++17
- FreeRTOS
- nanopb

Daha fazla bilgi için `prd.md` dosyasına bakınız. 