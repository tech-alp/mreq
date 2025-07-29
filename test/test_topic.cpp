#include "mreq/topic_metadata.hpp"
#include <cassert>
#include <iostream>
#include <thread>
#include <chrono>

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

// Metadata tanımları
MREQ_MESSAGE_TYPE(TestMessage1, "test_message_1");
MREQ_MESSAGE_TYPE(TestMessage2, "test_message_2");

} // namespace mreq_test

void test_topic_creation() {
    std::cout << "Testing topic creation..." << std::endl;
    
    auto* metadata = MREQ_ID(TestMessage1);
    mreq::TopicMetadata<mreq_test::TestMessage1> topic(metadata);
    
    assert(topic.get_metadata() == metadata);
    assert(topic.subscriber_count() == 0);
    assert(topic.get_sequence() == 0);
    
    std::cout << "✓ Topic creation test passed" << std::endl;
}

void test_topic_subscribe_unsubscribe() {
    std::cout << "Testing topic subscribe/unsubscribe..." << std::endl;
    
    auto* metadata = MREQ_ID(TestMessage1);
    mreq::TopicMetadata<mreq_test::TestMessage1> topic(metadata);
    
    // Abone ol
    auto token1 = topic.subscribe();
    assert(token1.has_value());
    assert(topic.subscriber_count() == 1);
    
    auto token2 = topic.subscribe();
    assert(token2.has_value());
    assert(token2.value() != token1.value());
    assert(topic.subscriber_count() == 2);
    
    // Aboneliği iptal et
    topic.unsubscribe(token1.value());
    assert(topic.subscriber_count() == 1);
    
    topic.unsubscribe(token2.value());
    assert(topic.subscriber_count() == 0);
    
    std::cout << "✓ Topic subscribe/unsubscribe test passed" << std::endl;
}

void test_topic_publish_read() {
    std::cout << "Testing topic publish/read..." << std::endl;
    
    auto* metadata = MREQ_ID(TestMessage1);
    mreq::TopicMetadata<mreq_test::TestMessage1> topic(metadata);
    
    // Abone ol
    auto token = topic.subscribe();
    assert(token.has_value());
    
    // Mesaj yayınla
    mreq_test::TestMessage1 msg{
        .value1 = 42,
        .value2 = 3.14f,
        .timestamp = 1234567890
    };
    
    topic.publish(&msg);
    assert(topic.get_sequence() == 1);
    
    // Mesajı oku
    assert(topic.check(token.value()) == true);
    
    mreq_test::TestMessage1 received_msg;
    bool read_success = topic.read(token.value(), &received_msg);
    assert(read_success == true);
    
    assert(received_msg.value1 == msg.value1);
    assert(received_msg.value2 == msg.value2);
    assert(received_msg.timestamp == msg.timestamp);
    
    // Mesaj zaten okunduğu için false dönmeli
    assert(topic.check(token.value()) == false);
    
    std::cout << "✓ Topic publish/read test passed" << std::endl;
}

void test_topic_multiple_subscribers() {
    std::cout << "Testing topic multiple subscribers..." << std::endl;
    
    auto* metadata = MREQ_ID(TestMessage1);
    mreq::TopicMetadata<mreq_test::TestMessage1> topic(metadata);
    
    // İki abone oluştur
    auto token1 = topic.subscribe();
    auto token2 = topic.subscribe();
    assert(token1.has_value() && token2.has_value());
    assert(topic.subscriber_count() == 2);
    
    // Mesaj yayınla
    mreq_test::TestMessage1 msg{
        .value1 = 100,
        .value2 = 2.718f,
        .timestamp = 9876543210
    };
    
    topic.publish(&msg);
    
    // Her iki abone de mesajı okuyabilmeli
    assert(topic.check(token1.value()) == true);
    assert(topic.check(token2.value()) == true);
    
    mreq_test::TestMessage1 received1, received2;
    assert(topic.read(token1.value(), &received1) == true);
    assert(topic.read(token2.value(), &received2) == true);
    
    assert(received1.value1 == received2.value1);
    assert(received1.value2 == received2.value2);
    assert(received1.timestamp == received2.timestamp);
    
    std::cout << "✓ Topic multiple subscribers test passed" << std::endl;
}

void test_topic_ring_buffer() {
    std::cout << "Testing topic ring buffer..." << std::endl;
    
    auto* metadata = MREQ_ID(TestMessage1);
    mreq::TopicMetadata<mreq_test::TestMessage1, 3> topic(metadata); // 3 elemanlı ring buffer
    
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
    
    std::cout << "✓ Topic ring buffer test passed" << std::endl;
}

void test_topic_thread_safety() {
    std::cout << "Testing topic thread safety..." << std::endl;
    
    auto* metadata = MREQ_ID(TestMessage1);
    mreq::TopicMetadata<mreq_test::TestMessage1> topic(metadata);
    
    auto token = topic.subscribe();
    assert(token.has_value());
    
    // Çoklu thread'de yayınlama
    const int num_threads = 4;
    const int messages_per_thread = 100;
    
    std::vector<std::thread> threads;
    
    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back([&topic, t, messages_per_thread]() {
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
    
    // Thread'leri bekle
    for (auto& thread : threads) {
        thread.join();
    }
    
    // Toplam mesaj sayısı kontrol et
    assert(topic.get_sequence() == num_threads * messages_per_thread);
    
    // Mesajları oku
    mreq_test::TestMessage1 received;
    int read_count = 0;
    
    while (topic.check(token.value())) {
        if (topic.read(token.value(), &received)) {
            read_count++;
        }
    }
    
    assert(read_count == num_threads * messages_per_thread);
    
    std::cout << "✓ Topic thread safety test passed" << std::endl;
}

int main() {
    std::cout << "=== MREQ Topic Unit Tests ===" << std::endl;
    
    try {
        test_topic_creation();
        test_topic_subscribe_unsubscribe();
        test_topic_publish_read();
        test_topic_multiple_subscribers();
        test_topic_ring_buffer();
        test_topic_thread_safety();
        
        std::cout << "\n=== All topic tests passed! ===" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Test failed with unknown exception" << std::endl;
        return 1;
    }
} 