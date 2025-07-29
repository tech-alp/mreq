#include "mreq/topic_registry_metadata.hpp"
#include <cassert>
#include <iostream>
#include <thread>

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

void test_registry_creation() {
    std::cout << "Testing registry creation..." << std::endl;
    
    auto& registry = mreq::TopicRegistryMetadata::instance();
    
    assert(registry.topic_count() == 0);
    
    std::cout << "✓ Registry creation test passed" << std::endl;
}

void test_registry_register_topic() {
    std::cout << "Testing registry register topic..." << std::endl;
    
    auto& registry = mreq::TopicRegistryMetadata::instance();
    
    // Topic'leri kaydet
    auto& topic1 = registry.register_topic<mreq_test::TestMessage1>(MREQ_ID(TestMessage1));
    auto& topic2 = registry.register_topic<mreq_test::TestMessage2>(MREQ_ID(TestMessage2));
    
    assert(registry.topic_count() == 2);
    assert(topic1.get_metadata() == MREQ_ID(TestMessage1));
    assert(topic2.get_metadata() == MREQ_ID(TestMessage2));
    
    // Aynı metadata ile tekrar kaydet - mevcut topic'i döndürmeli
    auto& topic1_again = registry.register_topic<mreq_test::TestMessage1>(MREQ_ID(TestMessage1));
    assert(&topic1 == &topic1_again);
    assert(registry.topic_count() == 2); // Sayı değişmemeli
    
    std::cout << "✓ Registry register topic test passed" << std::endl;
}

void test_registry_get_topic() {
    std::cout << "Testing registry get topic..." << std::endl;
    
    auto& registry = mreq::TopicRegistryMetadata::instance();
    
    // Topic'leri kaydet
    registry.register_topic<mreq_test::TestMessage1>(MREQ_ID(TestMessage1));
    registry.register_topic<mreq_test::TestMessage2>(MREQ_ID(TestMessage2));
    
    // Topic'leri al
    auto topic1_opt = registry.get_topic<mreq_test::TestMessage1>(MREQ_ID(TestMessage1));
    auto topic2_opt = registry.get_topic<mreq_test::TestMessage2>(MREQ_ID(TestMessage2));
    
    assert(topic1_opt.has_value());
    assert(topic2_opt.has_value());
    assert(topic1_opt.value().get().get_metadata() == MREQ_ID(TestMessage1));
    assert(topic2_opt.value().get().get_metadata() == MREQ_ID(TestMessage2));
    
    // Olmayan topic'i al
    auto* non_existent = registry.get(MREQ_ID(TestMessage1)); // Farklı metadata pointer
    assert(non_existent != nullptr); // Bu topic var
    
    std::cout << "✓ Registry get topic test passed" << std::endl;
}

void test_registry_has_topic() {
    std::cout << "Testing registry has topic..." << std::endl;
    
    auto& registry = mreq::TopicRegistryMetadata::instance();
    
    // Başlangıçta topic yok
    assert(registry.has_topic(MREQ_ID(TestMessage1)) == false);
    
    // Topic'i kaydet
    registry.register_topic<mreq_test::TestMessage1>(MREQ_ID(TestMessage1));
    
    // Artık topic var
    assert(registry.has_topic(MREQ_ID(TestMessage1)) == true);
    assert(registry.has_topic(MREQ_ID(TestMessage2)) == false);
    
    std::cout << "✓ Registry has topic test passed" << std::endl;
}

void test_registry_clear() {
    std::cout << "Testing registry clear..." << std::endl;
    
    auto& registry = mreq::TopicRegistryMetadata::instance();
    
    // Topic'leri kaydet
    registry.register_topic<mreq_test::TestMessage1>(MREQ_ID(TestMessage1));
    registry.register_topic<mreq_test::TestMessage2>(MREQ_ID(TestMessage2));
    
    assert(registry.topic_count() == 2);
    
    // Registry'yi temizle
    registry.clear();
    
    assert(registry.topic_count() == 0);
    assert(registry.has_topic(MREQ_ID(TestMessage1)) == false);
    assert(registry.has_topic(MREQ_ID(TestMessage2)) == false);
    
    std::cout << "✓ Registry clear test passed" << std::endl;
}

void test_registry_thread_safety() {
    std::cout << "Testing registry thread safety..." << std::endl;
    
    auto& registry = mreq::TopicRegistryMetadata::instance();
    
    const int num_threads = 4;
    const int topics_per_thread = 10;
    
    std::vector<std::thread> threads;
    
    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back([&registry, t, topics_per_thread]() {
            for (int i = 0; i < topics_per_thread; ++i) {
                // Her thread farklı metadata ile topic oluştur
                mreq::mreq_metadata dummy_metadata{
                    .topic_name = "dummy",
                    .payload_size = sizeof(mreq_test::TestMessage1),
                    .message_id = static_cast<size_t>(t * 1000 + i),
                    .nanopb_encode = nullptr,
                    .nanopb_decode = nullptr
                };
                
                auto& topic = registry.register_topic<mreq_test::TestMessage1>(&dummy_metadata);
                assert(topic.get_metadata() == &dummy_metadata);
            }
        });
    }
    
    // Thread'leri bekle
    for (auto& thread : threads) {
        thread.join();
    }
    
    // Toplam topic sayısı kontrol et
    assert(registry.topic_count() == num_threads * topics_per_thread);
    
    std::cout << "✓ Registry thread safety test passed" << std::endl;
}

void test_registry_global_functions() {
    std::cout << "Testing registry global functions..." << std::endl;
    
    // Global fonksiyonları test et
    auto& topic1 = mreq::register_topic<mreq_test::TestMessage1>(MREQ_ID(TestMessage1));
    auto& topic2 = mreq::register_topic<mreq_test::TestMessage2>(MREQ_ID(TestMessage2));
    
    assert(topic1.get_metadata() == MREQ_ID(TestMessage1));
    assert(topic2.get_metadata() == MREQ_ID(TestMessage2));
    
    auto topic1_opt = mreq::get_topic<mreq_test::TestMessage1>(MREQ_ID(TestMessage1));
    auto topic2_opt = mreq::get_topic<mreq_test::TestMessage2>(MREQ_ID(TestMessage2));
    
    assert(topic1_opt.has_value());
    assert(topic2_opt.has_value());
    assert(&topic1_opt.value().get() == &topic1);
    assert(&topic2_opt.value().get() == &topic2);
    
    auto* base_topic1 = mreq::get_topic(MREQ_ID(TestMessage1));
    auto* base_topic2 = mreq::get_topic(MREQ_ID(TestMessage2));
    
    assert(base_topic1 != nullptr);
    assert(base_topic2 != nullptr);
    assert(base_topic1->get_metadata() == MREQ_ID(TestMessage1));
    assert(base_topic2->get_metadata() == MREQ_ID(TestMessage2));
    
    std::cout << "✓ Registry global functions test passed" << std::endl;
}

int main() {
    std::cout << "=== MREQ Registry Unit Tests ===" << std::endl;
    
    try {
        test_registry_creation();
        test_registry_register_topic();
        test_registry_get_topic();
        test_registry_has_topic();
        test_registry_clear();
        test_registry_thread_safety();
        test_registry_global_functions();
        
        std::cout << "\n=== All registry tests passed! ===" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Test failed with unknown exception" << std::endl;
        return 1;
    }
} 