#include "gtest/gtest.h"
#include "mreq/mreq.hpp"
#include "test_messages.hpp"

// test_main.cpp'de tanımlanan global topic'lere erişim
extern mreq::Topic<TestMessage1, 1> test_topic_1_topic_instance;
extern mreq::Topic<TestMessage2, 5> test_topic_2_topic_instance;

TEST(TopicTest, SubscribeUnsubscribe) {
    auto token_opt = test_topic_1_topic_instance.subscribe();
    ASSERT_TRUE(token_opt.has_value());
    
    auto token = token_opt.value();
    EXPECT_FALSE(test_topic_1_topic_instance.check(token));

    test_topic_1_topic_instance.unsubscribe(token);
}

TEST(TopicTest, PublishRead) {
    auto token_opt = test_topic_1_topic_instance.subscribe();
    ASSERT_TRUE(token_opt.has_value());
    auto token = token_opt.value();

    TestMessage1 msg_sent{123, 45.6f, 789};
    test_topic_1_topic_instance.publish(msg_sent);

    ASSERT_TRUE(test_topic_1_topic_instance.check(token));

    auto msg_read_opt = test_topic_1_topic_instance.read(token);
    ASSERT_TRUE(msg_read_opt.has_value());

    const auto& msg_read = msg_read_opt.value();
    EXPECT_EQ(msg_read.value1, msg_sent.value1);
    EXPECT_FLOAT_EQ(msg_read.value2, msg_sent.value2);
    EXPECT_EQ(msg_read.timestamp, msg_sent.timestamp);

    EXPECT_FALSE(test_topic_1_topic_instance.check(token));
    auto msg_read_opt2 = test_topic_1_topic_instance.read(token);
    EXPECT_FALSE(msg_read_opt2.has_value());
}

TEST(TopicTest, RingBuffer) {
    auto token_opt = test_topic_2_topic_instance.subscribe();
    ASSERT_TRUE(token_opt.has_value());
    auto token = token_opt.value();

    for (int i = 0; i < 5; ++i) {
        TestMessage2 msg{static_cast<double>(i), true, {}, static_cast<uint64_t>(i)};
        test_topic_2_topic_instance.publish(msg);
    }

    TestMessage2 last_msg{99.0, false, {}, 999};
    test_topic_2_topic_instance.publish(last_msg);

    size_t read_count = 0;
    while(test_topic_2_topic_instance.check(token)) {
        auto msg_opt = test_topic_2_topic_instance.read(token);
        if(msg_opt) {
            read_count++;
            if(read_count == 5) {
                 EXPECT_EQ(msg_opt.value().value1, last_msg.value1);
            }
        } else {
            break;
        }
    }
    EXPECT_EQ(read_count, 5);
}
