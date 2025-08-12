#include "gtest/gtest.h"
#include "mreq/mreq.hpp"
#include "test_messages.hpp"

TEST(MetadataTest, Macros) {
  auto *metadata1 = MREQ_GET_METADATA(test_topic_1);
  auto *metadata2 = MREQ_GET_METADATA(test_topic_2);

  ASSERT_NE(metadata1, nullptr);
  ASSERT_NE(metadata2, nullptr);
  ASSERT_NE(metadata1, metadata2);

  EXPECT_STREQ(metadata1->topic_name, "test_topic_1");
  EXPECT_STREQ(metadata2->topic_name, "test_topic_2");
  EXPECT_EQ(metadata1->payload_size, sizeof(TestMessage1));
  EXPECT_EQ(metadata2->payload_size, sizeof(TestMessage2));
}

TEST(MetadataTest, Comparison) {
  auto *metadata1 = MREQ_GET_METADATA(test_topic_1);
  auto *metadata2 = MREQ_GET_METADATA(test_topic_2);
  auto *metadata1_again = MREQ_GET_METADATA(test_topic_1);

  EXPECT_EQ(metadata1, metadata1_again);
  EXPECT_NE(metadata1, metadata2);
}
