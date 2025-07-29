#include "mreq/mreq.hpp"
#include <cassert>
#include <iostream>
#include <thread>
#include <chrono>
#include <vector>

// Test mesaj yapıları
namespace mreq_test {

struct TestMessage1 {
    int32_t value1;
    float value2;
    uint64_t timestamp;
};

struct TestMessage2 {
    double value1;
    bool value2;
    char value3[16];
    uint64_t timestamp;
};

struct TestMessage3 {
    uint8_t data[32];
    uint64_t timestamp;
};

// Metadata tanımları
MREQ_MESSAGE_TYPE(TestMessage1, "test_message_1");
MREQ_MESSAGE_TYPE(TestMessage2, "test_message_2");
MREQ_MESSAGE_TYPE(TestMessage3, "test_message_3");

} // namespace mreq_test

void test_basic_integration() {
    std::cout << "Testing basic integration..." << std::endl;
    
    // Registry'den topic'leri al
    auto& topic1 = mreq::register_topic<mreq_test::TestMessage1>(MREQ_ID(TestMessage1));
    auto& topic2 = mreq::register_topic<mreq_test::TestMessage2>(MREQ_ID(TestMessage2));
    
    // Abone ol
    auto token1 = topic1.subscribe();
    auto token2 = topic2.subscribe();
    assert(token1.has_value() && token2.has_value());
    
    // Mesajları yayınla
    mreq_test::TestMessage1 msg1{
        .value1 = 42,
        .value2 = 3.14f,
        .timestamp = 1234567890
    };
    
    mreq_test::TestMessage2 msg2{
        .value1 = 2.718,
        .value2 = true,
        .value3 = "test",
        .timestamp = 9876543210
    };
    
    topic1.publish(&msg1);
    topic2.publish(&msg2);
    
    // Mesajları oku
    assert(topic1.check(token1.value()) == true);
    assert(topic2.check(token2.value()) == true);
    
    mreq_test::TestMessage1 received1;
    mreq_test::TestMessage2 received2;
    
    assert(topic1.read(token1.value(), &received1) == true);
    assert(topic2.read(token2.value(), &received2) == true);
    
    assert(received1.value1 == msg1.value1);
    assert(received1.value2 == msg1.value2);
    assert(received1.timestamp == msg1.timestamp);
    
    assert(received2.value1 == msg2.value1);
    assert(received2.value2 == msg2.value2);
    assert(std::strcmp(received2.value3, msg2.value3) == 0);
    assert(received2.timestamp == msg2.timestamp);
    
    std::cout << "✓ Basic integration test passed" << std::endl;
}

void test_multiple_subscribers_integration() {
    std::cout << "Testing multiple subscribers integration..." << std::endl;
    
    auto& topic = mreq::register_topic<mreq_test::TestMessage1>(MREQ_ID(TestMessage1));
    
    // Çoklu abone oluştur
    std::vector<std::optional<size_t>> tokens;
    for (int i = 0; i < 5; ++i) {
        auto token = topic.subscribe();
        assert(token.has_value());
        tokens.push_back(token);
    }
    
    assert(topic.subscriber_count() == 5);
    
    // Mesaj yayınla
    mreq_test::TestMessage1 msg{
        .value1 = 100,
        .value2 = 2.5f,
        .timestamp = 5555555555
    };
    
    topic.publish(&msg);
    
    // Tüm aboneler mesajı okuyabilmeli
    for (const auto& token_opt : tokens) {
        assert(topic.check(token_opt.value()) == true);
        
        mreq_test::TestMessage1 received;
        assert(topic.read(token_opt.value(), &received) == true);
        
        assert(received.value1 == msg.value1);
        assert(received.value2 == msg.value2);
        assert(received.timestamp == msg.timestamp);
    }
    
    // Mesajlar okunduktan sonra false dönmeli
    for (const auto& token_opt : tokens) {
        assert(topic.check(token_opt.value()) == false);
    }
    
    std::cout << "✓ Multiple subscribers integration test passed" << std::endl;
}

