#include "mreq/metadata.hpp"
#include <cassert>
#include <iostream>
#include <cstring>

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

void test_metadata_creation() {
    std::cout << "Testing metadata creation..." << std::endl;
    
    // Test metadata oluşturma
    auto metadata1 = mreq::create_metadata<mreq_test::TestMessage1>("test1");
    auto metadata2 = mreq::create_metadata<mreq_test::TestMessage2>("test2");
    
    // Boyut kontrolü
    assert(metadata1.payload_size == sizeof(mreq_test::TestMessage1));
    assert(metadata2.payload_size == sizeof(mreq_test::TestMessage2));
    
    // İsim kontrolü
    assert(std::strcmp(metadata1.topic_name, "test1") == 0);
    assert(std::strcmp(metadata2.topic_name, "test2") == 0);
    
    std::cout << "✓ Metadata creation test passed" << std::endl;
}

void test_hash_function() {
    std::cout << "Testing hash function..." << std::endl;
    
    // Aynı string için aynı hash
    auto hash1 = mreq::hash_string("test");
    auto hash2 = mreq::hash_string("test");
    assert(hash1 == hash2);
    
    // Farklı string'ler için farklı hash
    auto hash3 = mreq::hash_string("test1");
    auto hash4 = mreq::hash_string("test2");
    assert(hash3 != hash4);
    
    // Boş string için hash
    auto hash5 = mreq::hash_string("");
    assert(hash5 == 5381); // DJB2 hash'in başlangıç değeri
    
    std::cout << "✓ Hash function test passed" << std::endl;
}

void test_metadata_macros() {
    std::cout << "Testing metadata macros..." << std::endl;
    
    // MREQ_ID makrosu testi
    auto* metadata1 = MREQ_ID(TestMessage1);
    auto* metadata2 = MREQ_ID(TestMessage2);
    
    assert(metadata1 != nullptr);
    assert(metadata2 != nullptr);
    assert(metadata1 != metadata2);
    
    // Metadata içerik kontrolü
    assert(std::strcmp(metadata1->topic_name, "test_message_1") == 0);
    assert(std::strcmp(metadata2->topic_name, "test_message_2") == 0);
    assert(metadata1->payload_size == sizeof(mreq_test::TestMessage1));
    assert(metadata2->payload_size == sizeof(mreq_test::TestMessage2));
    
    std::cout << "✓ Metadata macros test passed" << std::endl;
}

void test_metadata_comparison() {
    std::cout << "Testing metadata comparison..." << std::endl;
    
    auto* metadata1 = MREQ_ID(TestMessage1);
    auto* metadata2 = MREQ_ID(TestMessage2);
    
    // Eşitlik kontrolü
    assert(*metadata1 == *metadata1);
    assert(*metadata1 != *metadata2);
    
    std::cout << "✓ Metadata comparison test passed" << std::endl;
}

void test_nanopb_metadata() {
    std::cout << "Testing nanopb metadata..." << std::endl;
    
    // nanopb fonksiyon pointer'ları (placeholder)
    auto dummy_encode = [](const void* data, void* buffer, size_t buffer_size, size_t* message_length) -> bool {
        return true;
    };
    
    auto dummy_decode = [](const void* buffer, size_t buffer_size, void* data) -> bool {
        return true;
    };
    
    // nanopb metadata oluşturma testi
    mreq::mreq_metadata nanopb_metadata{
        .topic_name = "nanopb_test",
        .payload_size = sizeof(mreq_test::TestMessage1),
        .message_id = mreq::hash_string("nanopb_test"),
        .nanopb_encode = dummy_encode,
        .nanopb_decode = dummy_decode
    };
    
    assert(nanopb_metadata.nanopb_encode != nullptr);
    assert(nanopb_metadata.nanopb_decode != nullptr);
    assert(std::strcmp(nanopb_metadata.topic_name, "nanopb_test") == 0);
    
    std::cout << "✓ Nanopb metadata test passed" << std::endl;
}

int main() {
    std::cout << "=== MREQ Metadata Unit Tests ===" << std::endl;
    
    try {
        test_metadata_creation();
        test_hash_function();
        test_metadata_macros();
        test_metadata_comparison();
        test_nanopb_metadata();
        
        std::cout << "\n=== All metadata tests passed! ===" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Test failed with unknown exception" << std::endl;
        return 1;
    }
} 