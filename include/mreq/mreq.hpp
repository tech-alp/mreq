#pragma once

#include <optional>
#include <cassert>
#include "metadata.hpp"
#include "topic.hpp"

#define MREQ_SUBSCRIBE(NAME) \
    MREQ_GET_METADATA(NAME)->subscribe_fn(MREQ_GET_METADATA(NAME)->topic_instance)

#define MREQ_UNSUBSCRIBE(NAME, TOKEN) \
    MREQ_GET_METADATA(NAME)->unsubscribe_fn(MREQ_GET_METADATA(NAME)->topic_instance, TOKEN)

#define MREQ_CHECK(NAME, TOKEN) \
    MREQ_GET_METADATA(NAME)->check_fn(MREQ_GET_METADATA(NAME)->topic_instance, TOKEN)

#define MREQ_PUBLISH(NAME, DATA) \
    MREQ_GET_METADATA(NAME)->publish_fn(MREQ_GET_METADATA(NAME)->topic_instance, &(DATA))

#define MREQ_READ(NAME, TOKEN) \
    (static_cast<mreq::Topic<decltype(NAME##_topic_instance)::value_type, 1>*>(MREQ_GET_METADATA(NAME)->topic_instance))->read(TOKEN)

#define MREQ_READ_MULTIPLE(NAME, TOKEN, BUFFER, COUNT) \
    (static_cast<mreq::Topic<decltype(NAME##_topic_instance)::value_type, 1>*>(MREQ_GET_METADATA(NAME)->topic_instance))->read_multiple(TOKEN, BUFFER, COUNT)