void test_ring_buffer_integration() {
    std::cout << "Testing ring buffer integration..." << std::endl;
    
    // 3 elemanlı ring buffer ile topic oluştur
    auto& topic = mreq::register_topic<mreq_test::TestMessage1, 3>(MREQ_ID(TestMessage1));
    
    auto token = topic.subscribe();
    assert(token.has_value());
    
    // 5 mesaj yayınla (ring buffer 3 elemanlı)
    for (int i = 0; i < 5; ++i) {
        mreq_test::TestMessage1 msg{
            .value1 = i,
            .value2 = static_cast<float>(i) * 1.5f,
            .timestamp = static_cast<uint64_t>(i) * 1000
        };
        topic.publish(&msg);
    }
    
    assert(topic.get_sequence() == 5);
    
    // Sadece son 3 mesaj okunabilmeli
    mreq_test::TestMessage1 received;
    int read_count = 0;
    
    while (topic.check(token.value()) && read_count < 5) {
        if (topic.read(token.value(), &received)) {
            read_count++;
        }
    }
    
    // Ring buffer boyutu 3 olduğu için sadece son 3 mesaj okunmalı
    assert(read_count == 3);
    
    std::cout << "✓ Ring buffer integration test passed" << std::endl;
}

void test_registry_integration() {
    std::cout << "Testing registry integration..." << std::endl;
    
    auto& registry = mreq::TopicRegistryMetadata::instance();
    
    // Registry'yi temizle
    registry.clear();
    assert(registry.topic_count() == 0);
    
    // Topic'leri kaydet
    auto& topic1 = registry.register_topic<mreq_test::TestMessage1>(MREQ_ID(TestMessage1));
    auto& topic2 = registry.register_topic<mreq_test::TestMessage2>(MREQ_ID(TestMessage2));
    auto& topic3 = registry.register_topic<mreq_test::TestMessage3>(MREQ_ID(TestMessage3));
    
    assert(registry.topic_count() == 3);
    assert(registry.has_topic(MREQ_ID(TestMessage1)) == true);
    assert(registry.has_topic(MREQ_ID(TestMessage2)) == true);
    assert(registry.has_topic(MREQ_ID(TestMessage3)) == true);
    
    // Topic'leri registry'den al
    auto topic1_opt = registry.get_topic<mreq_test::TestMessage1>(MREQ_ID(TestMessage1));
    auto topic2_opt = registry.get_topic<mreq_test::TestMessage2>(MREQ_ID(TestMessage2));
    auto topic3_opt = registry.get_topic<mreq_test::TestMessage3>(MREQ_ID(TestMessage3));
    
    assert(topic1_opt.has_value());
    assert(topic2_opt.has_value());
    assert(topic3_opt.has_value());
    
    assert(&topic1_opt.value().get() == &topic1);
    assert(&topic2_opt.value().get() == &topic2);
    assert(&topic3_opt.value().get() == &topic3);
    
    // Global fonksiyonları test et
    auto* base_topic1 = mreq::get_topic(MREQ_ID(TestMessage1));
    auto* base_topic2 = mreq::get_topic(MREQ_ID(TestMessage2));
    auto* base_topic3 = mreq::get_topic(MREQ_ID(TestMessage3));
    
    assert(base_topic1 != nullptr);
    assert(base_topic2 != nullptr);
    assert(base_topic3 != nullptr);
    
    assert(base_topic1->get_metadata() == MREQ_ID(TestMessage1));
    assert(base_topic2->get_metadata() == MREQ_ID(TestMessage2));
    assert(base_topic3->get_metadata() == MREQ_ID(TestMessage3));
    
    std::cout << "✓ Registry integration test passed" << std::endl;
}

