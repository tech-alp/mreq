# Product Requirements Document (PRD)
## MREQ – Modular Real-Time Embedded Queue

---

### 1. Project Overview
MREQ is a lightweight, modular, and type-safe publish-subscribe communication framework designed for real-time embedded systems. It enables safe and efficient data sharing between tasks in FreeRTOS-based environments using passive polling and zero-copy shared memory strategies.

---

### 2. Goals and Objectives
- Enable real-time, task-safe communication between isolated modules.
- Use `.proto` files and nanopb to define typed messages.
- Avoid dispatcher/callbacks; implement pure polling-based access.
- Ensure type-safe, zero-copy data handling via static buffers or ring buffers.
- Support automatic code generation from `.proto` definitions.
- Be compatible with STM32 and ESP32 platforms running FreeRTOS.

---

### 3. Scope
Defines the architecture of MREQ, covering:
- Topic abstraction via templates
- Trait-based compile-time metadata system
- Subscriber tracking with per-topic per-subscriber flags
- Shared memory buffering for messages
- Nanopb integration for message serialization
- Minimal, deterministic API surface
- Dynamic topic creation

Excluded from scope (future work):
- Inter-device transport (e.g. UART/UDP)
- QoS / retention / durability features

---

### 4. System Architecture Overview
- Each topic has a single shared memory buffer.
- Subscribers check for updates via polling.
- Messages are defined in .proto files and compiled using nanopb.
- Traits system provides ID, name, size, and metadata access at compile-time.
- Memory is zero-copy and statically allocated.

---

### 5. Functional Requirements
- `publish<T>(const T&)`: write message to topic buffer
- `subscribe<T>(subscriber_id_t)`: register subscriber
- `check<T>(subscriber_id_t, bool&)`: check for update
- `copy<T>(subscriber_id_t, T&)`: retrieve latest value
it can be development.

---

### 6. Components
- `Topic<T>`: template structure for each message type
- `TopicTraits<T>`: compile-time metadata mapping
- `SubscriberTable<T>`: update flag tracker per subscriber
- `interface.hpp`: API layer
- `proto_registry.py`: generates traits and registry from .proto

---

### 7. RTOS & Platform Integration
- Subscriber identity via FreeRTOS task handle
- Thread-safe, no dynamic memory
- No exceptions or RTTI (STM32/ESP32 safe)

---

### 8. Code Generation Workflow
1. `.proto` added to `proto/`
2. nanopb generates `.pb.h/.pb.c` into `{CMAKE_BINARY_DIR}/gen/`
3. `proto_registry.py` generates `topic_traits.hpp`, `topic_ids.hpp`
4. C++ build integrates generated sources via cmake

---

### 9. Acceptance Criteria
| Requirement | Test |
|-------------|------|
| Tasks can subscribe to topics | `subscribe<T>()` succeeds |
| Tasks can check for updates | `check<T>()` returns true after publish |
| Tasks can read messages | `copy<T>()` retrieves correct data |
| Publish is non-blocking | `publish<T>()` returns without delay |
| Type mismatch fails to compile | `publish<WrongType>()` triggers compile error |

---

### 10. Future Enhancements
- Logging and introspection APIs (e.g., `log_topic<T>()`)
- JSON serialization / GCS integration
- UDP or UART transport layer (via serialized nanopb frames)
- Priority-based subscriber handling
- Optional file-based or MQTT bridge

---

### 11. Folder Structure
```
/proto/                   → .proto definitions
/gen/                     → nanopb-generated code
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

---

### 12. Summary
MREQ provides a robust, modular, and type-safe real-time communication system tailored for embedded systems. With C++17 templates, polling-based semantics, nanopb message definitions, and zero-copy efficiency, it balances clarity and performance in FreeRTOS environments.
