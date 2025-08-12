#pragma once
#include "mreq/mreq.hpp"
#include "mreq/topic_registry.hpp"

// Test mesaj yapıları (global namespace'de)
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


// Yeni API'ye göre metadata ve topic bildirimleri
MREQ_METADATA_DECLARE(test_topic_1);
MREQ_TOPIC_DECLARE(TestMessage1, test_topic_1, 1);

MREQ_METADATA_DECLARE(test_topic_2);
MREQ_TOPIC_DECLARE(TestMessage2, test_topic_2, 5);

MREQ_METADATA_DECLARE(test_topic_3);
MREQ_TOPIC_DECLARE(TestMessage3, test_topic_3, 1);