void test_thread_safety_integration() {
    std::cout << "Testing thread safety integration..." << std::endl;
    
    auto& topic = mreq::register_topic<mreq_test::TestMessage1>(MREQ_ID(TestMessage1));
    
    // Çoklu thread'de abone ol
    const int num_threads = 4;
    const int messages_per_thread = 50;
    
    std::vector<std::thread> publisher_threads;
    std::vector<std::thread> subscriber_threads;
    
    // Publisher thread'leri
    for (int t = 0; t < num_threads; ++t) {
        publisher_threads.emplace_back([&topic, t, messages_per_thread]() {
            for (int i = 0; i < messages_per_thread; ++i) {
                mreq_test::TestMessage1 msg{
                    .value1 = t * 1000 + i,
                    .value2 = static_cast<float>(t) + static_cast<float>(i) * 0.1f,
                    .timestamp = static_cast<uint64_t>(t) * 1000000 + i
                };
                topic.publish(&msg);
                std::this_thread::sleep_for(std::chrono::microseconds(1));
            }
        });
    }
    
    // Subscriber thread'leri
    for (int t = 0; t < num_threads; ++t) {
        subscriber_threads.emplace_back([&topic, t, messages_per_thread]() {
            auto token = topic.subscribe();
            if (token.has_value()) {
                mreq_test::TestMessage1 received;
                int read_count = 0;
                
                while (read_count < messages_per_thread) {
                    if (topic.check(token.value())) {
                        if (topic.read(token.value(), &received)) {
                            read_count++;
                        }
                    }
                    std::this_thread::sleep_for(std::chrono::microseconds(10));
                }
                
                topic.unsubscribe(token.value());
            }
        });
    }
    
    // Thread'leri bekle
    for (auto& thread : publisher_threads) {
        thread.join();
    }
    
    for (auto& thread : subscriber_threads) {
        thread.join();
    }
    
    // Son durum kontrol et
    assert(topic.get_sequence() == num_threads * messages_per_thread);
    assert(topic.subscriber_count() == 0);
    
    std::cout << "✓ Thread safety integration test passed" << std::endl;
}

void test_metadata_integration() {
    std::cout << "Testing metadata integration..." << std::endl;
    
    // Metadata'ları kontrol et
    auto* metadata1 = MREQ_ID(TestMessage1);
    auto* metadata2 = MREQ_ID(TestMessage2);
    auto* metadata3 = MREQ_ID(TestMessage3);
    
    assert(metadata1 != nullptr);
    assert(metadata2 != nullptr);
    assert(metadata3 != nullptr);
    
    assert(metadata1 != metadata2);
    assert(metadata2 != metadata3);
    assert(metadata1 != metadata3);
    
    // Metadata içeriklerini kontrol et
    assert(std::strcmp(metadata1->topic_name, "test_message_1") == 0);
    assert(std::strcmp(metadata2->topic_name, "test_message_2") == 0);
    assert(std::strcmp(metadata3->topic_name, "test_message_3") == 0);
    
    assert(metadata1->payload_size == sizeof(mreq_test::TestMessage1));
    assert(metadata2->payload_size == sizeof(mreq_test::TestMessage2));
    assert(metadata3->payload_size == sizeof(mreq_test::TestMessage3));
    
    // Metadata karşılaştırma
    assert(*metadata1 == *metadata1);
    assert(*metadata1 != *metadata2);
    assert(*metadata2 != *metadata3);
    
    std::cout << "✓ Metadata integration test passed" << std::endl;
}

int main() {
    std::cout << "=== MREQ Integration Tests ===" << std::endl;
    
    try {
        test_basic_integration();
        test_multiple_subscribers_integration();
        test_ring_buffer_integration();
        test_registry_integration();
        test_thread_safety_integration();
        test_metadata_integration();
        
        std::cout << "\n=== All integration tests passed! ===" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Test failed with unknown exception" << std::endl;
        return 1;
    }
} 