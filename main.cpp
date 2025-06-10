#include <iostream>
#include <atomic>
#include <cstring>
#include <example.pb.h>
#include "src/mreq/interface.hpp"

// ExampleMessage için fonksiyonların implementasyonu
static ExampleMessage example_buffer;
static std::atomic<bool> example_updated = false;

template<>
void publish<ExampleMessage>(const ExampleMessage& msg) {
    std::memcpy(&example_buffer, &msg, sizeof(ExampleMessage));
    example_updated.store(true, std::memory_order_release);
}

template<>
void subscribe<ExampleMessage>() {
    // Gerçek sistemde abone kaydı yapılır, burada gerek yok
}

template<>
bool check<ExampleMessage>() {
    return example_updated.load(std::memory_order_acquire);
}

template<>
void copy<ExampleMessage>(ExampleMessage& out) {
    std::memcpy(&out, &example_buffer, sizeof(ExampleMessage));
    example_updated.store(false, std::memory_order_release);
}

int main() {
    ExampleMessage msg;
    msg.id = 42;
    msg.data = 3.14;

    // nanopb string için callback kullanımı yerine örnek olarak sadece id kullanıyoruz

    publish(msg);

    if (check<ExampleMessage>()) {
        ExampleMessage received;
        copy(received);
        std::cout << "Alınan id: " << received.id << ", " << received.data << std::endl;
    } else {
        std::cout << "Yeni mesaj yok." << std::endl;
    }
    return 0;
} 
