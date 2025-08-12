#include "gtest/gtest.h"
#include "mreq/mreq.hpp"
#include "test_messages.hpp"

TEST(RegistryTest, TopicRegistration) {
    auto *metadata1 = MREQ_GET_METADATA(test_topic_1);
    auto *metadata2 = MREQ_GET_METADATA(test_topic_2);

    auto* found_meta1 = mreq::find_topic_metadata(metadata1->message_id);
    ASSERT_NE(found_meta1, nullptr);
    EXPECT_EQ(found_meta1->message_id, metadata1->message_id);
    EXPECT_STREQ(found_meta1->topic_name, "test_topic_1");

    auto* found_meta2 = mreq::find_topic_metadata(metadata2->message_id);
    ASSERT_NE(found_meta2, nullptr);
    EXPECT_EQ(found_meta2->message_id, metadata2->message_id);
    EXPECT_STREQ(found_meta2->topic_name, "test_topic_2");
}

TEST(RegistryTest, FindNonExistentTopic) {
    auto* found_meta = mreq::find_topic_metadata(987654321);
    EXPECT_EQ(found_meta, nullptr);
}

TEST(RegistryTest, RegistrySize) {
    // test_main.cpp'de 3 topic tanımlandığı için boyutun en az 3 olmasını bekliyoruz.
    EXPECT_GE(mreq::TopicRegistry::instance().size(), 3);
}