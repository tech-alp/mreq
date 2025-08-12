#include "gtest/gtest.h"
#include "test_messages.hpp"

// test_messages.hpp'de bildirilen topic'leri ve metadata'ları tanımla
REGISTER_TOPIC_WITH_BUFFER(TestMessage1, test_topic_1, 1);
MREQ_METADATA_DEFINE(TestMessage1, test_topic_1, 1);

REGISTER_TOPIC_WITH_BUFFER(TestMessage2, test_topic_2, 5);
MREQ_METADATA_DEFINE(TestMessage2, test_topic_2, 5);

REGISTER_TOPIC_WITH_BUFFER(TestMessage3, test_topic_3, 1);
MREQ_METADATA_DEFINE(TestMessage3, test_topic_3, 1);

// GoogleTest ana fonksiyonu