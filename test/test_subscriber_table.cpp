#include "mreq/subscriber_table.hpp"
#include <cassert>
#include <iostream>
#include <thread>

// Test mesaj yapısı
struct TestMessage {
    int32_t value;
    uint64_t timestamp;
};

void test_subscriber_table_creation() {
    std::cout << "Testing subscriber table creation..." << std::endl;
    
    SubscriberTable<TestMessage> table;
    
    assert(table.subscriber_count() == 0);
    
    std::cout << "✓ Subscriber table creation test passed" << std::endl;
}

void test_subscriber_table_subscribe() {
    std::cout << "Testing subscriber table subscribe..." << std::endl;
    
    SubscriberTable<TestMessage> table;
    
    // İlk abone
    auto token1 = table.subscribe();
    assert(token1.has_value());
    assert(token1.value() == 0); // İlk slot
    assert(table.subscriber_count() == 1);
    
    // İkinci abone
    auto token2 = table.subscribe();
    assert(token2.has_value());
    assert(token2.value() == 1); // İkinci slot
    assert(token2.value() != token1.value());
    assert(table.subscriber_count() == 2);
    
    // Maksimum abone sayısına kadar doldur
    std::vector<std::optional<size_t>> tokens;
    for (int i = 0; i < MREQ_MAX_SUBSCRIBERS - 2; ++i) {
        auto token = table.subscribe();
        assert(token.has_value());
        tokens.push_back(token);
    }
    
    assert(table.subscriber_count() == MREQ_MAX_SUBSCRIBERS);
    
    // Maksimum abone sayısına ulaşıldığında nullopt dönmeli
    auto overflow_token = table.subscribe();
    assert(!overflow_token.has_value());
    assert(table.subscriber_count() == MREQ_MAX_SUBSCRIBERS);
    
    std::cout << "✓ Subscriber table subscribe test passed" << std::endl;
}

void test_subscriber_table_unsubscribe() {
    std::cout << "Testing subscriber table unsubscribe..." << std::endl;
    
    SubscriberTable<TestMessage> table;
    
    // Abone ol
    auto token1 = table.subscribe();
    auto token2 = table.subscribe();
    assert(token1.has_value() && token2.has_value());
    assert(table.subscriber_count() == 2);
    
    // Aboneliği iptal et
    table.unsubscribe(token1.value());
    assert(table.subscriber_count() == 1);
    
    // Aynı slot'u tekrar kullanabilmeli
    auto token3 = table.subscribe();
    assert(token3.has_value());
    assert(token3.value() == token1.value()); // Aynı slot geri kullanılmalı
    assert(table.subscriber_count() == 2);
    
    // Diğer aboneliği iptal et
    table.unsubscribe(token2.value());
    assert(table.subscriber_count() == 1);
    
    std::cout << "✓ Subscriber table unsubscribe test passed" << std::endl;
}

void test_subscriber_table_check() {
    std::cout << "Testing subscriber table check..." << std::endl;
    
    SubscriberTable<TestMessage> table;
    
    auto token = table.subscribe();
    assert(token.has_value());
    
    // Başlangıçta yeni mesaj yok
    assert(table.check(token.value(), 0) == false);
    
    // Mesaj yayınlandığında true dönmeli
    assert(table.check(token.value(), 1) == true);
    assert(table.check(token.value(), 5) == true);
    
    // Slot'u güncelle
    table.update_read_state(token.value(), 3, 0);
    assert(table.check(token.value(), 3) == false); // Aynı sequence
    assert(table.check(token.value(), 4) == true);  // Yeni sequence
    
    std::cout << "✓ Subscriber table check test passed" << std::endl;
}

void test_subscriber_table_update_read_state() {
    std::cout << "Testing subscriber table update read state..." << std::endl;
    
    SubscriberTable<TestMessage> table;
    
    auto token = table.subscribe();
    assert(token.has_value());
    
    // Başlangıç durumu
    auto& slot = table.get_slot(token.value());
    assert(slot.active == true);
    assert(slot.last_read_seq == 0);
    assert(slot.read_buffer_idx == 0);
    
    // Durumu güncelle
    table.update_read_state(token.value(), 5, 2);
    assert(slot.last_read_seq == 5);
    assert(slot.read_buffer_idx == 2);
    
    // Tekrar güncelle
    table.update_read_state(token.value(), 10, 3);
    assert(slot.last_read_seq == 10);
    assert(slot.read_buffer_idx == 3);
    
    std::cout << "✓ Subscriber table update read state test passed" << std::endl;
}

void test_subscriber_table_get_slot() {
    std::cout << "Testing subscriber table get slot..." << std::endl;
    
    SubscriberTable<TestMessage> table;
    
    auto token = table.subscribe();
    assert(token.has_value());
    
    // Slot'a erişim
    auto& slot = table.get_slot(token.value());
    assert(slot.active == true);
    
    // Slot'u değiştir
    slot.last_read_seq = 42;
    slot.read_buffer_idx = 7;
    
    // Değişikliklerin kalıcı olduğunu kontrol et
    auto& slot_again = table.get_slot(token.value());
    assert(slot_again.last_read_seq == 42);
    assert(slot_again.read_buffer_idx == 7);
    
    std::cout << "✓ Subscriber table get slot test passed" << std::endl;
}

void test_subscriber_table_thread_safety() {
    std::cout << "Testing subscriber table thread safety..." << std::endl;
    
    SubscriberTable<TestMessage> table;
    
    const int num_threads = 4;
    const int operations_per_thread = 100;
    
    std::vector<std::thread> threads;
    
    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back([&table, t, operations_per_thread]() {
            for (int i = 0; i < operations_per_thread; ++i) {
                // Abone ol
                auto token = table.subscribe();
                if (token.has_value()) {
                    // Durumu güncelle
                    table.update_read_state(token.value(), t * 1000 + i, i % 10);
                    
                    // Check yap
                    table.check(token.value(), t * 1000 + i + 1);
                    
                    // Aboneliği iptal et
                    table.unsubscribe(token.value());
                }
            }
        });
    }
    
    // Thread'leri bekle
    for (auto& thread : threads) {
        thread.join();
    }
    
    // Son durum kontrol et
    assert(table.subscriber_count() == 0);
    
    std::cout << "✓ Subscriber table thread safety test passed" << std::endl;
}

void test_subscriber_table_edge_cases() {
    std::cout << "Testing subscriber table edge cases..." << std::endl;
    
    SubscriberTable<TestMessage> table;
    
    // Geçersiz token ile işlemler
    table.unsubscribe(999); // Geçersiz token, hata vermemeli
    
    // Geçersiz token ile check
    assert(table.check(999, 1) == false);
    
    // Geçersiz token ile update
    table.update_read_state(999, 1, 0); // Hata vermemeli
    
    // Geçersiz token ile get_slot (undefined behavior olabilir, test etmiyoruz)
    
    // Boş table'da subscriber_count
    assert(table.subscriber_count() == 0);
    
    std::cout << "✓ Subscriber table edge cases test passed" << std::endl;
}

int main() {
    std::cout << "=== MREQ SubscriberTable Unit Tests ===" << std::endl;
    
    try {
        test_subscriber_table_creation();
        test_subscriber_table_subscribe();
        test_subscriber_table_unsubscribe();
        test_subscriber_table_check();
        test_subscriber_table_update_read_state();
        test_subscriber_table_get_slot();
        test_subscriber_table_thread_safety();
        test_subscriber_table_edge_cases();
        
        std::cout << "\n=== All subscriber table tests passed! ===" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Test failed with unknown exception" << std::endl;
        return 1;
    }
} 