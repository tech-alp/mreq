#include "gtest/gtest.h"
#include "mreq/mreq.hpp"
#include "test_messages.hpp"
#include <thread>
#include <vector>

// test_main.cpp'de tanımlanan global topic'lere erişim
extern mreq::Topic<TestMessage1, 1> test_topic_1_topic_instance;
extern mreq::Topic<TestMessage2, 5> test_topic_2_topic_instance;

TEST(IntegrationTest, BasicPublishSubscribe) {
    auto token_opt = test_topic_1_topic_instance.subscribe();
    ASSERT_TRUE(token_opt.has_value());
    auto token = token_opt.value();

    TestMessage1 msg_sent{10, 20.5f, 30};
    test_topic_1_topic_instance.publish(msg_sent);

    ASSERT_TRUE(test_topic_1_topic_instance.check(token));
    auto msg_read_opt = test_topic_1_topic_instance.read(token);
    ASSERT_TRUE(msg_read_opt.has_value());

    EXPECT_EQ(msg_read_opt.value().value1, msg_sent.value1);
}

TEST(IntegrationTest, MultipleSubscribers) {
    auto token1_opt = test_topic_1_topic_instance.subscribe();
    auto token2_opt = test_topic_1_topic_instance.subscribe();
    ASSERT_TRUE(token1_opt.has_value());
    ASSERT_TRUE(token2_opt.has_value());
    auto token1 = token1_opt.value();
    auto token2 = token2_opt.value();

    TestMessage1 msg_sent{100, 200.5f, 300};
    test_topic_1_topic_instance.publish(msg_sent);

    ASSERT_TRUE(test_topic_1_topic_instance.check(token1));
    auto msg1_opt = test_topic_1_topic_instance.read(token1);
    ASSERT_TRUE(msg1_opt.has_value());
    EXPECT_EQ(msg1_opt.value().value1, msg_sent.value1);

    ASSERT_TRUE(test_topic_1_topic_instance.check(token2));
    auto msg2_opt = test_topic_1_topic_instance.read(token2);
    ASSERT_TRUE(msg2_opt.has_value());
    EXPECT_EQ(msg2_opt.value().value1, msg_sent.value1);
}

TEST(IntegrationTest, RingBufferOverrun) {
    auto token_opt = test_topic_2_topic_instance.subscribe();
    ASSERT_TRUE(token_opt.has_value());
    auto token = token_opt.value();

    for (int i = 0; i < 7; ++i) {
        test_topic_2_topic_instance.publish({(double)i, false, {}, (uint64_t)i});
    }

    size_t count = 0;
    double first_val = 0;
    while(auto msg_opt = test_topic_2_topic_instance.read(token)) {
        if (count == 0) {
            first_val = msg_opt.value().value1;
        }
        count++;
    }
    EXPECT_EQ(count, 5);
    EXPECT_EQ(first_val, 2.0);
}